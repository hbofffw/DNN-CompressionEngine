
#include "BinaryNode.h"

///////////////////////////////////////////////////////////////////////////////
#pragma region implementation of the CBinaryCheckNode class, computations at the check nodes
CBinaryCheckNode::CBinaryCheckNode()
{
}


CBinaryCheckNode::~CBinaryCheckNode()
{
}

void CBinaryCheckNode::Initialize()
{
	m_aTanhLLR.resize(m_aEdges.size(), 0.0);
}

uint8_t CBinaryCheckNode::ComputeMessage()
{
	int n;
	double p;

	std::transform(m_aEdges.begin(), m_aEdges.end(), m_aTanhLLR.begin(),
		[](EDGE* edge) {return tanh(edge->VartoChk / 2); });

	p = 1.0;
	// get the products below the current index
	for (n = 0; n < m_aEdges.size(); n++) {
		m_aEdges[n]->ChktoVar = p;
		p *= m_aTanhLLR[n];
	}

	// get the products above the current index
	p = 1.0;
	for (n = m_aEdges.size() - 1; n >= 0; n--) {
		m_aEdges[n]->ChktoVar *= p;
		p *= m_aTanhLLR[n];
	}

	// compute the arctanh and verify that the received decoded values 
	// satisfy the check node 
	uint8_t err = 0;
	for (auto &edge : m_aEdges) {
		edge->ChktoVar = 2 * atanh((1.0 - 2 * m_uiS) * edge->ChktoVar);
		err ^= edge->uiDecValue;
	}

	err ^= m_uiS;
	return err;
}

#pragma endregion

///////////////////////////////////////////////////////////////////////////////
#pragma region implementation of the CBinaryCheckNode class, computations at the check nodes
CBinaryVariableNode::CBinaryVariableNode()
{
}


CBinaryVariableNode::~CBinaryVariableNode()
{
}

void CBinaryVariableNode::SetInitialProbability(double adPx_y[2][2], uint8_t &y)
{
	m_dq0 = log(adPx_y[0][y] / adPx_y[1][y]);
}

uint8_t CBinaryVariableNode::ComputeMessage()
{
	double sum;

	sum = std::accumulate(m_aEdges.begin(), m_aEdges.end(), m_dq0,
		[](const double &current_sum, const EDGE* edge) {return current_sum + edge->ChktoVar; });

	// compute the decoded value
	m_uiValue = (sum > 0) ? 0 : 1;
	for (auto &edge : m_aEdges) {
		edge->VartoChk = sum - edge->ChktoVar;
		edge->uiDecValue = m_uiValue;
	}

	return m_uiValue;
}

#pragma endregion