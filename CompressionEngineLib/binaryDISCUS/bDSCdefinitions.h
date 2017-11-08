#pragma once

#include "../General.h"

struct FACTOR_GRAPH {
	uint32_t  uiNumCheckNodes;
	uint32_t  uiNumVariableNodes;
	uint32_t  uiNumEdges;

	std::vector<std::pair<uint32_t, uint32_t>> edges;
};

struct EDGE {
	double  ChktoVar;
	double  VartoChk;

	uint8_t uiDecValue;
};