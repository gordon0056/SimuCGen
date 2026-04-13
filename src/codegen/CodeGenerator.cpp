#include "codegen/CodeGenerator.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

CodeGenerator::CodeGenerator(const Model & model, const std::vector<int> & schedule)
    : m_model(model)
    , m_schedule(schedule)
{
    for (const auto & conn : m_model.m_connections)
    {
        m_sidToConnection[conn.m_srcSid] = &conn;
    }

    for (const auto & conn : m_model.m_connections)
    {
        m_dstToConns[conn.m_dstSid].push_back(&conn);
    }
    for (auto & pair : m_dstToConns)
    {
        std::sort(
            pair.second.begin(),
            pair.second.end(),
            [](const Connection * a, const Connection * b)
            {
                return a->m_dstPort < b->m_dstPort;
            }
        );
    }
}

void CodeGenerator::emit(std::ostream & out)
{
    emitIncludes(out);
    emitStruct(out);
    emitInitFunction(out);
    emitStepFunction(out);
    emitExtPorts(out);
}

void CodeGenerator::emitIncludes(std::ostream & out)
{
    out << "#include \"nwocg_run.h\"\n";
    out << "#include <stddef.h>\n";
    out << "#include <math.h>\n";
    out << "\n";
}

void CodeGenerator::emitStruct(std::ostream & out)
{
    out << "static struct\n{\n";

    std::set<int> emittedUnitDelays;

    for (int sid : m_schedule)
    {
        const Block & block = m_model.m_blocks[m_model.m_sidToIndex.at(sid)];

        if (block.m_type == BlockType::OUTPORT)
            continue;
        if (block.m_type == BlockType::UNIT_DELAY)
            continue;

        if (block.m_type != BlockType::INPORT)
        {
            auto it = m_dstToConns.find(sid);
            if (it != m_dstToConns.end())
            {
                for (const Connection * conn : it->second)
                {
                    auto srcIt = m_model.m_sidToIndex.find(conn->m_srcSid);
                    if (srcIt == m_model.m_sidToIndex.end())
                        continue;
                    const Block & srcBlock = m_model.m_blocks[srcIt->second];
                    if (srcBlock.m_type == BlockType::UNIT_DELAY
                        && emittedUnitDelays.find(conn->m_srcSid) == emittedUnitDelays.end())
                    {
                        out << "    double " << signalName(conn->m_srcSid) << ";\n";
                        emittedUnitDelays.insert(conn->m_srcSid);
                    }
                }
            }
        }

        out << "    double " << signalName(sid) << ";\n";
    }

    for (int sid : m_schedule)
    {
        const Block & block = m_model.m_blocks[m_model.m_sidToIndex.at(sid)];
        if (block.m_type == BlockType::UNIT_DELAY
            && emittedUnitDelays.find(sid) == emittedUnitDelays.end())
        {
            out << "    double " << signalName(sid) << ";\n";
        }
    }

    out << "} nwocg;\n";
    out << "\n";
}

void CodeGenerator::emitInitFunction(std::ostream & out)
{
    out << "void nwocg_generated_init()\n{\n";

    for (int sid : m_schedule)
    {
        const Block & block = m_model.m_blocks[m_model.m_sidToIndex.at(sid)];
        if (block.m_type == BlockType::UNIT_DELAY)
        {
            std::string sig = signalName(sid);
            out << "    nwocg." << sig << " = 0;\n";
        }
    }

    out << "}\n";
    out << "\n";
}

void CodeGenerator::emitStepFunction(std::ostream & out)
{
    out << "void nwocg_generated_step()\n{\n";

    for (int sid : m_schedule)
    {
        const Block & block = m_model.m_blocks[m_model.m_sidToIndex.at(sid)];

        if (block.m_type == BlockType::INPORT)
            continue;

        if (block.m_type == BlockType::OUTPORT)
            continue;

        if (block.m_type == BlockType::UNIT_DELAY)
            continue;

        std::string sig = signalName(sid);
        std::string expr = buildExpression(block);
        out << "    nwocg." << sig << " = " << expr << ";\n";
    }

    bool hasUnitDelay = false;
    for (int sid : m_schedule)
    {
        const Block & block = m_model.m_blocks[m_model.m_sidToIndex.at(sid)];
        if (block.m_type == BlockType::UNIT_DELAY)
        {
            hasUnitDelay = true;
            break;
        }
    }
    if (hasUnitDelay)
    {
        out << "\n";
    }

    for (int sid : m_schedule)
    {
        const Block & block = m_model.m_blocks[m_model.m_sidToIndex.at(sid)];
        if (block.m_type != BlockType::UNIT_DELAY)
            continue;

        std::string dstSig = signalName(sid);

        std::string srcSig;
        auto it = m_dstToConns.find(sid);
        if (it != m_dstToConns.end() && !it->second.empty())
        {
            srcSig = signalName(it->second[0]->m_srcSid);
        }

        if (!srcSig.empty())
        {
            out << "    nwocg." << dstSig << " = nwocg." << srcSig << ";\n";
        }
    }

    out << "}\n";
    out << "\n";
}

void CodeGenerator::emitExtPorts(std::ostream & out)
{
    out << "static const nwocg_ExtPort\n";
    out << "    ext_ports[] =\n{\n";

    std::vector<const Block *> inports;
    std::vector<const Block *> outports;

    for (int sid : m_schedule)
    {
        const Block & block = m_model.m_blocks[m_model.m_sidToIndex.at(sid)];
        if (block.m_type == BlockType::INPORT)
        {
            inports.push_back(&block);
        }
        else if (block.m_type == BlockType::OUTPORT)
        {
            outports.push_back(&block);
        }
    }

    std::sort(
        outports.begin(),
        outports.end(),
        [](const Block * a, const Block * b) { return a->m_name < b->m_name; }
    );
    std::sort(
        inports.begin(),
        inports.end(),
        [](const Block * a, const Block * b) { return a->m_name < b->m_name; }
    );

    for (const Block * block : outports)
    {
        std::string sig;
        auto it = m_dstToConns.find(block->m_sid);
        if (it == m_dstToConns.end() || it->second.empty())
        {
            std::cerr << "[ERROR] CodeGenerator: Outport has no input connection. SID="
                      << block->m_sid << "\n";
            std::exit(1);
        }
        sig = signalName(it->second[0]->m_srcSid);
        out << "    { \"" << block->m_name << "\", &nwocg." << sig << ", 0 },\n";
    }

    for (const Block * block : inports)
    {
        std::string sig = signalName(block->m_sid);
        out << "    { \"" << block->m_name << "\", &nwocg." << sig << ", 1 },\n";
    }

    out << "    { 0, 0, 0 },\n";
    out << "};\n";
    out << "\n";
    out << "const nwocg_ExtPort * const\n";
    out << "    nwocg_generated_ext_ports = ext_ports;\n";
    out << "\n";
    out << "const size_t\n";
    out << "    nwocg_generated_ext_ports_size = sizeof(ext_ports);\n";
}

static std::string sanitize(const std::string & s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s)
    {
        if (c != ' ' && c != '\t')
            out += c;
    }
    return out;
}

std::string CodeGenerator::signalName(int sid)
{
    auto it = m_sidToSignal.find(sid);
    if (it != m_sidToSignal.end())
        return it->second;

    const Block & block = m_model.m_blocks[m_model.m_sidToIndex.at(sid)];

    if (!block.m_name.empty())
    {
        m_sidToSignal[sid] = sanitize(block.m_name);
        return m_sidToSignal[sid];
    }

    auto connIt = m_sidToConnection.find(sid);
    if (connIt != m_sidToConnection.end() && !connIt->second->m_signalName.empty())
    {
        m_sidToSignal[sid] = sanitize(connIt->second->m_signalName);
        return m_sidToSignal[sid];
    }

    m_sidToSignal[sid] = "var_" + std::to_string(sid);
    return m_sidToSignal[sid];
}

std::string CodeGenerator::formatDouble(double value, int sid)
{
    if (!std::isfinite(value))
    {
        std::cerr << "[ERROR] CodeGenerator: non-finite double value in block. SID=" << sid << "\n";
        std::exit(1);
    }

    std::ostringstream oss;
    oss.precision(std::numeric_limits<double>::max_digits10);
    oss << std::fixed << value;
    std::string s = oss.str();

    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
    if (!s.empty() && s.back() == '.')
        s.pop_back();

    return s;
}

std::string CodeGenerator::buildExpression(const Block & block)
{
    switch (block.m_type)
    {
        case BlockType::SUM:
        {
            auto it = m_dstToConns.find(block.m_sid);
            if (it == m_dstToConns.end() || it->second.empty())
                return "0";

            std::vector<std::pair<std::string, char>> inputs;
            for (std::size_t i = 0; i < it->second.size(); ++i)
            {
                char pol = (i < block.m_inputs.size()) ? block.m_inputs[i] : '+';
                inputs.push_back({signalName(it->second[i]->m_srcSid), pol});
            }

            if (inputs.empty())
            {
                std::cerr << "[WARN] CodeGenerator: Sum block has no inputs. SID="
                          << block.m_sid << "\n";
                return "0";
            }

            std::string expr;
            for (std::size_t i = 0; i < inputs.size(); ++i)
            {
                char pol = inputs[i].second;
                if (i == 0)
                {
                    if (pol == '-')
                        expr += "-nwocg." + inputs[i].first;
                    else
                        expr += "nwocg." + inputs[i].first;
                }
                else
                {
                    if (pol == '-')
                        expr += " - nwocg." + inputs[i].first;
                    else
                        expr += " + nwocg." + inputs[i].first;
                }
            }
            return expr;
        }

        case BlockType::GAIN:
        {
            auto it = m_dstToConns.find(block.m_sid);
            if (it != m_dstToConns.end() && !it->second.empty())
            {
                std::string srcSig = signalName(it->second[0]->m_srcSid);
                return "nwocg." + srcSig + " * " + formatDouble(block.m_gain, block.m_sid);
            }
            std::cerr << "[WARN] CodeGenerator: Gain block missing Gain parameter, using 0.0. SID="
                      << block.m_sid << "\n";
            return formatDouble(block.m_gain, block.m_sid);
        }

        case BlockType::INPORT:
        case BlockType::OUTPORT:
        case BlockType::UNIT_DELAY:
            return "0";

        default:
            std::cerr << "[ERROR] Scheduler: unsupported BlockType. SID=" << block.m_sid << "\n";
            std::exit(1);
    }
}
