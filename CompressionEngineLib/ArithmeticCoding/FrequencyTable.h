#pragma once

#include "ACdefinitions.h"

// Note: No checking is done on the range of input data and stored variables
class CFrequencyTable
{
public:
	CFrequencyTable();
	~CFrequencyTable();

	void Initialize(uint8_t uiAlphabetSize);
	void Initialize(uint64_t *pFreq, uint8_t uiAlphabetSize);
	CFrequencyTable& operator= (CFrequencyTable & table);

	void IncrementFreq(uint8_t &symbol);
	void UpdateCDF();

	void GetRange(uint8_t &symbol, uint64_t &lower, uint64_t &upper, uint64_t &total);

	uint8_t   GetAlphabetSize() { return m_uiAlphabetSize; }
	uint64_t* GetFrequencyTable() { return m_aFreqs.data(); }
	uint64_t  GetTotal() { return m_uiTotal; }

private:
	uint8_t  m_uiAlphabetSize;         // number of symbols
	uint64_t m_uiTotal;                // sum of frequencies
	std::vector<uint64_t> m_aFreqs;    // frequency of each symbol
	std::vector<uint64_t> m_aCDF;      // cumulative frequencies

	bool m_bValid;                     // are the cumulative frequencies valid?
};
