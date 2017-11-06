#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <numeric>
#include <cstdlib>
#include <iostream> // C++ I/O library header

#include "PEGDesign.h"
#include "Random.h"

using namespace std;

CNodesInGraph::CNodesInGraph(void)
{
	connectionSymbolBit.reserve(10);
	connectionParityBit.reserve(10);
}

void CNodesInGraph::setNumOfConnectionSymbolBit(int deg)
{
	numOfConnectionSymbolBit = deg;
	connectionSymbolBit.resize(deg, 0);
}

void CNodesInGraph::initConnectionParityBit(int deg)
{
	maxDegParity = deg;
	numOfConnectionParityBit = 0;
	connectionParityBit.clear();
}

CNodesInGraph::~CNodesInGraph(void)
{
}

CPEGDesign::CPEGDesign(void) : verbose(true) { } //H(nullptr), 

int CPEGDesign::GenerateLDPCMatrix(int M, int N, int *symbolDegSequence, int sglConcent, int tgtGirth, bool verbose_)
{
	int i, j, k, m, index, localDepth = 100;

	m_ExpandDepth = (tgtGirth - 4) / 2;
	if (m_ExpandDepth < 0) m_ExpandDepth = 0;

	//	H = nullptr;
	verbose = verbose_;

	myrandom = new Random();  //(12345678l, 987654321lu);

	m_numChkNodes = M;
	m_numVarNodes = N;
	//	this->filename = filename;

	m_alocalGirth.resize(m_numVarNodes, 0);

	m_aNodesInGraph = new CNodesInGraph[m_numVarNodes];
	for (i = 0; i < m_numVarNodes; i++)
		m_aNodesInGraph[i].setNumOfConnectionSymbolBit(symbolDegSequence[i]);

	j = std::accumulate(symbolDegSequence, symbolDegSequence + m_numVarNodes, 0);
	k = j / m_numChkNodes;

	std::vector<int> mid(m_numChkNodes, k);
	for (i = 0; i < j - k*m_numChkNodes; i++) mid[i]++;

	for (i = 0; i < m_numChkNodes; i++) {
		if (sglConcent == 0) m_aNodesInGraph[i].initConnectionParityBit(mid[i]);
		else  m_aNodesInGraph[i].initConnectionParityBit();
	}

	for (k = 0; k < m_numVarNodes; k++)
	{
		m = 1000000;
		index = -1;
		for (i = 0; i < m_numChkNodes; i++) {
			if ((m_aNodesInGraph[i].numOfConnectionParityBit < m) &&
				(m_aNodesInGraph[i].numOfConnectionParityBit < m_aNodesInGraph[i].maxDegParity)) {
				m = m_aNodesInGraph[i].numOfConnectionParityBit;
				index = i;
			}
		}

		m_aNodesInGraph[k].connectionSymbolBit[0] = index;//least connections of parity bit

		int iter = 0;
		bool biterate = true;

		while (biterate && iter <= 30) {
			biterate = false;
			m_alocalGirth[k] = 100;

			for (m = 1; m < m_aNodesInGraph[k].numOfConnectionSymbolBit; m++) {
				m_aNodesInGraph[k].connectionSymbolBit[m] = selectParityConnect(k, m, localDepth);
				m_alocalGirth[k] = (m_alocalGirth[k] > localDepth) ? localDepth : m_alocalGirth[k];

				if ((((k > 0) && (m_alocalGirth[k] < m_alocalGirth[k - 1])) || (m_alocalGirth[k] == 0))
					&& (iter < 30)) {
					biterate = true;
					iter++;
					break;
				}
			}
		}

		if (verbose) {
			cout << "k=" << k << "  ";
			for (m = 0; m < m_aNodesInGraph[k].numOfConnectionSymbolBit; m++)
				cout << m_aNodesInGraph[k].connectionSymbolBit[m] << " ";
			cout << "LocalGirth=" << 2 * m_alocalGirth[k] + 4;
			cout << endl;
		}
		updateConnection(k);
	}

	if (verbose) {
		cout << "Showing the row weight distribution..." << endl;
		for (i = 0; i < m_numChkNodes; i++)
			cout << m_aNodesInGraph[i].numOfConnectionParityBit << " ";
		cout << endl;
	}

	ofstream cycleFile;
	cycleFile.open("leftHandGirth.log", ios::out);
	localDepth = 100;
	for (k = 0; k < m_numVarNodes; k++) {
		if (m_alocalGirth[k] < localDepth) localDepth = m_alocalGirth[k];
		if (localDepth == 100) cycleFile << "inf ";
		else cycleFile << 2 * localDepth + 4 << " ";
	}
	cycleFile << endl;
	cycleFile.close();

	if (verbose) {
		cout << "*************************************************************" << endl;
		cout << "       The global girth of the PEG Tanner graph :=" << 2 * localDepth + 4 << endl;
		cout << "*************************************************************" << endl;
	}

	// count number of edges in the graph
	int numEdges = 0;
	for (i = 0; i < m_numChkNodes; i++)
		numEdges += m_aNodesInGraph[i].numOfConnectionParityBit;

	return numEdges;
}

CPEGDesign::~CPEGDesign(void)
{
	delete[] m_aNodesInGraph;
	m_aNodesInGraph = nullptr;

	delete myrandom;
}

int CPEGDesign::selectParityConnect(int kthSymbol, int mthConnection, int & cycle)
{
	int i, j, k, index, mincycles, numCur, cpNumCur;
	int *tmp, *med;
	int *current;//take note of the covering parity bits

	mincycles = 0;
	tmp = new int[m_numChkNodes]; med = new int[m_numChkNodes];

	numCur = mthConnection;
	current = new int[mthConnection];
	for (i = 0; i < mthConnection; i++)
		current[i] = m_aNodesInGraph[kthSymbol].connectionSymbolBit[i];

LOOP:
	mincycles++;
	for (i = 0; i < m_numChkNodes; i++) tmp[i] = 0;
	//maintain 
	for (i = 0; i < mthConnection; i++)
		tmp[m_aNodesInGraph[kthSymbol].connectionSymbolBit[i]] = 1;

	for (i = 0; i < numCur; i++) {
		for (j = 0; j < m_aNodesInGraph[current[i]].numOfConnectionParityBit; j++) {
			for (k = 0; k < m_aNodesInGraph[m_aNodesInGraph[current[i]].connectionParityBit[j]].numOfConnectionSymbolBit; k++) {
				tmp[m_aNodesInGraph[m_aNodesInGraph[current[i]].connectionParityBit[j]].connectionSymbolBit[k]] = 1;
			}
		}
	}

	index = 0; cpNumCur = 0;
	for (i = 0; i < m_numChkNodes; i++) {
		if (tmp[i] == 1) cpNumCur++;
		if (tmp[i] == 1 || m_aNodesInGraph[i].numOfConnectionParityBit >= m_aNodesInGraph[i].maxDegParity)
			index++;
	}
	if (cpNumCur == numCur) {//can not expand any more
	  //additional handlement to select one having least connections
		j = 10000000; //dummy number
		for (i = 0; i < m_numChkNodes; i++) {
			if (tmp[i] == 0 && m_aNodesInGraph[i].numOfConnectionParityBit < j && m_aNodesInGraph[i].numOfConnectionParityBit < m_aNodesInGraph[i].maxDegParity)
				j = m_aNodesInGraph[i].numOfConnectionParityBit;
		}
		for (i = 0; i < m_numChkNodes; i++) {
			if (tmp[i] == 0) {
				if (m_aNodesInGraph[i].numOfConnectionParityBit != j || m_aNodesInGraph[i].numOfConnectionParityBit >= m_aNodesInGraph[i].maxDegParity) {
					tmp[i] = 1;
				}
			}
		}
		index = 0;
		for (i = 0; i < m_numChkNodes; i++) if (tmp[i] == 1) index++;
		//----------------------------------------------------------------
		j = (*myrandom).uniform(0, m_numChkNodes - index) + 1; //randomly selected
		index = 0;
		for (i = 0; i < m_numChkNodes; i++) {
			if (tmp[i] == 0) index++;
			if (index == j) break;
		}
		delete[] tmp; tmp = nullptr;
		delete[] current; current = nullptr;
		
		return(i);
	}
	else if (index == m_numChkNodes || mincycles > m_ExpandDepth) {
		//covering all parity nodes or meet the upper bound on cycles
		cycle = mincycles - 1;
		for (i = 0; i < m_numChkNodes; i++) tmp[i] = 0;
		for (i = 0; i < numCur; i++) tmp[current[i]] = 1;
		index = 0;
		for (i = 0; i < m_numChkNodes; i++) if (tmp[i] == 1) index++;
		if (index != numCur) { cout << "Error in the case of (index==m_numChkNodes)" << endl; exit(-1); }
		//additional handlement to select one having least connections
		j = 10000000;
		for (i = 0; i < m_numChkNodes; i++) {
			if (tmp[i] == 0 && m_aNodesInGraph[i].numOfConnectionParityBit < j && m_aNodesInGraph[i].numOfConnectionParityBit < m_aNodesInGraph[i].maxDegParity)
				j = m_aNodesInGraph[i].numOfConnectionParityBit;
		}
		for (i = 0; i < m_numChkNodes; i++) {
			if (tmp[i] == 0) {
				if (m_aNodesInGraph[i].numOfConnectionParityBit != j || m_aNodesInGraph[i].numOfConnectionParityBit >= m_aNodesInGraph[i].maxDegParity) {
					tmp[i] = 1;
				}
			}
		}

		index = 0;
		for (i = 0; i < m_numChkNodes; i++) if (tmp[i] == 1) index++;

		j = (*myrandom).uniform(0, m_numChkNodes - index) + 1;
		index = 0;
		for (i = 0; i < m_numChkNodes; i++) {
			if (tmp[i] == 0) index++;
			if (index == j) break;
		}
		delete[] tmp; tmp = nullptr;
		delete[] current; current = nullptr;

		return(i);
	}
	else if (cpNumCur > numCur && index != m_numChkNodes) {
		delete[] current;
		current = NULL;
		numCur = cpNumCur;
		current = new int[numCur];
		index = 0;
		for (i = 0; i < m_numChkNodes; i++) {
			if (tmp[i] == 1) { current[index] = i; index++; }
		}
		goto LOOP;
	}
	else
		return -1;

	return i;
}

void CPEGDesign::updateConnection(int kthSymbol) {
	int i, m;

	for (i = 0; i < m_aNodesInGraph[kthSymbol].numOfConnectionSymbolBit; i++) {
		m = m_aNodesInGraph[kthSymbol].connectionSymbolBit[i];//m [0, m_numChkNodes) parity node

		m_aNodesInGraph[m].connectionParityBit.push_back(kthSymbol);
		m_aNodesInGraph[m].numOfConnectionParityBit++;
	}
}

int CPEGDesign::GetSparseMatrix(uint32_t *pRowIdx, uint32_t *pColIdx)
{
	int i, j;
	int index = 0;
	for (i = 0; i < m_numChkNodes; i++)
		for (j = 0; j < m_aNodesInGraph[i].numOfConnectionParityBit; j++) {
			pRowIdx[index] = (uint32_t)i;
			pColIdx[index] = (uint32_t)j;
			index++;
		}

	return index;
}

void CPEGDesign::loadH(int **H) {
	int i, j;

	for (i = 0; i < m_numChkNodes; i++) {
		for (j = 0; j < m_numVarNodes; j++) {
			H[i][j] = 0;
		}
	}
	for (i = 0; i < m_numChkNodes; i++) {
		for (j = 0; j < m_aNodesInGraph[i].numOfConnectionParityBit; j++) {
			H[i][m_aNodesInGraph[i].connectionParityBit[j]] = 1;
		}
	}
}
