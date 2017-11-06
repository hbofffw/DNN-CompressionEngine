#pragma once

#include "bDSCdefinitions.h"

class CBinaryNode
{
public:
	CBinaryNode() {};
	~CBinaryNode() {};

	void ResetNode() {
		m_aEdges.clear();
	};

	void AddEdge(EDGE *pEdge) {
		m_aEdges.push_back(pEdge);
	};

	virtual void Initialize() {};
	virtual uint8_t ComputeMessage() { return 0; };

protected:
	std::vector<EDGE *> m_aEdges;
};

// check node
class CBinaryCheckNode :
	public CBinaryNode
{
public:
	CBinaryCheckNode();
	~CBinaryCheckNode();

	void Initialize();

	// decoding function
	uint8_t ComputeMessage() override;      // compute the outgoing message to each edge and return the syndorm error (0/1)
	void SetSyndrome(uint8_t &s) {
		m_uiS = s;
	};

private:
	uint8_t m_uiS;                              // syndrome
	std::vector<double> m_aTanhLLR;             // tanh(q/2) where q is the message received from variable node
};

// variable node
class CBinaryVariableNode :
	public CBinaryNode
{
public:
	CBinaryVariableNode();
	~CBinaryVariableNode();

	// decoding functions
	uint8_t ComputeMessage() override;      // compute the outgoing message to each edge and return the decoded value (0/1)
	void SetInitialProbability(double adPx_y[2][2], uint8_t &y);
	uint8_t GetValue() {
		return m_uiValue;
	};

private:
	double m_dq0;                 // log (P(x=0|y)/P(x=1|y))
	uint8_t m_uiValue;            // the decoded value at the variable node
};

