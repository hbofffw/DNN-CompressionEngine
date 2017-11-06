#pragma once

#include "nDSCdefinitions.h"

class CGaloisField
{
public:
	CGaloisField();
	~CGaloisField();

	int   Initialize(uint32_t q = 1U);
	uint8_t Multiply(uint8_t a, uint8_t b);
	uint8_t Divide(uint8_t a, uint8_t b);
	uint8_t Add(uint8_t a, uint8_t b) {
		return (a^b);
	}

private:
	void ReleaseMemory();

	uint32_t m_uiQ;

	uint8_t*  m_aLogQ;
	uint8_t*  m_aExpQ;
};

