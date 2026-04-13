#include "codegen/CodeGenerator.h"
#include "parser/XmlParser.h"
#include "graph/BlockGraph.h"
#include "scheduler/Scheduler.h"
#include "model/Model.h"

#include <gtest/gtest.h>
#include <fstream>
#include <sstream>

class CodeGeneratorTest : public ::testing::Test
{
protected:
    static void writeTempFile(const std::string & path, const std::string & content)
    {
        std::ofstream ofs(path);
        ofs << content;
    }

    static void removeTempFile(const std::string & path)
    {
        std::remove(path.c_str());
    }
};

TEST_F(CodeGeneratorTest, FormatDoubleEdgeCases)
{
    std::string xml =
        "<?xml version=\"1.0\"?>\n"
        "<System>"
        "  <Block Name=\"A\" SID=\"1\" BlockType=\"Inport\"><P Name=\"Port\">1</P></Block>"
        "  <Block Name=\"G1\" SID=\"2\" BlockType=\"Gain\"><P Name=\"Gain\">3.0</P></Block>"
        "  <Block Name=\"G2\" SID=\"3\" BlockType=\"Gain\"><P Name=\"Gain\">0.01</P></Block>"
        "  <Block Name=\"G3\" SID=\"4\" BlockType=\"Gain\"><P Name=\"Gain\">2.5</P></Block>"
        "  <Line><Src>1#R:1</Src><P Name=\"Dst\">2#L:1</P></Line>"
        "  <Line><Src>1#R:1</Src><P Name=\"Dst\">3#L:1</P></Line>"
        "  <Line><Src>1#R:1</Src><P Name=\"Dst\">4#L:1</P></Line>"
        "</System>";

    std::string tmpFile = "test_format.xml";
    writeTempFile(tmpFile, xml);

    XmlParser xp;
    Model model = xp.parse(tmpFile);

    BlockGraph graph(model);
    graph.build();

    Scheduler sched(graph, model);
    std::vector<int> schedule = sched.schedule();

    CodeGenerator gen(model, schedule);

    std::ostringstream oss;
    gen.emit(oss);
    std::string output = oss.str();

    // Check that 3.0 is formatted as "3" (not "3.000000")
    EXPECT_NE(output.find("* 3"), std::string::npos);
    EXPECT_EQ(output.find("* 3."), std::string::npos);

    // Check that 0.01 stays as "0.01"
    EXPECT_NE(output.find("* 0.01"), std::string::npos);

    // Check that 2.5 stays as "2.5"
    EXPECT_NE(output.find("* 2.5"), std::string::npos);

    removeTempFile(tmpFile);
}

TEST_F(CodeGeneratorTest, SignalNameAllPriorities)
{
    std::string xml =
        "<?xml version=\"1.0\"?>\n"
        "<System>"
        "  <Block Name=\"InA\" SID=\"1\" BlockType=\"Inport\"><P Name=\"Port\">1</P></Block>"
        "  <Block Name=\"SumB\" SID=\"2\" BlockType=\"Sum\"><P Name=\"Inputs\">+-</P></Block>"
        "  <Block Name=\"OutC\" SID=\"3\" BlockType=\"Outport\"><P Name=\"Port\">1</P></Block>"
        "  <Line Name=\"mySignal\"><Src>1#R:1</Src><P Name=\"Dst\">2#L:1</P></Line>"
        "  <Line><Src>2#R:1</Src><P Name=\"Dst\">3#L:1</P></Line>"
        "</System>";

    std::string tmpFile = "test_signame.xml";
    writeTempFile(tmpFile, xml);

    XmlParser xp;
    Model model = xp.parse(tmpFile);

    BlockGraph graph(model);
    graph.build();

    Scheduler sched(graph, model);
    std::vector<int> schedule = sched.schedule();

    CodeGenerator gen(model, schedule);

    std::ostringstream oss;
    gen.emit(oss);
    std::string output = oss.str();

    // Priority 1: Block name "SumB" should be used for SID=2 (not line name "mySignal")
    EXPECT_NE(output.find("SumB"), std::string::npos);

    // Outport SID=3 is not emitted in struct; check it's not generating invalid code
    EXPECT_NE(output.find("static struct"), std::string::npos);

    removeTempFile(tmpFile);
}

TEST_F(CodeGeneratorTest, BuildExpression)
{
    std::string xml =
        "<?xml version=\"1.0\"?>\n"
        "<System>"
        "  <Block Name=\"In\" SID=\"1\" BlockType=\"Inport\"><P Name=\"Port\">1</P></Block>"
        "  <Block Name=\"S\" SID=\"2\" BlockType=\"Sum\"><P Name=\"Inputs\">+-</P></Block>"
        "  <Block Name=\"G\" SID=\"3\" BlockType=\"Gain\"><P Name=\"Gain\">2</P></Block>"
        "  <Block Name=\"Out\" SID=\"4\" BlockType=\"Outport\"><P Name=\"Port\">1</P></Block>"
        "  <Line><Src>1#R:1</Src><P Name=\"Dst\">2#L:1</P></Line>"
        "  <Line><Src>1#R:1</Src><P Name=\"Dst\">2#L:2</P></Line>"
        "  <Line><Src>2#R:1</Src><P Name=\"Dst\">3#L:1</P></Line>"
        "  <Line><Src>3#R:1</Src><P Name=\"Dst\">4#L:1</P></Line>"
        "</System>";

    std::string tmpFile = "test_expr.xml";
    writeTempFile(tmpFile, xml);

    XmlParser xp;
    Model model = xp.parse(tmpFile);

    BlockGraph graph(model);
    graph.build();

    Scheduler sched(graph, model);
    std::vector<int> schedule = sched.schedule();

    CodeGenerator gen(model, schedule);

    std::ostringstream oss;
    gen.emit(oss);
    std::string output = oss.str();

    // Sum expression should contain +/- operators
    EXPECT_NE(output.find(" - "), std::string::npos);

    // Gain expression should contain * operator
    EXPECT_NE(output.find(" * 2"), std::string::npos);

    removeTempFile(tmpFile);
}
