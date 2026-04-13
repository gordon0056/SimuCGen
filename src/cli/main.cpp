#include "parser/XmlParser.h"
#include "graph/BlockGraph.h"
#include "scheduler/Scheduler.h"
#include "codegen/CodeGenerator.h"

#include <iostream>

static void printUsage(const char * progName)
{
    std::cerr << "Usage: " << progName << " <input.xml>\n";
    std::cerr << "  Converts a Simulink-like XML model to C code (nwocg_run.c)\n";
    std::cerr << "  Output is written to stdout.\n";
}

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        std::cerr << "[ERROR] CLI: no input file specified.\n";
        printUsage(argv[0]);
        return 1;
    }

    const std::string inputFile = argv[1];

    XmlParser parser;
    Model model = parser.parse(inputFile);

    BlockGraph graph(model);
    graph.build();
    graph.validate();

    Scheduler scheduler(graph, model);
    std::vector<int> schedule = scheduler.schedule();

    CodeGenerator generator(model, schedule);
    generator.emit(std::cout);

    return 0;
}
