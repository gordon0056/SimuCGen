#pragma once

#include "graph/BlockGraph.h"
#include "model/Model.h"

#include <deque>
#include <vector>

/**
* @brief Deterministic topological scheduler (FIFO Kahn).
*
* Produces a schedule—a vector of block SIDs in evaluation order.
* UnitDelay blocks are scheduled early (in-degree = 0),
* but the CodeGenerator processes them with a separate flush pass at the end of step().
*/
class Scheduler
{
public:
    explicit Scheduler(const BlockGraph & graph, const Model & model);

    /**
	* @brief Executes Kahn's algorithm and returns a schedule.
	*
	* Algorithm: FIFO queue; initial nodes with in-degree=0 are sorted by SID
	* for determinism. Each time a node is retrieved, its SID is added to the result;
	* neighbors update their in-degree.
	*
	* @return Vector of SIDs in topological order
	* @post result.size() == model.m_blocks.size(); otherwise, algebraic loop → exit(1)
	*/
    std::vector<int> schedule();

private:
    const BlockGraph & m_graph;
    const Model & m_model;
};
