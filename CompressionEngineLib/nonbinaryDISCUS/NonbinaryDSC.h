#pragma once

#include "GaloisField.h"
#include "Node.h"

class CNonbinaryDSC
{
public:
	CNonbinaryDSC();
	~CNonbinaryDSC();

	int  InitializeGraph(FACTOR_GRAPH &graph, uint32_t q);

	int  Decode(std::vector<uint8_t> &s, std::vector<uint8_t> &y, double adPx_y[],
		std::vector<uint8_t> &dec_x, uint32_t uiMaxIter = 10);

	int  Encode(std::vector<uint8_t> &x, std::vector<uint8_t> &s);

private:
	uint32_t m_uiq;
	uint32_t m_uiQ;
	CGaloisField m_gf;

	// variables for decoding
	std::vector<EDGE> m_aEdges;
	std::vector<CCheckNode> m_aCheckNodes;
	std::vector<CVariableNode> m_aVariableNodes;

	// the sparse matrix, used for encoding
	std::vector<std::vector<uint32_t>>  m_uiSparseEntry;
	std::vector<std::vector<uint8_t>> m_dtSparseValues;
};

