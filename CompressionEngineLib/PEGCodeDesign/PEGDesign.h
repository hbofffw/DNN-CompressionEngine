#pragma once

#include <vector>
#include "Random.h"


class CNodesInGraph
{
public:
	CNodesInGraph(void);
	~CNodesInGraph(void);

	void setNumOfConnectionSymbolBit(int deg);
	void initConnectionParityBit(int deg = 10000);

	int numOfConnectionParityBit;
	std::vector<int> connectionParityBit;

	int numOfConnectionSymbolBit;
	std::vector<int> connectionSymbolBit;
	
	int maxDegParity;
};

class CPEGDesign
{
public:
	CPEGDesign(void);
	~CPEGDesign(void);

	int GenerateLDPCMatrix(int M, int N, int *symbolDegSequence, int sglConcent, int tgtGirth, bool verbose_ = true);
	int GetSparseMatrix(uint32_t *pRowIdx, uint32_t *pColIdx);

	void loadH(int **H);

private:
	int selectParityConnect(int kthSymbol, int mthConnection, int & cycle);
	void updateConnection(int kthSymbol);
	bool verbose;

	std::vector<int> m_alocalGirth;
	int m_numChkNodes;
	int m_numVarNodes;
	int m_ExpandDepth;
	CNodesInGraph *m_aNodesInGraph;
	Random *myrandom;
};
