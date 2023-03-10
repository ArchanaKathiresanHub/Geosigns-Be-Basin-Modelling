#include <algorithm>
#include <math.h>
#include <limits.h>
#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "DistributedGrid.h"
#include "decompositionCalculator.h"
#include "LogHandler.h"

#define Round(a) ((int) (((double) a) + 0.5))

using namespace DataAccess;

using namespace Interface;


bool DistributedGrid::checkForValidPartitioning(int M, int N)
{
	int mSelected;
	int nSelected;

	int numberOfCores;
	MPI_Comm_size(PETSC_COMM_WORLD, &numberOfCores);
	return DecompositionCalculator::calculateStaticDecomposition(M, N, mSelected, nSelected, numberOfCores);
}

DistributedGrid::DistributedGrid(double minI, double minJ,
	double maxI, double maxJ, int numI, int numJ, int lowResNumI, int lowResNumJ, const std::vector<std::vector<int>>& domainShape) :
	m_globalGrid(minI, minJ, maxI, maxJ, numI, numJ), m_numProcsI(-1), m_numProcsJ(-1), m_numsI(0), m_numsJ(0),
	m_returnsI(0), m_returnsJ(0)
{
	int totalNumberOfCores;
	MPI_Comm_size(PETSC_COMM_WORLD, &totalNumberOfCores);

	// Calculate the core partitioning based on the low resolution output grid as that is the grid mostly used.
	// If there is no core partitioning for the low res grid, use the high res grid and hope the low res grid will not be used (checked elsewhere).
	if (lowResNumI <= 1 or lowResNumJ <= 1) {
		PetscPrintf(PETSC_COMM_WORLD,
			"\nBasin_Error: Unable to partition a %d x %d grid, please increase the number of grid nodes.\nThere must be at least two nodes in each direction \n", lowResNumI, lowResNumJ);
		PetscPrintf(PETSC_COMM_WORLD, "\nExiting ...\n\n");

		MPI_Finalize();
		exit(-1);
	}

	int mpiRank;
	MPI_Comm_rank(PETSC_COMM_WORLD, &mpiRank);

	int numICores = 0;
	int numJCores = 0;
	std::vector<int> cellSizesI;
	std::vector<int> cellSizesJ;
	int dynamicDecomposition = 0;
	int foundValidDecomposition = 0;
	if (mpiRank == 0)
	{
		double maxWidthDeviationPercentageForDynamicDecomposition = 25.0;
		DecompositionCalculator decompositionCalculator(domainShape, totalNumberOfCores, numIGlobal(), numJGlobal(), maxWidthDeviationPercentageForDynamicDecomposition, lowResNumI, lowResNumJ);
		foundValidDecomposition = decompositionCalculator.calculateDecomposition(numICores, numJCores, cellSizesI, cellSizesJ);
		dynamicDecomposition = !cellSizesI.empty() && !cellSizesJ.empty();
	}

	MPI_Barrier(PETSC_COMM_WORLD);
	// MPI_CXX_BOOL is a MPI3 native datatype, our windows Build used MS_MPI MPI2, hence an MPI_INT is used
	MPI_Bcast(&dynamicDecomposition, 1, MPI_INT, 0, PETSC_COMM_WORLD);
	MPI_Bcast(&foundValidDecomposition, 1, MPI_INT, 0, PETSC_COMM_WORLD);

	if (!foundValidDecomposition)
	{
		PetscPrintf(PETSC_COMM_WORLD,
			"\nUnable to partition a %d (%d) x %d (%d) grid using %d cores, please select a different number of cores:\n", lowResNumI, numIGlobal(), lowResNumJ, numJGlobal(), totalNumberOfCores);
		PetscPrintf(PETSC_COMM_WORLD, "\tSelect either 1 core or M * N cores where M <= %d and N <= %d.\n", max(1, lowResNumI), max(1, lowResNumJ));
		PetscPrintf(PETSC_COMM_WORLD, "\tPlease note that these numbers may still be too high (application-dependent)!\n");
		PetscPrintf(PETSC_COMM_WORLD, "Exiting ...\n\n");

		MPI_Finalize();
		exit(-1);
	}

	PetscErrorCode err;
	if (dynamicDecomposition)
	{
		err = createPETSCDynamicDecomposition(numICores, numJCores, cellSizesI, cellSizesJ);
	}
	else
	{
		err = createPETSCStaticDecomposition(numICores, numJCores);
	}

	err = DMSetFromOptions(m_localInfo.da);
	err = DMSetUp(m_localInfo.da);
	err = DMDAGetLocalInfo(m_localInfo.da, &m_localInfo);
	err = DMCreateGlobalVector(m_localInfo.da, &m_vecGlobal);

	calculateNums(this); // calculated because fastcauldron is using them to create its own DA's.

#ifdef DEBUG_DYNAMIC_DECOMPOSITION
	// Print the corners of the sub-domains
	int start[2];
	int count[2];
	DMDAGetCorners(m_localInfo.da, &start[0], &start[1], PETSC_IGNORE, &count[0], &count[1], PETSC_IGNORE);
	std::cout << (std::to_string(start[0]) + "  " + std::to_string(start[1]) + "\n").c_str();
	std::cout  << (std::to_string(start[0] + count[0]) + "  " + std::to_string(start[1]) + "\n").c_str();
	std::cout  << (std::to_string(start[0] + count[0]) + "  " + std::to_string(start[1] + count[1]) + "\n").c_str();
	std::cout  << (std::to_string(start[0]) + "  " + std::to_string(start[1] + count[1]) + "\n").c_str();
#endif
}

PetscErrorCode DistributedGrid::createPETSCDynamicDecomposition(int& numICores, int& numJCores, std::vector<int>& cellSizesI, std::vector<int>& cellSizesJ)
{
	LogHandler(LogHandler::INFO_SEVERITY) << "Dynamic Domain Decomposition is used to distribute the model over the cores";

	int lxSize = cellSizesI.size();
	int lySize = cellSizesJ.size();

	MPI_Barrier(PETSC_COMM_WORLD);
	MPI_Bcast(&numICores, 1, MPI_INT, 0, PETSC_COMM_WORLD);
	MPI_Bcast(&numJCores, 1, MPI_INT, 0, PETSC_COMM_WORLD);

	MPI_Bcast(&lxSize, 1, MPI_INT, 0, PETSC_COMM_WORLD);
	MPI_Bcast(&lySize, 1, MPI_INT, 0, PETSC_COMM_WORLD);

	cellSizesI.reserve(lxSize);
	cellSizesJ.reserve(lySize);

	MPI_Bcast(cellSizesI.data(), lxSize, MPI_INT, 0, PETSC_COMM_WORLD);
	MPI_Bcast(cellSizesJ.data(), lySize, MPI_INT, 0, PETSC_COMM_WORLD);

	return DMDACreate2d(PETSC_COMM_WORLD, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE, DMDA_STENCIL_BOX,
		numIGlobal(), numJGlobal(),
		numICores, numJCores,
		1, 1, &cellSizesI[0], &cellSizesJ[0], &m_localInfo.da);
}

PetscErrorCode DistributedGrid::createPETSCStaticDecomposition(int& numICores, int& numJCores)
{
	LogHandler(LogHandler::INFO_SEVERITY) << "Static Domain Decomposition is used to distribute the model over the cores";

	MPI_Barrier(PETSC_COMM_WORLD);
	MPI_Bcast(&numICores, 1, MPI_INT, 0, PETSC_COMM_WORLD);
	MPI_Bcast(&numJCores, 1, MPI_INT, 0, PETSC_COMM_WORLD);

	// Use old method
	return DMDACreate2d(PETSC_COMM_WORLD, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE, DMDA_STENCIL_BOX,
		numIGlobal(), numJGlobal(),
		numICores, numJCores,
		1, 1, nullptr, nullptr, &m_localInfo.da);
}

/// Create a low res local grid and base its grid distribution on this, high res, local grid.
DistributedGrid::DistributedGrid(const Grid* referenceGrid, double minI, double minJ,
	double maxI, double maxJ, int numI, int numJ) :
	m_globalGrid(minI, minJ, maxI, maxJ, numI, numJ), m_numProcsI(-1), m_numProcsJ(-1), m_numsI(0), m_numsJ(0),
	m_returnsI(0), m_returnsJ(0)
{
	calculateNums(referenceGrid);
	PetscErrorCode err;
	//  global dimension in each direction of the array
	PetscInt M = numIGlobal();
	PetscInt N = numJGlobal();
	// corresponding number of processors in each dimension
	PetscInt m = referenceGrid->numProcsI();
	PetscInt n = referenceGrid->numProcsJ();
	//Arrays containing the number of nodes in each cell along the x and y coordinates, or NULL. If non-null, these must be of length as m and n, and the corresponding m and n cannot be PETSC_DECIDE.
	//The sum of the lx[] entries must be M, and the sum of the ly[] entries must be N.

	auto lx = numsI();
	auto ly = numsJ();
	err = DMDACreate2d(PETSC_COMM_WORLD, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE, DMDA_STENCIL_BOX,
		M, N,
		m, n, 1, 1,
		lx, ly, &m_localInfo.da);

#if DEBUGGING
	PetscSynchronizedPrintf(PETSC_COMM_WORLD, "Rank %d: xs = %d, xm = %d, ys = %d, ym = %d\n",
		ddd::GetRank(), m_localInfo.xs, m_localInfo.xm, m_localInfo.ys, m_localInfo.ym);
	PetscSynchronizedPrintf(PETSC_COMM_WORLD, "Rank %d: gxs = %d, gxm = %d, gys = %d, gym = %d\n",
		ddd::GetRank(), m_localInfo.gxs, m_localInfo.gxm, m_localInfo.gys, m_localInfo.gym);
	//PetscSynchronizedFlush (PETSC_COMM_WORLD);
#endif
	err = DMSetFromOptions(m_localInfo.da);
	err = DMSetUp(m_localInfo.da);
	err = DMDAGetLocalInfo(m_localInfo.da, &m_localInfo);
	err = DMCreateGlobalVector(m_localInfo.da, &m_vecGlobal);
	calculateNums(this);
}

void DistributedGrid::calculateNums(const Grid* referenceGrid)
{
	int procI, procJ;

	PetscMPIInt rank;

	MPI_Comm_rank(PETSC_COMM_WORLD, &rank);

	// processor-specific numbers of nodes in the i and j directions, respectively

	int sizeI = referenceGrid->numProcsI() * sizeof(PetscInt);
	int sizeJ = referenceGrid->numProcsJ() * sizeof(PetscInt);

	PetscMalloc(sizeI, &m_numsI);
	PetscMalloc(sizeJ, &m_numsJ);

	for (procI = 0; procI < referenceGrid->numProcsI(); ++procI) m_numsI[procI] = 0;
	for (procJ = 0; procJ < referenceGrid->numProcsJ(); ++procJ) m_numsJ[procJ] = 0;

	int scaleI = Round(deltaI() / referenceGrid->deltaI());
	int scaleJ = Round(deltaJ() / referenceGrid->deltaJ());

	assert(scaleI >= 1);
	assert(scaleJ >= 1);

	int i, j;
	for (i = 0; i < numIGlobal(); ++i)
	{
		unsigned int refI, refJ;
		convertToGrid(*referenceGrid, i, 0, refI, refJ);
#if 0
		PetscPrintf(PETSC_COMM_WORLD, "I: %d ==> %d\n", i, refI);
#endif
		if ((referenceGrid->onLowISide() || int(refI) >= referenceGrid->firstI()) &&
			(referenceGrid->onHighISide() || int(refI) <= referenceGrid->lastI()))
		{
#if 0
			fprintf(stderr, "Rank %d: onLowISide = %d, onHighISide = %d\n",
				rank,
				referenceGrid->onLowISide(), referenceGrid->onHighISide());

			fprintf(stderr, "Rank %d: (refI (%d) >= firstI (%d)) = %d\n",
				rank, refI, referenceGrid->firstI(), refI >= referenceGrid->firstI());
			fprintf(stderr, "Rank %d: (refI (%d) >= lastI (%d)) = %d\n",
				rank, refI, referenceGrid->lastI(), refI >= referenceGrid->lastI());

			fprintf(stderr, "Rank %d, I (refI): %d (%d) -> %d\n", rank, j, refI, rank / referenceGrid->numProcsI());
#endif
			m_numsI[rank % referenceGrid->numProcsI()]++;
		}
	}

	for (j = 0; j < numJGlobal(); ++j)
	{
		unsigned int refI, refJ;
		convertToGrid(*referenceGrid, 0, j, refI, refJ);
#if 0
		PetscPrintf(PETSC_COMM_WORLD, "J: %d ==> %d\n", j, refJ);
#endif
		if ((referenceGrid->onLowJSide() || int(refJ) >= referenceGrid->firstJ()) &&
			(referenceGrid->onHighJSide() || int(refJ) <= referenceGrid->lastJ()))
		{
#if 0
			fprintf(stderr, "Rank %d: onLowJSide = %d, onHighJSide = %d\n",
				rank,
				referenceGrid->onLowJSide(), referenceGrid->onHighJSide());

			fprintf(stderr, "Rank %d: (refJ (%d) >= firstJ (%d)) = %d\n",
				rank, refJ, referenceGrid->firstJ(), refJ >= referenceGrid->firstJ());
			fprintf(stderr, "Rank %d: (refJ (%d) >= lastJ (%d)) = %d\n",
				rank, refJ, referenceGrid->lastJ(), refJ >= referenceGrid->lastJ());

			fprintf(stderr, "Rank %d, J (refJ): %d (%d) -> %d\n", rank, j, refJ, rank / referenceGrid->numProcsI());
#endif
			m_numsJ[rank / referenceGrid->numProcsI()]++;
		}
	}

	int numNodesI = 0, numNodesJ = 0;

	for (procI = 0; procI < referenceGrid->numProcsI(); ++procI)
	{
		PetscInt tmpNumI;
		MPI_Allreduce(&m_numsI[procI], &tmpNumI, 1, MPI_INT, MPI_MAX, PETSC_COMM_WORLD);
		m_numsI[procI] = tmpNumI;
		numNodesI += m_numsI[procI];
#if 0
		PetscPrintf(PETSC_COMM_WORLD, "I: %d <- %d (%d)\n", procI, tmpNumI, numNodesI);
#endif
	}

	for (procJ = 0; procJ < referenceGrid->numProcsJ(); ++procJ)
	{
		PetscInt tmpNumJ;
		MPI_Allreduce(&m_numsJ[procJ], &tmpNumJ, 1, MPI_INT, MPI_MAX, PETSC_COMM_WORLD);
		m_numsJ[procJ] = tmpNumJ;
		numNodesJ += m_numsJ[procJ];
#if 0
		PetscPrintf(PETSC_COMM_WORLD, "J: %d <- %d (%d) \n", procJ, tmpNumJ, numNodesJ);
#endif
	}

	assert(numNodesI == numIGlobal());
	assert(numNodesJ == numJGlobal());
}

DistributedGrid::~DistributedGrid(void)
{
	VecDestroy(&m_vecGlobal);
	DMDestroy(&m_localInfo.da);

	if (m_numsI) PetscFree(m_numsI);
	if (m_numsJ) PetscFree(m_numsJ);

	if (m_returnsI) delete[] m_returnsI;
	if (m_returnsJ) delete[] m_returnsJ;
}

double DistributedGrid::minI(void) const
{
	return minI(false);
}

double DistributedGrid::minI(bool withGhosts) const
{
	return minIGlobal() + firstI(withGhosts) * deltaI();
}

double DistributedGrid::minJ(void) const
{
	return minJ(false);
}

double DistributedGrid::minJ(bool withGhosts) const
{
	return minJGlobal() + firstJ(withGhosts) * deltaJ();
}

double DistributedGrid::maxI(void) const
{
	return maxI(false);
}

double DistributedGrid::maxI(bool withGhosts) const
{
	return minIGlobal() + lastI(withGhosts) * deltaI();
}

double DistributedGrid::maxJ(void) const
{
	return maxJ(false);
}

double DistributedGrid::maxJ(bool withGhosts) const
{
	return minJGlobal() + lastJ(withGhosts) * deltaJ();
}

int DistributedGrid::numI(void) const
{
	return numI(false);
}

int DistributedGrid::numI(bool withGhosts) const
{
	return withGhosts ? m_localInfo.gxm : m_localInfo.xm;
}

int DistributedGrid::numJ(void) const
{
	return numJ(false);
}

int DistributedGrid::numJ(bool withGhosts) const
{
	return withGhosts ? m_localInfo.gym : m_localInfo.ym;
}

int DistributedGrid::firstI(void) const
{
	return firstI(false);
}

int DistributedGrid::firstI(bool withGhosts) const
{
	return withGhosts ? m_localInfo.gxs : m_localInfo.xs;
}

int DistributedGrid::firstJ(void) const
{
	return firstJ(false);
}

int DistributedGrid::firstJ(bool withGhosts) const
{
	return withGhosts ? m_localInfo.gys : m_localInfo.ys;
}

int DistributedGrid::lastI(void) const
{
	return lastI(false);
}

int DistributedGrid::lastI(bool withGhosts) const
{
	return withGhosts ? (m_localInfo.gxs + m_localInfo.gxm - 1) : (m_localInfo.xs + m_localInfo.xm - 1);
}

int DistributedGrid::lastJ(void) const
{
	return lastJ(false);
}

int DistributedGrid::lastJ(bool withGhosts) const
{
	return withGhosts ? (m_localInfo.gys + m_localInfo.gym - 1) : (m_localInfo.ys + m_localInfo.ym - 1);
}

double DistributedGrid::minIGlobal(void) const
{
	return m_globalGrid.minI();
}

double DistributedGrid::minJGlobal(void) const
{
	return m_globalGrid.minJ();
}

double DistributedGrid::maxIGlobal(void) const
{
	return m_globalGrid.maxI();
}

double DistributedGrid::maxJGlobal(void) const
{
	return m_globalGrid.maxJ();
}

int DistributedGrid::numIGlobal(void) const
{
	return m_globalGrid.numI();
}

int DistributedGrid::numJGlobal(void) const
{
	return m_globalGrid.numJ();
}

double DistributedGrid::deltaIGlobal(void) const
{
	return m_globalGrid.deltaI();
}

double DistributedGrid::deltaJGlobal(void) const
{
	return m_globalGrid.deltaJ();
}

bool DistributedGrid::onLowISide(void) const
{
	return firstI(false) == 0;
}

bool DistributedGrid::onHighISide(void) const
{
	return lastI(false) == numIGlobal() - 1;
}

bool DistributedGrid::onLowJSide(void) const
{
	return firstJ(false) == 0;
}

bool DistributedGrid::onHighJSide(void) const
{
	return lastJ(false) == numJGlobal() - 1;
}

double DistributedGrid::deltaI(void) const
{
	return m_globalGrid.deltaI();
}

double DistributedGrid::deltaJ(void) const
{
	return m_globalGrid.deltaJ();
}

int DistributedGrid::numProcsI(void) const
{
	if (m_numProcsI < 0)
	{
		DMDAGetInfo(m_localInfo.da, PETSC_IGNORE,
			PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE,
			&m_numProcsI, PETSC_IGNORE, PETSC_IGNORE,
			PETSC_IGNORE, PETSC_IGNORE,
			PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE,
			PETSC_IGNORE);
	}

	return m_numProcsI;
}

int DistributedGrid::numProcsJ(void) const
{
	if (m_numProcsJ < 0)
	{
		DMDAGetInfo(m_localInfo.da, PETSC_IGNORE,
			PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE,
			PETSC_IGNORE, &m_numProcsJ, PETSC_IGNORE,
			PETSC_IGNORE, PETSC_IGNORE,
			PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE,
			PETSC_IGNORE);
	}
	return m_numProcsJ;
}

PetscInt* DistributedGrid::numsI(void) const
{
	return (m_numsI ? m_numsI : PETSC_IGNORE);
}

PetscInt* DistributedGrid::numsJ(void) const
{
	return (m_numsJ ? m_numsJ : PETSC_IGNORE);
}

bool DistributedGrid::isGridPoint(unsigned int i, unsigned int j) const
{
	return m_globalGrid.isGridPoint(i, j);
}

bool DistributedGrid::getGridPoint(double posI, double posJ, unsigned int& i, unsigned int& j) const
{
	return m_globalGrid.getGridPoint(posI, posJ, i, j);
}

// Added by V.R. Ambati (13/07/2011):
// getGridPoint(....) is an abstract overloaded function. Some of its implementations are
// missing in the distributed version. Missing are implemented by simply
// throwing an error. return m_globalGrid.getGridPoint (posI, posJ, i, j) may
// also work here but I am not sure.
bool DistributedGrid::getGridPoint(double posI, double posJ, double& i, double& j) const
{
	throw "bool Grid::getGridPoint (double posI, double posJ, double & i, double & j) const is not implemented in distributed version. Use bool Grid::getGridPoint (double posI, double posJ, unsigned int & i, unsigned int & j) instead.";
}

double DistributedGrid::getSurface(unsigned int i, unsigned int j) const
{
	return m_globalGrid.getSurface(i, j);
}

bool DistributedGrid::getPosition(unsigned int i, unsigned int j, double& posI, double& posJ) const
{
	return m_globalGrid.getPosition(i, j, posI, posJ);
}

bool DistributedGrid::getPosition(double i, double j, double& posI, double& posJ) const
{
	return m_globalGrid.getPosition(i, j, posI, posJ);
}

bool DistributedGrid::convertToGrid(const Grid& toGrid,
	unsigned int fromI, unsigned int fromJ,
	unsigned int& toI, unsigned int& toJ) const
{
	return m_globalGrid.convertToGrid(dynamic_cast<const DistributedGrid&>(toGrid).getGlobalGrid(), fromI, fromJ, toI, toJ);
}

bool DistributedGrid::convertToGrid(const Grid& toGrid,
	unsigned int fromI, unsigned int fromJ,
	double& toI, double& toJ) const
{
	return m_globalGrid.convertToGrid(dynamic_cast<const DistributedGrid&>(toGrid).getGlobalGrid(), fromI, fromJ, toI, toJ);
}


unsigned int DistributedGrid::getRank(unsigned int i, unsigned int j) const
{
	unsigned int rankLocal = 0, rankGlobal;

	PetscMPIInt rank;
	MPI_Comm_rank(PETSC_COMM_WORLD, &rank);

	if (i >= firstI() && i <= lastI() &&
		j >= firstJ() && j <= lastJ())
	{
		rankLocal = rank;
	}

	MPI_Allreduce(&rankLocal, &rankGlobal, 1, MPI_INT, MPI_MAX, PETSC_COMM_WORLD);

	return rankGlobal;
}

double DistributedGrid::getDistance(unsigned int i1, unsigned int j1, unsigned int i2, unsigned int j2) const
{
	return m_globalGrid.getDistance(i1, j1, i2, j2);
}

const GlobalGrid& DistributedGrid::getGlobalGrid(void) const
{
	return m_globalGrid;
}

const DMDALocalInfo& DistributedGrid::getLocalInfo(void) const
{
	return m_localInfo;
}

const Vec& DistributedGrid::getVec(void) const
{
	return m_vecGlobal;
}

void DistributedGrid::asString(string& str) const
{
	ostringstream buf;

	buf << "Grid:";
	buf << " numI = " << numI() << ", numJ = " << numJ();
	buf << ", minI = " << minI() << ", minJ = " << minJ();
	buf << ", deltaI = " << deltaI() << ", deltaJ = " << deltaJ() << endl;

	str = buf.str();
}

void DistributedGrid::printOn(ostream& ostr) const
{
	string str;
	asString(str);
	ostr << str;
}

void DistributedGrid::printDistributionOn(MPI_Comm comm) const
{
	int i, j;

	// i indices
	PetscPrintf(comm, "   | ");

	for (i = 0; i < numIGlobal(); i++)
	{
		PetscPrintf(comm, "%4d", i);
	}
	PetscPrintf(comm, "\n");

	// table line
	PetscPrintf(comm, "-----");

	for (i = 0; i < numIGlobal(); i++)
	{
		PetscPrintf(comm, "----");
	}

	PetscPrintf(comm, "\n");
	PetscPrintf(comm, "   | ");
	PetscPrintf(comm, "\n");

	for (j = 0; j < numJGlobal(); j++)
	{
		// j indices
		PetscPrintf(comm, "%2d | ", j);

		for (i = 0; i < numIGlobal(); i++)
		{
			// int pointRank = 0;
			int pointRank = getRank(i, j);

			PetscPrintf(comm, "%4d", pointRank);
		}
		PetscPrintf(comm, "\n");
	}
	PetscPrintf(comm, "\n");
}

bool DistributedGrid::hasSameGridding(const Grid& grid) const
{
	return (
		this->numI() == grid.numI() &&
		this->numJ() == grid.numJ() &&
		this->firstI() == grid.firstI() &&
		this->firstJ() == grid.firstJ() &&
		this->lastI() == grid.lastI() &&
		this->lastJ() == grid.lastJ()
		);

}
