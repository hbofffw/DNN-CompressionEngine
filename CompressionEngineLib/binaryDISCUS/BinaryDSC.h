#pragma once

#include "BinaryNode.h"

class CBinaryDSC
{
public:
	CBinaryDSC();
	~CBinaryDSC();

	uint32_t InitializeGraph(FACTOR_GRAPH &graph);

	uint32_t Decode(std::vector<uint8_t> &s, std::vector<uint8_t> &y, double adPx_y[2][2],
		     std::vector<uint8_t> &dec_x, uint32_t uiMaxIter = 10);

	uint32_t Encode(std::vector<uint8_t> &x, std::vector<uint8_t> &s);

private:
	// variables for decoding
	std::vector<EDGE> m_aEdges;
	std::vector<CBinaryCheckNode> m_aCheckNodes;
	std::vector<CBinaryVariableNode> m_aVariableNodes;

	// the sparse matrix, used for encoding
	std::vector<std::vector<uint32_t>> m_uiSparseH;
};

