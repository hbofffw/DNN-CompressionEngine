#include "FrequencyTable.h"



CFrequencyTable::CFrequencyTable()
{
	m_uiAlphabetSize = 0;
	m_uiTotal = 0;
	m_bValid = false;
}


CFrequencyTable::~CFrequencyTable()
{
}

void CFrequencyTable::Initialize(uint8_t uiAlphabetSize)
{
	m_uiAlphabetSize = uiAlphabetSize;
	m_aFreqs.assign(uiAlphabetSize, 1);
	m_aCDF.resize(uiAlphabetSize, 0);
	
	UpdateCDF();
}

void CFrequencyTable::Initialize(uint64_t *pFreq, uint8_t uiAlphabetSize)
{
	m_uiAlphabetSize = uiAlphabetSize;
	m_aFreqs.resize(uiAlphabetSize, 1);
	m_aCDF.resize(uiAlphabetSize, 0);

	std::copy(pFreq, pFreq + m_uiAlphabetSize, m_aFreqs.begin());
	UpdateCDF();
}


CFrequencyTable& CFrequencyTable::operator=(CFrequencyTable &table)
{
	if (this == &table)
		return *this;

	uint8_t  uiAlphabet;
	uint64_t *pFreq;

	uiAlphabet = table.GetAlphabetSize();
	pFreq = table.GetFrequencyTable();
	Initialize(pFreq, uiAlphabet);

	return *this;
}

//uint8_t CFrequencyTable::GetFrequencyTable(uint64_t* &pFreq)
//{
//	pFreq = m_aFreqs.data();
//
//	return m_uiAlphabetSize;
//}

void CFrequencyTable::UpdateCDF()
{
	std::partial_sum(m_aFreqs.begin(), m_aFreqs.end(), m_aCDF.begin());
	m_uiTotal = m_aCDF.back();

	m_bValid = true;
}

void CFrequencyTable::IncrementFreq(uint8_t &symbol)
{
	m_aFreqs[symbol] ++;
	m_uiTotal++;
	m_bValid = false;
}

void CFrequencyTable::GetRange(uint8_t &symbol, uint64_t &lower, uint64_t &upper, uint64_t &total)
{
	if (!m_bValid)
		UpdateCDF();
	
	lower = (symbol == 0) ? 0 : m_aCDF[symbol - 1];
	upper = m_aCDF[symbol];
	total = m_aCDF.back();
}
