#include "parser/XmlParser.h"
#include "model/Model.h"

#include <algorithm>
#include <gtest/gtest.h>
#include <fstream>
#include <string>

class XmlParserTest : public ::testing::Test
{
protected:
    XmlParser parser;

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

TEST_F(XmlParserTest, ParseAllBlockTypes)
{
    std::string xml =
        "<?xml version=\"1.0\"?>\n"
        "<System>"
        "  <Block Name=\"In\" SID=\"1\" BlockType=\"Inport\"><P Name=\"Port\">1</P></Block>"
        "  <Block Name=\"Out\" SID=\"2\" BlockType=\"Outport\"><P Name=\"Port\">1</P></Block>"
        "  <Block Name=\"S\" SID=\"3\" BlockType=\"Sum\"><P Name=\"Inputs\">+-</P></Block>"
        "  <Block Name=\"G\" SID=\"4\" BlockType=\"Gain\"><P Name=\"Gain\">3.5</P></Block>"
        "  <Block Name=\"D\" SID=\"5\" BlockType=\"UnitDelay\"><P Name=\"SampleTime\">-1</P></Block>"
        "</System>";

    std::string tmpFile = "test_all_blocks.xml";
    writeTempFile(tmpFile, xml);

    Model model = parser.parse(tmpFile);

    EXPECT_EQ(model.m_blocks.size(), 5);
    EXPECT_EQ(model.m_blocks[0].m_type, BlockType::INPORT);
    EXPECT_EQ(model.m_blocks[1].m_type, BlockType::OUTPORT);
    EXPECT_EQ(model.m_blocks[2].m_type, BlockType::SUM);
    EXPECT_EQ(model.m_blocks[3].m_type, BlockType::GAIN);
    EXPECT_EQ(model.m_blocks[4].m_type, BlockType::UNIT_DELAY);

    // Check sorting by SID
    for (std::size_t i = 1; i < model.m_blocks.size(); ++i)
    {
        EXPECT_LT(model.m_blocks[i - 1].m_sid, model.m_blocks[i].m_sid);
    }

    removeTempFile(tmpFile);
}

TEST_F(XmlParserTest, RecursiveBranch)
{
    std::string xml =
        "<?xml version=\"1.0\"?>\n"
        "<System>"
        "  <Block Name=\"Src\" SID=\"1\" BlockType=\"Inport\"><P Name=\"Port\">1</P></Block>"
        "  <Block Name=\"D1\" SID=\"2\" BlockType=\"Outport\"><P Name=\"Port\">1</P></Block>"
        "  <Block Name=\"D2\" SID=\"3\" BlockType=\"Outport\"><P Name=\"Port\">1</P></Block>"
        "  <Block Name=\"D3\" SID=\"4\" BlockType=\"Outport\"><P Name=\"Port\">1</P></Block>"
        "  <Line Name=\"mainLine\">"
        "    <Src>1#R:1</Src>"
        "    <P Name=\"Dst\">2#L:1</P>"
        "    <Branch>"
        "      <P Name=\"Dst\">3#L:1</P>"
        "      <Branch>"
        "        <P Name=\"Dst\">4#L:1</P>"
        "      </Branch>"
        "    </Branch>"
        "  </Line>"
        "</System>";

    std::string tmpFile = "test_branch.xml";
    writeTempFile(tmpFile, xml);

    Model model = parser.parse(tmpFile);

    // Should have 3 connections (one per Dst point)
    EXPECT_EQ(model.m_connections.size(), 3);

    // All should have the same srcSid
    for (const auto & conn : model.m_connections)
    {
        EXPECT_EQ(conn.m_srcSid, 1);
    }

    // Check all destination SIDs are present
    std::vector<int> dstSids;
    for (const auto & conn : model.m_connections)
    {
        dstSids.push_back(conn.m_dstSid);
    }
    std::sort(dstSids.begin(), dstSids.end());
    EXPECT_EQ(dstSids[0], 2);
    EXPECT_EQ(dstSids[1], 3);
    EXPECT_EQ(dstSids[2], 4);

    removeTempFile(tmpFile);
}

TEST_F(XmlParserTest, UnknownBlockType)
{
    std::string xml =
        "<?xml version=\"1.0\"?>\n"
        "<System>"
        "  <Block Name=\"X\" SID=\"99\" BlockType=\"Integrator\"></Block>"
        "</System>";

    std::string tmpFile = "test_unknown.xml";
    writeTempFile(tmpFile, xml);

    EXPECT_EXIT(parser.parse(tmpFile), ::testing::ExitedWithCode(1), "");

    removeTempFile(tmpFile);
}

TEST_F(XmlParserTest, MissingFile)
{
    EXPECT_EXIT(parser.parse("nonexistent_file_xyz.xml"), ::testing::ExitedWithCode(1), "");
}
