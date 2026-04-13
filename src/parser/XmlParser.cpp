#include "parser/XmlParser.h"

#include <algorithm>
#include <iostream>
#include <sstream>

Model XmlParser::parse(const std::string & filePath)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filePath.c_str());
    if (!result)
    {
        std::cerr << "[ERROR] XmlParser: cannot open file: " << filePath << "\n";
        std::exit(1);
    }

    Model model;

    pugi::xml_node root = doc.first_child();
    for (const auto & blockNode : root.children("Block"))
    {
        parseBlock(blockNode, model);
    }

    std::sort(
        model.m_blocks.begin(),
        model.m_blocks.end(),
        [](const Block & a, const Block & b) { return a.m_sid < b.m_sid; }
    );

    for (std::size_t i = 0; i < model.m_blocks.size(); ++i)
    {
        model.m_sidToIndex[model.m_blocks[i].m_sid] = i;
    }

    for (const auto & lineNode : root.children("Line"))
    {
        std::string srcStr = getParam(lineNode, "Src");
        if (srcStr.empty())
        {
            pugi::xml_node srcNode = lineNode.child("Src");
            if (srcNode)
                srcStr = srcNode.text().as_string("");
        }
        if (srcStr.empty())
            continue;

        Connection srcEndpoint = parseEndpoint(srcStr);
        parseLine(lineNode, srcEndpoint.m_dstSid, srcEndpoint.m_dstPort, model);
    }

    return model;
}

void XmlParser::parseBlock(const pugi::xml_node & node, Model & model)
{
    Block block;

    block.m_sid = node.attribute("SID").as_int(-1);
    if (block.m_sid < 0)
    {
        std::cerr << "[ERROR] XmlParser: missing SID attribute.\n";
        std::exit(1);
    }

    const char * rawType = node.attribute("BlockType").as_string();
    if (!rawType || rawType[0] == '\0')
    {
        std::cerr << "[ERROR] XmlParser: missing BlockType. SID=" << block.m_sid << "\n";
        std::exit(1);
    }

    block.m_type = resolveBlockType(rawType);
    if (block.m_type == BlockType::UNKNOWN)
    {
        std::cerr << "[ERROR] Scheduler: unsupported BlockType \"" << rawType << "\". SID=" << block.m_sid << "\n";
        std::exit(1);
    }

    block.m_name = node.attribute("Name").as_string();

    for (const auto & pNode : node.children("P"))
    {
        std::string pname = pNode.attribute("Name").as_string();
        std::string pvalue = pNode.text().as_string();

        if (pname == "Gain")
        {
            block.m_gain = std::stod(pvalue);
        }
        else if (pname == "Inputs")
        {
            block.m_inputs = pvalue;
        }
        else if (pname == "SampleTime")
        {
            block.m_sampleTime = std::stod(pvalue);
        }
        else if (pname == "Name")
        {
            if (block.m_name.empty())
            {
                block.m_name = pvalue;
            }
        }
        else if (pname == "Port")
        {
            Port port;
            port.m_portNumber = std::stoi(pvalue);
            port.m_name = pNode.attribute("Name").as_string();
            block.m_ports.push_back(port);
        }
    }

    model.m_blocks.push_back(block);
}

void XmlParser::parseLine(
    const pugi::xml_node & node,
    int srcSid,
    int srcPort,
    Model & model)
{
    for (const auto & dstNode : node.children("P"))
    {
        std::string dstName = dstNode.attribute("Name").as_string();
        if (dstName == "Dst")
        {
            Connection conn;
            conn.m_srcSid = srcSid;
            conn.m_srcPort = srcPort;

            std::string dstValue = dstNode.text().as_string();
            Connection parsed = parseEndpoint(dstValue);
            conn.m_dstSid = parsed.m_dstSid;
            conn.m_dstPort = parsed.m_dstPort;

            std::string lineName = getParam(node, "Name");
            if (lineName.empty())
            {
                lineName = node.attribute("Name").as_string();
            }
            if (!lineName.empty())
            {
                conn.m_signalName = lineName;
            }

            model.m_connections.push_back(conn);
        }
    }

    for (const auto & branchNode : node.children("Branch"))
    {
        parseLine(branchNode, srcSid, srcPort, model);
    }
}

Connection XmlParser::parseEndpoint(const std::string & endpoint)
{
    Connection result;
    result.m_srcSid = 0;
    result.m_srcPort = 0;
    result.m_dstSid = 0;
    result.m_dstPort = 0;

    auto hashPos = endpoint.find('#');
    if (hashPos == std::string::npos)
        return result;

    result.m_dstSid = std::stoi(endpoint.substr(0, hashPos));

    auto colonPos = endpoint.find(':', hashPos);
    if (colonPos == std::string::npos)
        return result;

    std::string dir = endpoint.substr(hashPos + 1, colonPos - hashPos - 1);
    result.m_dstPort = std::stoi(endpoint.substr(colonPos + 1));

    return result;
}

std::string XmlParser::getParam(const pugi::xml_node & block, const std::string & name)
{
    for (const auto & pNode : block.children("P"))
    {
        if (pNode.attribute("Name").as_string() == name)
        {
            return pNode.text().as_string();
        }
    }
    return "";
}

BlockType XmlParser::resolveBlockType(const std::string & typeStr)
{
    if (typeStr == "Inport")
        return BlockType::INPORT;
    if (typeStr == "Outport")
        return BlockType::OUTPORT;
    if (typeStr == "Sum")
        return BlockType::SUM;
    if (typeStr == "Gain")
        return BlockType::GAIN;
    if (typeStr == "UnitDelay")
        return BlockType::UNIT_DELAY;
    return BlockType::UNKNOWN;
}
