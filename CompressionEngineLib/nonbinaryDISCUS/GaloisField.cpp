#include "GaloisField.h"



CGaloisField::CGaloisField()
{
	m_aLogQ = g_logq1;
	m_aExpQ = g_expq1;
	m_uiQ = 2;
}


CGaloisField::~CGaloisField()
{
}

int CGaloisField::Initialize(uint32_t q)
{
	if (q > MAX_q)
		return -1;

	m_uiQ = (1U) << q;
	uint32_t s = m_uiQ * sizeof(uint8_t);
	switch (q)
	{
	case 1:
		m_aLogQ = g_logq1;
		m_aExpQ = g_expq1;
		break;
	case 2:
		m_aLogQ = g_logq2;
		m_aExpQ = g_expq2;
		break;
	case 3:
		m_aLogQ = g_logq3;
		m_aExpQ = g_expq3;
		break;
	case 4:
		m_aLogQ = g_logq4;
		m_aExpQ = g_expq4;
		break;
	case 5:
		m_aLogQ = g_logq5;
		m_aExpQ = g_expq5;
		break;
	case 6:
		m_aLogQ = g_logq6;
		m_aExpQ = g_expq6;
		break;
	case 7:
		m_aLogQ = g_logq7;
		m_aExpQ = g_expq7;
		break;
	case 8:
		m_aLogQ = g_logq8;
		m_aExpQ = g_expq8;
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
