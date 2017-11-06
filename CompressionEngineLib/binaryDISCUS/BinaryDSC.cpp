
#include "BinaryDSC.h"

CBinaryDSC::CBinaryDSC()
{
}


CBinaryDSC::~CBinaryDSC()
{
}

uint32_t CBinaryDSC::InitializeGraph(FACTOR_GRAPH &graph)
{
	if (graph.uiNumEdges != graph.edges.size())
		return 1;

	// reset the edges, check nodes and variables nodes in the graph
	m_aEdges.resize(graph.uiNumEdges);
	m_aCheckNodes.resize(graph.uiNumCheckNodes, CBinaryCheckNode());
	m_aVariableNodes.resize(graph.uiNumVariableNodes, CBinaryVariableNode());

	for (auto &node : m_aCheckNodes)
		node.ResetNode();

	for (auto &node : m_aVariableNodes)
		node.ResetNode();

	// initialize the neighbors of the check nodes and variable nodes in 
	// the factor graph. edge = (check node, variable node)
	for (uint32_t n = 0; n < graph.uiNumEdges; n++) {
		m_aCheckNodes[graph.edges[n].first].AddEdge(&m_aEdges[n]);
		m_aVariableNodes[graph.edges[n].second].AddEdge(&m_aEdges[n]);
	}

	for (auto &node : m_aCheckNodes)
		node.Initialize();

	for (auto &node : m_aVariableNodes)
		node.Initialize();

	// resize and clear the rows of H
	m_uiSparseH.resize(graph.uiNumCheckNodes);
	for (auto &row : m_uiSparseH)
		row.clear();

	for (auto &edge : graph.edges)
		m_uiSparseH[edge.first].push_back(edge.second);

	return 0;
}

uint32_t CBinaryDSC::Decode(std::vector<uint8_t> &s, std::vector<uint8_t> &y, 
						    double adPx_y[2][2], std::vector<uint8_t> &dec_x, uint32_t uiMaxIter)
{
	// check the size of the inputs and output
	if (s.size() != m_aCheckNodes.size())
		return 1;

	if ((y.size() != m_aVariableNodes.size()) || (y.size() != dec_x.size()))
		return 2;

	uint32_t n;
	uint32_t uiNumFailedChecks;

	// initialize the nodes of the factor graph
	for (n = 0; n < s.size(); n++)
		m_aCheckNodes[n].SetSyndrome(s[n]);

	for (n = 0; n < y.size(); n++)
		m_aVariableNodes[n].SetInitialProbability(adPx_y, y[n]);

	// initialize the messages
	for (auto &edge : m_aEdges) {
		edge.ChktoVar = 0;
		edge.VartoChk = 0;
		edge.uiDecValue = 0;
	}

	// run message passing algorithm
	for (n = 0; n < uiMaxIter; n++) {
		uiNumFailedChecks = 0;

		// compute message at the variable nodes
		for (auto &node : m_aVariableNodes)
			node.ComputeMessage();

		// compute messages at the check nodes and verify if the decoded values 
		// satisfy the syndrome
		for (auto &node : m_aCheckNodes)
			uiNumFailedChecks += node.ComputeMessage();

		if (uiNumFailedChecks == 0)
			break;
	}

	// copy the decoded values to the output
	std::transform(m_aVariableNodes.begin(), m_aVariableNodes.end(), dec_x.begin(),
		[](CBinaryVariableNode &node) {return node.GetValue(); });

	return uiNumFailedChecks;
}

uint32_t CBinaryDSC::Encode(std::vector<uint8_t> &x, std::vector<uint8_t> &s)
{
	if (m_uiSparseH.size() != s.size())
		return 1;

	uint8_t syndrome;
	for (size_t n = 0; n < s.size(); n++) {
		syndrome = 0;
		for (auto &idx : m_uiSparseH[n])
			syndrome ^= x[idx];

		s[n] = syndrome;
	}

	return 0;
}