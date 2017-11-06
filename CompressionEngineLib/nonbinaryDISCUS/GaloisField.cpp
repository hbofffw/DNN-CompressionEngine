#include "GaloisField.h"



CGaloisField::CGaloisField()
{
	m_aLogQ = nullptr;
	m_aExpQ = nullptr;
}


CGaloisField::~CGaloisField()
{
	ReleaseMemory();
}

void CGaloisField::ReleaseMemory()
{
	if (!m_aLogQ) {
		delete[] m_aLogQ;
		m_aLogQ = nullptr;
	}

	if (!m_aExpQ) {
		delete[] m_aExpQ;
		m_aExpQ = nullptr;
	}
}

int CGaloisField::Initialize(uint32_t q)
{
	if (q > MAX_q)
		return -1;

	ReleaseMemory();

	m_uiQ = (1U) << q;
	m_aLogQ = new uint8_t[m_uiQ];
	m_aExpQ = new uint8_t[m_uiQ - 1];

	uint32_t s = m_uiQ * sizeof(uint8_t);
	switch (q)
	{
	case 1:
		memcpy(m_aLogQ, g_logq1, s);
		memcpy(m_aExpQ, g_expq1, s - 1);
		break;
	case 2:
		memcpy(m_aLogQ, g_logq2, s);
		memcpy(m_aExpQ, g_expq2, s - 1);
		break;
	case 3:
		memcpy(m_aLogQ, g_logq3, s);
		memcpy(m_aExpQ, g_expq3, s - 1);
		break;
	case 4:
		memcpy(m_aLogQ, g_logq4, s);
		memcpy(m_aExpQ, g_expq4, s - 1);
		break;
	case 5:
		memcpy(m_aLogQ, g_logq5, s);
		memcpy(m_aExpQ, g_expq5, s - 1);
		break;
	case 6:
		memcpy(m_aLogQ, g_logq6, s);
		memcpy(m_aExpQ, g_expq6, s - 1);
		break;
	case 7:
		memcpy(m_aLogQ, g_logq7, s);
		memcpy(m_aExpQ, g_expq7, s - 1);
		break;
	case 8:
		memcpy(m_aLogQ, g_logq8, s);
		memcpy(m_aExpQ, g_expq8, s - 1);
		break;
	default:
		return -2;
	}

	return 0;
}

uint8_t  CGaloisField::Multiply(uint8_t a, uint8_t b)
{
	if (a == 0 || b == 0)
		return 0;

	if (a == 1)
		return b;

	if (b == 1)
		return a;

	return (m_aExpQ[(m_aLogQ[a] + m_aLogQ[b]) % (m_uiQ - 1)]);
}

uint8_t  CGaloisField::Divide(uint8_t a, uint8_t b)
{
	if (a == 0)
		return 0;

	if (b == 1)
		return a;

	return (m_aExpQ[(m_aLogQ[a] + m_aLogQ[b]) % (m_uiQ - 1)]);
}
