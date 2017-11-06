#include "NonbinaryDSC.h"


CNonbinaryDSC::CNonbinaryDSC()
{
}


CNonbinaryDSC::~CNonbinaryDSC()
{
}

int CNonbinaryDSC::InitializeGraph(FACTOR_GRAPH &graph, uint32_t q)
{
	if ((graph.uiNumEdges != graph.edges.size()) || 
		(graph.uiNumEdges != graph.Hvalues.size()))
		return -1;

	m_uiq = q;
	m_uiQ = 1U << q;

	m_gf.Initialize(q);

	// reset the edges, check nodes and variables nodes in the graph
	m_aEdges.resize(graph.uiNumEdges);
	m_aCheckNodes.resize(graph.uiNumCheckNodes, CCheckNode());
	m_aVariableNodes.resize(graph.uiNumVariableNodes, CVariableNode());

	for (auto &node : m_aCheckNodes)
		node.ResetNode();

	for (auto &node : m_aVariableNodes)
		node.ResetNode();

	// initialize the edges
	for (uint32_t n = 0; n < graph.uiNumEdges; n++) {
		m_aEdges[n].adChktoVar.assign(m_uiQ, 0);
		m_aEdges[n].adVartoChk.assign(m_uiQ, 0);
		m_aEdges[n].auiPermutation.assign(m_uiQ, 0);
		m_aEdges[n].h = graph.Hvalues[n];

		for (uint8_t a = 0; a < m_uiQ; a++)
			m_aEdges[n].auiPermutation[a] = m_gf.Multiply(a, graph.Hvalues[n]);
	}

	// initialize the neighbors of the check nodes and variable nodes in 
	// the factor graph. edge = (check node, variable node)
	for (uint32_t n = 0; n < graph.uiNumEdges; n++) {
		m_aCheckNodes[graph.edges[n].first].AddEdge(&m_aEdges[n]);
		m_aVariableNodes[graph.edges[n].second].AddEdge(&m_aEdges[n]);
	}

	for (auto &node : m_aCheckNodes)
		node.Initialize(q, &m_gf);

	for (auto &node : m_aVariableNodes)
		node.Initialize(q, &m_gf);

	// resize and clear the rows of H
	m_uiSparseEntry.resize(graph.uiNumCheckNodes);
	m_dtSparseValues.resize(graph.uiNumCheckNodes);
	for (uint32_t n = 0; n < graph.uiNumCheckNodes; n++) {
		m_uiSparseEntry[n].clear();
		m_dtSparseValues[n].clear();
	}

	uint32_t chkIdx, varIdx;
	for (uint32_t n = 0; n < graph.edges.size(); n++) {
		chkIdx = graph.edges[n].first;
		varIdx = graph.edges[n].second;
		m_uiSparseEntry[chkIdx].push_back(varIdx);
		m_dtSparseValues[chkIdx].push_back(graph.Hvalues[n]);
	}

	return 0;
}

int CNonbinaryDSC::Decode(std::vector<uint8_t> &s, std::vector<uint8_t> &y, double adPx_y[],
	                       std::vector<uint8_t> &dec_x, uint32_t uiMaxIter)
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
		edge.adChktoVar.assign(m_uiQ, 0);
		edge.adVartoChk.assign(m_uiQ, 0);
		edge.dtDecValue = 0;
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
		[](CVariableNode &node) {return node.GetValue(); });

	return uiNumFailedChecks;
}

int CNonbinaryDSC::Encode(std::vector<uint8_t> &x, std::vector<uint8_t> &s)
{
	if (m_uiSparseEntry.size() != s.size())
		return 1;

	uint8_t syndrome;
	size_t n, m;
	for (n = 0; n < s.size(); n++) {
		syndrome = 0;
		for (m=0; m<m_dtSparseValues[n].size(); m++)
			syndrome ^= m_gf.Multiply(x[m_uiSparseEntry[n][m]], m_dtSparseValues[n][m]);

		s[n] = syndrome;
	}

	return 0;
}