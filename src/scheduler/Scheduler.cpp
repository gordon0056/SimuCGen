#include "scheduler/Scheduler.h"

#include <algorithm>
#include <deque>
#include <iostream>
#include <sstream>
#include <vector>

Scheduler::Scheduler(const BlockGraph & graph, const Model & model)
    : m_graph(graph)
    , m_model(model)
{
}

std::vector<int> Scheduler::schedule()
{
    std::map<int, int> inDegree = m_graph.inDegree();
    const auto & adj = m_graph.adjacency();

    std::vector<int> zeroNodes;
    for (const auto & block : m_model.m_blocks)
    {
        if (inDegree[block.m_sid] == 0)
        {
            zeroNodes.push_back(block.m_sid);
        }
    }
    std::sort(zeroNodes.begin(), zeroNodes.end());

    std::deque<int> queue(zeroNodes.begin(), zeroNodes.end());

    std::vector<int> result;
    result.reserve(m_model.m_blocks.size());

    while (!queue.empty())
    {
        int node = queue.front();
        queue.pop_front();
        result.push_back(node);

        auto it = adj.find(node);
        if (it != adj.end())
        {
            std::vector<int> neighbours = it->second;
            for (int neighbour : neighbours)
            {
                inDegree[neighbour]--;
                if (inDegree[neighbour] == 0)
                {
                    queue.push_back(neighbour);
                }
            }
        }
    }

    if (result.size() != m_model.m_blocks.size())
    {
        std::vector<int> unprocessed;
        for (const auto & block : m_model.m_blocks)
        {
            bool found = false;
            for (int sid : result)
            {
                if (sid == block.m_sid)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                unprocessed.push_back(block.m_sid);
            }
        }
        std::sort(unprocessed.begin(), unprocessed.end());

        std::ostringstream oss;
        oss << "[ERROR] BlockGraph: algebraic loop detected. SIDs:";
        for (int sid : unprocessed)
        {
            oss << " " << sid;
        }
        oss << "\n";
        std::cerr << oss.str();
        std::exit(1);
    }

    return result;
}
