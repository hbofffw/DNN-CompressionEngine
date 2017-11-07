#include "DecoderEngine.h"



CDecoderEngine::CDecoderEngine()
{
}


CDecoderEngine::~CDecoderEngine()
{
}

int CDecoderEngine::Initialize(CompressionMethod cMethod, QuantizationMethod qMethod, uint8_t uiQLevels, uint32_t uiRow, uint32_t uiCol, uint32_t uiInitialSeed)
{
	return 0;
}

int CDecoderEngine::Decode(uint8_t *auiInput, float *afqParams, uint32_t row, uint32_t col)
{
	return 0;
}


void CDecoderEngine::DecodeRawBinarySequence()
{
	int8_t bitPosition;
	uint32_t idx;
	
	//memcpy_s(dparam.data(), 2 * sizeof(float), code.data(), 2 * sizeof(float));
	//bitPosition = 8;
	//idx = dparam.size() * sizeof(float);
	//uint8_t q;
	//uint8_t mask;
	//mask = (1 << bpq) - 1;
	//for (size_t n = 0; n < dquant.size(); n++) {
	//	if (bitPosition >= bpq) {
	//		q = (code[idx] >> (bitPosition - bpq)) & mask;
	//	}
	//	else {
	//		q = ((code[idx] << (bpq - bitPosition)) & mask);
	//		q |= code[idx + 1] >> (8 - bpq + bitPosition);
	//	}

	//	dquant[n] = q;

	//	bitPosition -= bpq;
	//	if (bitPosition <= 0) {
	//		bitPosition += 8;
	//		idx++;
	//	}
	//}
}