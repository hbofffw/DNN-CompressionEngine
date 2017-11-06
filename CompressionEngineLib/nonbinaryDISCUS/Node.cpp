#include "Node.h"
#include <numeric>
#include <algorithm>

#pragma region implementations of the base CNode clase

uint32_t CNode::m_uiq = 1U;
uint32_t CNode::m_uiQ = 2U;
CGaloisField* CNode::m_gf = nullptr;

CNode::CNode()
{
}


CNode::~CNode()
{
}

#pragma endregion

///////////////////////////////////////////////////////////////////////////////
#pragma region implementation of the CCheckNode class, computations at the check nodes
CCheckNode::CCheckNode()
{
}


CCheckNode::~CCheckNode()
{
}

void CCheckNode::Initialize(uint32_t q, CGaloisField *gf)
{
	CNode::Initialize(q, gf);

	// resize the vectors
	m_adUpc.resize(m_iDegree, std::vector<double>(m_uiQ, 0));
	m_adVtp.resize(m_iDegree, std::vector<double>(m_uiQ, 0));
}

void  CCheckNode::ApplyFFT(std::vector<double> &x, std::vector<double> &y)
{
	uint32_t  a, step, idx0, idx1, sep;

	// computation on LSB first to initialize y
	for (a = 0; a < (m_uiQ / 2); a++) {
		idx0 = 2 * a;
		idx1 = idx0 + 1;
		y[idx0] = x[idx0] + x[idx1];
		y[idx1] = x[idx0] - x[idx1];
	}

	double v1;
	// computing FFT on the remaining of bit positions
	for (step = 1; step < m_uiq; step++) {
		sep = 1 << step;
		for (a = 0; a < (m_uiQ / 2); a++) {
			idx0 = 2 * a - (a % sep);
			idx1 = idx0 + sep;
			v1 = y[idx0];
			y[idx0] += y[idx1];
			y[idx1] = v1 - y[idx1];
		}
	}

	// divide by 1/sqrt(2) ^ q, not necessary at the moment.
}

// in-place FFT computation
void  CCheckNode::ApplyFFT(std::vector<double> &x)
{
	uint32_t  a, step, idx0, idx1, sep;
	double v1;

	// computing FFT on the remaining of bit positions
	for (step = 0; step < m_uiq; step++) {
		sep = 1 << step;
		for (a = 0; a < (m_uiQ / 2); a++) {
			idx0 = 2 * a - (a % sep);
			idx1 = idx0 + sep;
			v1 = x[idx0];
			x[idx0] += x[idx1];
			x[idx1] = v1 - x[idx1];
		}
	}

	// divide by 1/sqrt(2) ^ q, not necessary at the moment.
}

uint8_t CCheckNode::ComputeMessage()
{
	uint8_t a;
	int n;

	// compute Upc and its FFT
	for (n = 0; n < m_iDegree; n++) {
		// 1- permutation of the incoming message
		if (n == 0)
			for (a = 0; a < m_uiQ; a++)
				m_adUpc[n][m_aEdges[n]->auiPermutation[a] ^ m_dtS] = exp(m_aEdges[n]->adVartoChk[a]);
		else
			for (a = 0; a < m_uiQ; a++)
				m_adUpc[n][m_aEdges[n]->auiPermutation[a]] = exp(m_aEdges[n]->adVartoChk[a]);

		// 2- compute FFT
		ApplyFFT(m_adUpc[n]);
	}

	// compute the outgoing message
	// 1- element-wise product of the FFT of permuted incoming message
	double p;
	for (a = 0; a < m_uiQ; a++) {
		// get the products below the current index
		p = 1.0;
		for (n = 0; n < m_iDegree; n++) {
			m_adVtp[n][a] = p;
			p *= m_adUpc[n][a];
		}
		// get the products above the current index
		p = 1.0;
		for (n = m_iDegree - 1; n >= 0; n--) {
			m_adVtp[n][a] *= p;
			p *= m_adUpc[n][a];
		}
	}

	// 2- compute FFT
	for (n = 0; n < m_iDegree; n++)
		ApplyFFT(m_adVtp[n]);

	// update the message, since division by 1/2^q is ignored in the computation of FFT, apply it here
	// inverse of permutation
	for (n = 0; n < m_iDegree; n++)
		if (n == 0)
			for (a = 0; a < m_uiQ; a++)
				m_aEdges[n]->adChktoVar[a] = log(m_adVtp[n][m_aEdges[n]->auiPermutation[a] ^ m_dtS] / m_uiQ + epsilon);
		else
			for (a = 0; a < m_uiQ; a++)
				m_aEdges[n]->adChktoVar[a] = log(m_adVtp[n][m_aEdges[n]->auiPermutation[a]] / m_uiQ + epsilon);

	// compute the error in syndrome
	uint8_t err = 0;
	for (auto &edge : m_aEdges) {
		err ^= m_gf->Multiply(edge->dtDecValue, edge->h);
	}

	err ^= m_dtS;
	return (err != 0) ? 1 : 0;
}

#pragma endregion

///////////////////////////////////////////////////////////////////////////////
#pragma region implementation of the CVariableNode class, computations at the varaible nodes
CVariableNode::CVariableNode()
{
}


CVariableNode::~CVariableNode()
{
}

void CVariableNode::Initialize(uint32_t q, CGaloisField *gf)
{
	CNode::Initialize(q, gf);

	m_adLogP0.resize(m_uiQ, 0);
	m_adTotalLL.resize(m_uiQ, 0);
}

void CVariableNode::SetInitialProbability(double adPx_y[], uint8_t &y)
{
	// Px_y stored originally as P[x][y] = P[x*Q+y]
	for (uint8_t a = 0; a < m_uiQ; a++)
		m_adLogP0[a] = log(adPx_y[a*m_uiQ + y]);
}

uint8_t CVariableNode::ComputeMessage()
{
	double sum;
	// compute log-likelihood of each symbol based on all incoming messages to the variable node
	// LL[a] = log(P0(a)) + sum_e log(V_e[a])
	m_adTotalLL = m_adLogP0;
	for (auto edge : m_aEdges) {
		std::transform(m_adTotalLL.begin(), m_adTotalLL.end(), edge->adChktoVar.begin(),
			m_adTotalLL.begin(), std::plus<double>());
	}

	// compute un-normalized outgoing messages for each edge
	for (auto edge : m_aEdges)
		std::transform(m_adTotalLL.begin(), m_adTotalLL.end(), edge->adChktoVar.begin(),
			edge->adVartoChk.begin(), std::minus<double>());

	// decode based on the maximum log-likelihood
	auto it = std::max_element(m_adTotalLL.begin(), m_adTotalLL.end());
	m_uiValue = (uint8_t) std::distance(m_adTotalLL.begin(), it);

	// normalize messages to be probability distribution
	double dmax;
	for (auto edge : m_aEdges) {
		edge->dtDecValue = m_uiValue;

		dmax = *(std::max_element(edge->adVartoChk.begin(), edge->adVartoChk.end()));
		sum = std::accumulate(edge->adVartoChk.begin(), edge->adVartoChk.end(), 0.0,
			[&](const double &current_sum, const double& x) {return current_sum + exp(x - dmax); });
		sum = log(sum) + dmax;                // note that always sum>=1 from previous step
		std::transform(edge->adVartoChk.begin(), edge->adVartoChk.end(), edge->adVartoChk.begin(),
			std::bind2nd(std::minus<double>(), sum));
	}

	return m_uiValue;
}

#pragma endregion