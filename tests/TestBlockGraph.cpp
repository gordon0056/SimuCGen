#include "graph/BlockGraph.h"
#include "parser/XmlParser.h"
#include "model/Model.h"

#include <gtest/gtest.h>
#include <fstream>

class BlockGraphTest : public ::testing::Test
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

TEST_F(BlockGraphTest, AdjacencyList)
{
    std::string xml =
        "<?xml version=\"1.0\"?>\n"
        "<System>"
        "  <Block Name=\"A\" SID=\"1\" BlockType=\"Inport\"><P Name=\"Port\">1</P></Block>"
        "  <Block Name=\"B\" SID=\"2\" BlockType=\"Sum\"><P Name=\"Inputs\">+-</P></Block>"
        "  <Block Name=\"C\" SID=\"3\" BlockType=\"Outport\"><P Name=\"Port\">1</P></Block>"
        "  <Line><Src>1#R:1</Src><P Name=\"Dst\">2#L:1</P></Line>"
        "  <Line><Src>2#R:1</Src><P Name=\"Dst\">3#L:1</P></Line>"
        "</System>";

    std::string tmpFile = "test_adj.xml";
    writeTempFile(tmpFile, xml);

    XmlParser xp;
    Model model = xp.parse(tmpFile);

    BlockGraph graph(model);
    graph.build();

    const auto & adj = graph.adjacency();
    EXPECT_EQ(adj.at(1).size(), 1);
    EXPECT_EQ(adj.at(1)[0], 2);
    EXPECT_EQ(adj.at(2).size(), 1);
    EXPECT_EQ(adj.at(2)[0], 3);

    removeTempFile(tmpFile);
}

TEST_F(BlockGraphTest, SoftEdges)
{
    std::string xml =
        "<?xml version=\"1.0\"?>\n"
        "<System>"
        "  <Block Name=\"A\" SID=\"1\" BlockType=\"Inport\"><P Name=\"Port\">1</P></Block>"
        "  <Block Name=\"B\" SID=\"2\" BlockType=\"Sum\"><P Name=\"Inputs\">+-</P></Block>"
        "  <Block Name=\"D\" SID=\"3\" BlockType=\"UnitDelay\"><P Name=\"SampleTime\">-1</P></Block>"
        "  <Block Name=\"C\" SID=\"4\" BlockType=\"Outport\"><P Name=\"Port\">1</P></Block>"
        "  <Line><Src>1#R:1</Src><P Name=\"Dst\">2#L:1</P></Line>"
        "  <Line><Src>2#R:1</Src><P Name=\"Dst\">3#L:1</P></Line>"
        "  <Line><Src>3#R:1</Src><P Name=\"Dst\">4#L:1</P></Line>"
        "</System>";

    std::string tmpFile = "test_soft.xml";
    writeTempFile(tmpFile, xml);

    XmlParser xp;
    Model model = xp.parse(tmpFile);

    BlockGraph graph(model);
    graph.build();

    const auto & inDeg = graph.inDegree();
    // UnitDelay (SID=3) input should NOT be counted in inDegree
    EXPECT_EQ(inDeg.at(3), 0);  // Soft edge not counted
    EXPECT_EQ(inDeg.at(2), 1);  // Hard edge from SID=1

    removeTempFile(tmpFile);
}

TEST_F(BlockGraphTest, InvalidConnectionRef)
{
    std::string xml =
        "<?xml version=\"1.0\"?>\n"
        "<System>"
        "  <Block Name=\"A\" SID=\"1\" BlockType=\"Inport\"><P Name=\"Port\">1</P></Block>"
        "  <Line><Src>1#R:1</Src><P Name=\"Dst\">999#L:1</P></Line>"
        "</System>";

    std::string tmpFile = "test_invalid.xml";
    writeTempFile(tmpFile, xml);

    XmlParser xp;
    Model model = xp.parse(tmpFile);

    BlockGraph graph(model);
    graph.build();

    EXPECT_EXIT(graph.validate(), ::testing::ExitedWithCode(1), "");

    removeTempFile(tmpFile);
}
