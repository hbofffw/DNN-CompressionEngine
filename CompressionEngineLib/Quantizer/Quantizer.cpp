
#include "Quantizer.h"

// Quantizing the given input array with only 1 bit.The threshold is fixed to zero and 
// the reconstruction values are computed to minimize the MSE.
void OneBitQuantizer(float *afInput, uint32_t uiLen, float *afRec, uint8_t *abtQ)
{
	uint32_t n;
	uint32_t nP = 0, nM = 0;
	afRec[0] = 0;
	afRec[1] = 0;
	
	// find the sign of input
	for (n = 0; n < uiLen; n++)	{
		if (afInput[n] > 0) {
			abtQ[n] = 1;
			afRec[1] += afInput[n];
			nP++;
		}
		else {
			abtQ[n] = 0;
			afRec[0] += afInput[n];
			nM++;
		}
	}

	// compute reconstruction points
	afRec[0] /= (nM + epsilon);
	afRec[1] /= (nP + epsilon);
}


// Dequanitze from the given 1-bit quantization and the reconstruction values.
void OneBitDequantizer(uint8_t *abtQ, float *afRec, uint8_t uiLen, float *afOutput)
{
	for (uint32_t n = 0; n < uiLen; n++)
		afOutput[n] = afRec[abtQ[n]];
}

// quantize and dequantie functions for dithered quantization scheme. note that the 
// first bin reconstruction value is fMinValue + fStepSize/2. to reduce the computations,
// it is computed once and feeded to the function.
inline uint8_t QuantizeValue(float x, float &fMinValue, float &fStepSize, uint8_t &uiMaxQ)
{

	if (x <= fMinValue)
		return 0;
	else
		return __min(uiMaxQ, (uint8_t((x - fMinValue) / fStepSize)));
}

inline float DequantizeValue(uint8_t uiQ, float &fFirstBin, float &fStepSize)
{
	return fFirstBin + fStepSize * uiQ;
}

void DitheredQuantizer(float *afInput, float &fMinValue, float &fStepSize, uint8_t uiMaxQ, uint32_t uiLen, uint32_t &seed, uint8_t *abtQ)
{
	// uniform noise generatation
	std::mt19937 mt(seed);
	std::uniform_real_distribution<float> dist(-fStepSize/2, fStepSize/2);
	auto genNoise = std::bind(dist, mt);

	// the quantizer function
	auto quantizer = std::bind(QuantizeValue, std::placeholders::_1, fMinValue, fStepSize, uiMaxQ);
	std::transform(afInput, afInput + uiLen, abtQ, [&](float &x)->uint8_t {return  quantizer(x + genNoise()); });

	// update seed
	seed = uint32_t((genNoise() + fStepSize / 2) / fStepSize * (UINT32_MAX - 1));
}

void DitheredDequantizer(uint8_t *abtQ, float &fMinValue, float &fStepSize, uint8_t uiLen, uint32_t &seed, float *afOutput)
{
	// uniform noise generatation
	std::mt19937 mt(seed);
	std::uniform_real_distribution<float> dist(-fStepSize/2, fStepSize/2);
	auto genNoise = std::bind(dist, mt);

	float fFirstBin;
	fFirstBin = fMinValue + fStepSize / 2;
	// the dequantizer function
	auto dequantizer = std::bind(DequantizeValue, std::placeholders::_1, fFirstBin, fStepSize);
	std::transform(abtQ, abtQ + uiLen, afOutput, [&](uint8_t &q)->float {return (dequantizer(q) - genNoise()); });

	// update seed
	seed = uint32_t((genNoise() + fStepSize / 2) / fStepSize * (UINT32_MAX - 1));
}

void UniformQuantizer(float *afInput, float &fMinValue, float &fStepSize, uint8_t uiMaxQ, uint32_t uiLen, uint8_t *abtQ)
{
	// the quantizer function
	auto quantizer = std::bind(QuantizeValue, std::placeholders::_1, fMinValue, fStepSize, uiMaxQ);
	std::transform(afInput, afInput + uiLen, abtQ, [&](float &x)->uint8_t {return  quantizer(x); });
}

void UniformDequantizer(uint8_t *abtQ, float &fMinValue, float &fStepSize, uint8_t uiLen, float *afOutput)
{
	float fFirstBin;
	fFirstBin = fMinValue + fStepSize / 2;
	// the dequantizer function
	auto dequantizer = std::bind(DequantizeValue, std::placeholders::_1, fFirstBin, fStepSize);
	std::transform(abtQ, abtQ + uiLen, afOutput, [&](uint8_t &q)->float {return dequantizer(q); });
}
