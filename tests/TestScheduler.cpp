#include "scheduler/Scheduler.h"
#include "graph/BlockGraph.h"
#include "parser/XmlParser.h"
#include "model/Model.h"

#include <gtest/gtest.h>
#include <fstream>

class SchedulerTest : public ::testing::Test
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

TEST_F(SchedulerTest, PiControllerOrder)
{
    std::string xml =
        "<?xml version=\"1.0\"?>\n"
        "<System>"
        "  <Block Name=\"Ref\" SID=\"1\" BlockType=\"Inport\"><P Name=\"Port\">1</P></Block>"
        "  <Block Name=\"Fb\" SID=\"2\" BlockType=\"Inport\"><P Name=\"Port\">1</P></Block>"
        "  <Block Name=\"SumErr\" SID=\"3\" BlockType=\"Sum\"><P Name=\"Inputs\">+-</P></Block>"
        "  <Block Name=\"Kp\" SID=\"4\" BlockType=\"Gain\"><P Name=\"Gain\">2.5</P></Block>"
        "  <Block Name=\"Out\" SID=\"5\" BlockType=\"Outport\"><P Name=\"Port\">1</P></Block>"
        "  <Line><Src>1#R:1</Src><P Name=\"Dst\">3#L:1</P></Line>"
        "  <Line><Src>2#R:1</Src><P Name=\"Dst\">3#L:2</P></Line>"
        "  <Line><Src>3#R:1</Src><P Name=\"Dst\">4#L:1</P></Line>"
        "  <Line><Src>4#R:1</Src><P Name=\"Dst\">5#L:1</P></Line>"
        "</System>";

    std::string tmpFile = "test_pi_sched.xml";
    writeTempFile(tmpFile, xml);

    XmlParser xp;
    Model model = xp.parse(tmpFile);

    BlockGraph graph(model);
    graph.build();

    Scheduler sched(graph, model);
    std::vector<int> schedule = sched.schedule();

    // Expected: Inports first (1, 2), then Sum (3), then Gain (4), then Outport (5)
    ASSERT_EQ(schedule.size(), 5);

    // Inports should come before Sum
    auto inport1Pos = std::find(schedule.begin(), schedule.end(), 1);
    auto inport2Pos = std::find(schedule.begin(), schedule.end(), 2);
    auto sumPos = std::find(schedule.begin(), schedule.end(), 3);
    auto gainPos = std::find(schedule.begin(), schedule.end(), 4);

    EXPECT_LT(inport1Pos, sumPos);
    EXPECT_LT(inport2Pos, sumPos);
    EXPECT_LT(sumPos, gainPos);

    removeTempFile(tmpFile);
}

TEST_F(SchedulerTest, AlgebraicLoopDetection)
{
    std::string xml =
        "<?xml version=\"1.0\"?>\n"
        "<System>"
        "  <Block Name=\"A\" SID=\"10\" BlockType=\"Sum\"><P Name=\"Inputs\">++</P></Block>"
        "  <Block Name=\"B\" SID=\"20\" BlockType=\"Sum\"><P Name=\"Inputs\">+-</P></Block>"
        "  <Block Name=\"C\" SID=\"30\" BlockType=\"Sum\"><P Name=\"Inputs\">+-</P></Block>"
        "  <Line><Src>10#R:1</Src><P Name=\"Dst\">20#L:1</P></Line>"
        "  <Line><Src>20#R:1</Src><P Name=\"Dst\">30#L:1</P></Line>"
        "  <Line><Src>30#R:1</Src><P Name=\"Dst\">10#L:1</P></Line>"
        "</System>";

    std::string tmpFile = "test_loop.xml";
    writeTempFile(tmpFile, xml);

    XmlParser xp;
    Model model = xp.parse(tmpFile);

    BlockGraph graph(model);
    graph.build();

    Scheduler sched(graph, model);

    EXPECT_EXIT(sched.schedule(), ::testing::ExitedWithCode(1), "algebraic loop detected");

    removeTempFile(tmpFile);
}
