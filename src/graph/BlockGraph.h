#pragma once

#include "model/Model.h"

#include <map>
#include <vector>

/**
* @brief Builds a directed graph of blocks based on the Model.
*
* UnitDelay blocks are treated specially: their edges are considered
* "soft" and are not counted in the in-degree for the Kahn algorithm.
* This allows the scheduler to process the UnitDelay before its provider.
*/
class BlockGraph
{
public:
    explicit BlockGraph(const Model & model);

    /**
	* @brief Builds an adjacency list and an in-degree table.
	* Edges in UnitDelay blocks: included in adjacency, but NOT in-degree.
	* After construction, the neighbors of each node are sorted by SID; duplicates are removed.
	*/
    void build();

    /**
	* @brief Checks graph integrity: all SIDs in the Connection exist in the Model.
	* @note Call after build().
	*/
    void validate();

    const std::map<int, std::vector<int>> & adjacency() const;
    const std::map<int, int> & inDegree() const;

private:
    const Model & m_model;
    std::map<int, std::vector<int>> m_adj;    ///< Adjacency list; values ​​are sorted by SID
    std::map<int, int> m_inDegree;            ///< In-degree; soft edges are not taken into account
};
