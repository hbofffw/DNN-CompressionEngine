#pragma once

#include <cstdint>

#include <algorithm>
#include <random>
#include <functional>
#include <vector>
#include <numeric>
#include <cmath>

#define epsilon  1e-10f

enum CompressionMethod {
	Raw,      // no compression
	FAC,      // fixed arithmetic coding, input frequency is provided
	AAC,      // adaptive arithmetic coding, no initial input frequency
	DISCUS    // distributed source coding using syndrome
};

enum QuantizationMethod {
	Binary,   // 1-bit quantization, sign(.)
	Uniform,  // uniform quantization, minimum value and step size are provided
	Dithered  // uniform quantization with dithered noise
};