#include "parser/XmlParser.h"
#include "graph/BlockGraph.h"
#include "scheduler/Scheduler.h"
#include "codegen/CodeGenerator.h"
#include "model/Model.h"

#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <string>

#ifndef FIXTURE_DIR
#define FIXTURE_DIR "tests/fixtures"
#endif

#define FIXTURE_PATH(x) std::string(FIXTURE_DIR) + "/" + x

class IntegrationTest : public ::testing::Test
{
protected:
    static std::string readFile(const std::string & path)
    {
        std::ifstream ifs(path);
        std::stringstream ss;
        ss << ifs.rdbuf();
        return ss.str();
    }

    static std::string runCodegen(const std::string & xmlPath)
    {
        XmlParser parser;
        Model model = parser.parse(xmlPath);

        BlockGraph graph(model);
        graph.build();
        graph.validate();

        Scheduler sched(graph, model);
        std::vector<int> schedule = sched.schedule();

        CodeGenerator gen(model, schedule);
        std::ostringstream oss;
        gen.emit(oss);
        return oss.str();
    }
};

TEST_F(IntegrationTest, PiControllerEndToEnd)
{
    // This test verifies the full pipeline produces correct output
    std::string output = runCodegen(FIXTURE_PATH("pi_controller.xml"));

    // Check key structural elements
    EXPECT_NE(output.find("#include \"nwocg_run.h\""), std::string::npos);
    EXPECT_NE(output.find("#include <stddef.h>"), std::string::npos);
    EXPECT_NE(output.find("#include <math.h>"), std::string::npos);
    EXPECT_NE(output.find("static struct"), std::string::npos);
    EXPECT_NE(output.find("} nwocg;"), std::string::npos);
    EXPECT_NE(output.find("void nwocg_generated_init()"), std::string::npos);
    EXPECT_NE(output.find("void nwocg_generated_step()"), std::string::npos);
    EXPECT_NE(output.find("nwocg_generated_ext_ports"), std::string::npos);
    EXPECT_NE(output.find("nwocg_generated_ext_ports_size"), std::string::npos);

    // Check computation (Block names are priority 1, not Line names)
    EXPECT_NE(output.find("nwocg.SumError = nwocg.Ref - nwocg.Feedback"), std::string::npos);
    EXPECT_NE(output.find("nwocg.Kp = nwocg.SumError * 2.5"), std::string::npos);

    // Check ext_ports ordering
    size_t outPos = output.find("\"Output\"");
    size_t fbPos = output.find("\"Feedback\"");
    size_t refPos = output.find("\"Ref\"");
    size_t termPos = output.find("{ 0, 0, 0 }");

    EXPECT_NE(outPos, std::string::npos);
    EXPECT_NE(fbPos, std::string::npos);
    EXPECT_NE(refPos, std::string::npos);
    EXPECT_NE(termPos, std::string::npos);

    // Outport before Inports
    EXPECT_LT(outPos, fbPos);
    EXPECT_LT(outPos, refPos);
    // Terminator last
    EXPECT_LT(outPos, termPos);
    EXPECT_LT(fbPos, termPos);
}

TEST_F(IntegrationTest, AlgebraicLoopError)
{
    XmlParser parser;
    Model model = parser.parse(FIXTURE_PATH("algebraic_loop.xml"));

    BlockGraph graph(model);
    graph.build();

    Scheduler sched(graph, model);

    EXPECT_EXIT(sched.schedule(), ::testing::ExitedWithCode(1), "algebraic loop detected");
}

TEST_F(IntegrationTest, DeterminismCheck)
{
    std::string run1 = runCodegen(FIXTURE_PATH("pi_controller.xml"));
    std::string run2 = runCodegen(FIXTURE_PATH("pi_controller.xml"));

    EXPECT_EQ(run1, run2);
}

TEST_F(IntegrationTest, BranchParsing)
{
    std::string output = runCodegen(FIXTURE_PATH("branch_test.xml"));

    // Should have 3 outport signals
    EXPECT_NE(output.find("Dst1"), std::string::npos);
    EXPECT_NE(output.find("Dst2"), std::string::npos);
    EXPECT_NE(output.find("Dst3"), std::string::npos);
}

TEST_F(IntegrationTest, PiControllerWithDelay)
{
    // Full PI controller with UnitDelay: tests soft edges, init, and flush
    std::string output = runCodegen(FIXTURE_PATH("pi_controller_with_delay.xml"));

    // Init should zero UnitDelay
    EXPECT_NE(output.find("nwocg.UnitDelay1 = 0"), std::string::npos);

    // Computation expressions (Block names are priority 1)
    EXPECT_NE(output.find("nwocg.Add1 = nwocg.setpoint - nwocg.feedback"), std::string::npos);
    EXPECT_NE(output.find("nwocg.P_gain = nwocg.Add1 * 3"), std::string::npos);
    EXPECT_NE(output.find("nwocg.I_gain = nwocg.Add1 * 2"), std::string::npos);
    EXPECT_NE(output.find("nwocg.Ts = nwocg.I_gain * 0.01"), std::string::npos);
    EXPECT_NE(output.find("nwocg.Add2 = nwocg.Ts + nwocg.UnitDelay1"), std::string::npos);
    EXPECT_NE(output.find("nwocg.Add3 = nwocg.P_gain + nwocg.Add2"), std::string::npos);

    // UnitDelay flush at the end
    EXPECT_NE(output.find("nwocg.UnitDelay1 = nwocg.Add2"), std::string::npos);
}

TEST_F(IntegrationTest, UserNwocgModel)
{
    // Test the exact model from user's task: nwocg with branches and UnitDelay
    std::string output = runCodegen(FIXTURE_PATH("nwocg_user.xml"));

    // Init
    EXPECT_NE(output.find("nwocg.UnitDelay1 = 0"), std::string::npos);

    // Step expressions
    EXPECT_NE(output.find("nwocg.Add1 = nwocg.setpoint - nwocg.feedback"), std::string::npos);
    EXPECT_NE(output.find("nwocg.P_gain = nwocg.Add1 * 3"), std::string::npos);
    EXPECT_NE(output.find("nwocg.I_gain = nwocg.Add1 * 2"), std::string::npos);
    EXPECT_NE(output.find("nwocg.Ts = nwocg.I_gain * 0.01"), std::string::npos);
    EXPECT_NE(output.find("nwocg.Add2 = nwocg.Ts + nwocg.UnitDelay1"), std::string::npos);
    EXPECT_NE(output.find("nwocg.Add3 = nwocg.P_gain + nwocg.Add2"), std::string::npos);

    // UnitDelay flush
    EXPECT_NE(output.find("nwocg.UnitDelay1 = nwocg.Add2"), std::string::npos);

    // Ext ports: Outport first, then Inports
    EXPECT_NE(output.find("\"command\", &nwocg.Add3, 0"), std::string::npos);
    EXPECT_NE(output.find("\"feedback\", &nwocg.feedback, 1"), std::string::npos);
    EXPECT_NE(output.find("\"setpoint\", &nwocg.setpoint, 1"), std::string::npos);
}

TEST_F(IntegrationTest, BranchTest2Nested)
{
    // Test deeply nested branches (3 levels)
    std::string output = runCodegen(FIXTURE_PATH("branch_test2.xml"));

    // All 3 outports should point to the same Gain1 signal (Block name priority 1)
    EXPECT_NE(output.find("\"out1\", &nwocg.Gain1, 0"), std::string::npos);
    EXPECT_NE(output.find("\"out2\", &nwocg.Gain1, 0"), std::string::npos);
    EXPECT_NE(output.find("\"out3\", &nwocg.Gain1, 0"), std::string::npos);

    // Gain expression (Block name priority 1)
    EXPECT_NE(output.find("nwocg.Gain1 = nwocg.input * 5"), std::string::npos);
}

TEST_F(IntegrationTest, ComplexChainWithMultipleGains)
{
    // Complex chain: 2 Inports -> Sum -> 2 Gains -> Sum -> UnitDelay -> Sum -> Outport
    std::string output = runCodegen(FIXTURE_PATH("complex_chain.xml"));

    // Init (Block name priority 1)
    EXPECT_NE(output.find("nwocg.delay1 = 0"), std::string::npos);

    // Step (Block names: sum1, g1, g2, sum2, sum3, delay1)
    EXPECT_NE(output.find("nwocg.sum1 = nwocg.in1 + nwocg.in2"), std::string::npos);
    EXPECT_NE(output.find("nwocg.g1 = nwocg.sum1 * 2"), std::string::npos);
    EXPECT_NE(output.find("nwocg.g2 = nwocg.sum1 * 0.5"), std::string::npos);
    EXPECT_NE(output.find("nwocg.sum2 = nwocg.g1 - nwocg.g2"), std::string::npos);
    EXPECT_NE(output.find("nwocg.sum3 = nwocg.sum2 + nwocg.delay1"), std::string::npos);

    // UnitDelay flush
    EXPECT_NE(output.find("nwocg.delay1 = nwocg.sum2"), std::string::npos);
}
