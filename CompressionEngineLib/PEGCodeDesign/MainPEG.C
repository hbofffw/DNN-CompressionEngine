/************************************************************************/
/*                                                                      */
/*        Free software: Progressive edge-growth (PEG) algorithm        */
/*        Created by Xiaoyu Hu                                          */
/*                   Evangelos Eletheriou                               */
/*                   Dieter Arnold                                      */
/*        IBM Research, Zurich Research Lab., Switzerland               */
/*                                                                      */
/*        The C++ sources files have been compiled using xlC compiler   */
/*        at IBM RS/6000 running AIX. For other compilers and platforms,*/
/*        minor changes might be needed.                                */
/*                                                                      */
/*        Bug reporting to: xhu@zurich.ibm.com                          */
/**********************************************************************/

////
// Modified by F. P. Beekhof; 2008 / 08 / 19
////

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <vector>
#include <numeric>
#include <algorithm>
#include "PEGDesign.h"
#include "Random.h"
#include "CyclesOfGraph.h"

const double EPS = 1e-6;

int main(int argc, char * argv[]) {
	int sglConcent = 1;  // default to non-strictly concentrated parity-check distribution
	int targetGirth = 100000; // default to greedy PEG version 
	std::string degFileName;
	int M = 600, N = 800;
	bool verbose = true;

	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-numM") == 0) {
			M = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-numN") == 0) {
			N = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-degFileName") == 0) {
			degFileName = argv[++i];
		}
		else if (strcmp(argv[i], "-sglConcent") == 0) {
			sglConcent = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-tgtGirth") == 0) {
			targetGirth = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-q") == 0) {
			verbose = false;
		}
	}
	if (M == -1 || N == -1) {
		std::cout << "Error: M or N not specified!" << std::endl;
		exit(-1);
	}
	if (M > N) {
		std::cout << "Error: M must be smaller than N!" << std::endl;
		exit(-1);
	}

	std::vector<int> degSeq(N);

	std::ifstream infn(degFileName.c_str());
	if (!infn) { std::cout << "\nCannot open file " << degFileName << std::endl; exit(-1); }
	
	int m;
	infn >> m;
	std::vector<int> deg(m);
	std::vector<double> degFrac(m);
	for (int i = 0; i < m; i++) infn >> deg[i];
	for (int i = 0; i < m; i++) infn >> degFrac[i];
	infn.close();

	double dtmp;
	std::partial_sum(degFrac.begin(), degFrac.end(), degFrac.begin());
	dtmp = degFrac.back();
	if (abs(dtmp - 1.0) > EPS) {
		std::cout.setf(std::ios::fixed, std::ios::floatfield);
		std::cout << "\n Invalid degree distribution (node perspective): sum != 1.0 but " << std::setprecision(10) << dtmp << std::endl; exit(-1);
	}

	std::transform(degFrac.begin(), degFrac.end(), degFrac.begin(), [&](double &x) {return __min(N-1, int(N*x)); });
	degFrac[degFrac.size() - 1] = N-1;
	int prvIdx = 0;
	int nxtIdx;

	for (int i = 0; i < deg.size(); i++) {
		nxtIdx = (int) degFrac[i];
		for (int j = prvIdx; j <= nxtIdx; j++)
			degSeq[j] = deg[i];

		prvIdx = nxtIdx+1;
	}

	CPEGDesign bigGirth;

	int numEdges = 0;
	numEdges = bigGirth.GenerateLDPCMatrix(M, N, degSeq.data(),	sglConcent, targetGirth, verbose);

	uint32_t *row, *col;
	row = new uint32_t[numEdges];
	col = new uint32_t[numEdges];
	bigGirth.GetSparseMatrix(row, col);

	//computing local girth distribution  
	if (verbose && N < 10000) {
		std::cout << " Now computing the local girth on the global Tanner graph setting. " << std::endl;
		std::cout << "     might take a bit long time. Please wait ...                   " << std::endl;
		int **H;
		H = new int*[M];
		for (int i = 0; i < M; i++) H[i] = new int[N];

		bigGirth.loadH(H);
		CyclesOfGraph cog(M, N, H);
		cog.getCyclesTable();
		cog.printCyclesTable();

		for (int i = 0; i < M; i++)
			delete[] H[i];
		
		delete[] H;
	}

	delete[] row;
	delete[] col;
}




