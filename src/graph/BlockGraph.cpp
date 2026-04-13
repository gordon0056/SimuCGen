#include "graph/BlockGraph.h"

#include <algorithm>
#include <iostream>
#include <sstream>

BlockGraph::BlockGraph(const Model & model)
    : m_model(model)
{
}

void BlockGraph::build()
{
    for (const auto & block : m_model.m_blocks)
    {
        m_adj[block.m_sid] = std::vector<int>();
        m_inDegree[block.m_sid] = 0;
    }

    for (const auto & conn : m_model.m_connections)
    {
        auto it = m_model.m_sidToIndex.find(conn.m_dstSid);
        if (it == m_model.m_sidToIndex.end())
            continue;

        const Block & dstBlock = m_model.m_blocks[it->second];
        bool isSoftEdge = (dstBlock.m_type == BlockType::UNIT_DELAY);

        m_adj[conn.m_srcSid].push_back(conn.m_dstSid);

        if (!isSoftEdge)
        {
            m_inDegree[conn.m_dstSid]++;
        }
    }

    for (auto & pair : m_adj)
    {
        std::sort(pair.second.begin(), pair.second.end());
        pair.second.erase(
            std::unique(pair.second.begin(), pair.second.end()),
            pair.second.end()
        );
    }

    for (auto & pair : m_inDegree)
    {
        pair.second = 0;
    }

    for (const auto & pair : m_adj)
    {
        for (int neighbour : pair.second)
        {
            auto it = m_model.m_sidToIndex.find(neighbour);
            if (it != m_model.m_sidToIndex.end())
            {
                const Block & dstBlock = m_model.m_blocks[it->second];
                if (dstBlock.m_type != BlockType::UNIT_DELAY)
                {
                    m_inDegree[neighbour]++;
                }
            }
        }
    }
}

void BlockGraph::validate()
{
    for (const auto & conn : m_model.m_connections)
    {
        if (m_model.m_sidToIndex.find(conn.m_srcSid) == m_model.m_sidToIndex.end())
        {
            std::cerr << "[ERROR] BlockGraph: connection references non-existent SID="
                      << conn.m_srcSid << "\n";
            std::exit(1);
        }
        if (m_model.m_sidToIndex.find(conn.m_dstSid) == m_model.m_sidToIndex.end())
        {
            std::cerr << "[ERROR] BlockGraph: connection references non-existent SID="
                      << conn.m_dstSid << "\n";
            std::exit(1);
        }
    }
}

const std::map<int, std::vector<int>> & BlockGraph::adjacency() const
{
    return m_adj;
}

const std::map<int, int> & BlockGraph::inDegree() const
{
    return m_inDegree;
}
