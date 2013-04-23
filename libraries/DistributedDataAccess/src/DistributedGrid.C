#include <math.h>
#include <values.h>
#include <assert.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <sstream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include<strstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <sstream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include "Interface/DistributedGrid.h"

#define Round(a) ((int) (((double) a) + 0.5))

using namespace DataAccess;

using namespace Interface;


bool DistributedGrid::CalculatePartitioning (int M, int N, int & mSelected, int & nSelected)
{
   int size;

#if 0
   PetscPrintf (PETSC_COMM_WORLD, "\nCalculating partioning for %d, %d\n", M, N);
#endif

   MPI_Comm_size (PETSC_COMM_WORLD, &size);

   double minimumScalingRatio = 1e10;
   bool scalingFound = false;
   int m, n;
   for (m = size; m > 0; --m)
   {
      n = size / m;
      if (m * n == size && m <= M && n <= N)
      {
	 scalingFound = true;
	 double mScaling = double(M) / double (m);
	 double nScaling = double(N) / double (n);

	 double scalingRatio = std::max (mScaling, nScaling) / std::min (mScaling, nScaling);
	 if (scalingRatio < minimumScalingRatio)
	 {
	    mSelected = m, nSelected = n;
	    minimumScalingRatio = scalingRatio;
	 }
#if 0
	 PetscPrintf (PETSC_COMM_WORLD, "m = %d, n = %d, mScaling = %lf, nScaling = %lf, scalingRatio = %lf, mSelected = %d, nSelected = %d\n",
	       m, n, mScaling, nScaling, scalingRatio, mSelected, nSelected);
#endif

      }
   }

#if 0
   if (scalingFound)
   {
      PetscPrintf (PETSC_COMM_WORLD,
	    "\nPartitioning %d x %d grid using %d cores into %d x %d cores\n", M, N, size, mSelected, nSelected);
   }
#endif
   return scalingFound;
}

DistributedGrid::DistributedGrid (double minI, double minJ,
      double maxI, double maxJ, int numI, int numJ, int lowResNumI, int lowResNumJ) :
   m_globalGrid (minI, minJ, maxI, maxJ, numI, numJ), m_numProcsI (-1), m_numProcsJ (-1), m_numsI (0), m_numsJ (0),
   m_convertingTo (0), m_conversionsToI (0), m_conversionsToJ (0), m_returnsI (0), m_returnsJ (0)
{
   int numICores, numJCores;

   int size;
   MPI_Comm_size (PETSC_COMM_WORLD, &size);

   // Calculate the core partitioning based on the low resolution output grid as that is the grid mostly used.
   // If there is no core partitioning for the low res grid, use the high res grid and hope the low res grid will not be used (checked elsewhere).
   if (!CalculatePartitioning (lowResNumI, lowResNumJ, numICores, numJCores) &&
	 !CalculatePartitioning (numIGlobal (), numJGlobal (), numICores, numJCores))
   {
      PetscPrintf (PETSC_COMM_WORLD,
                   "\nUnable to partition a %d (%d) x %d (%d) grid using %d cores, please select a different number of cores:\n", lowResNumI, numIGlobal (),  lowResNumJ, numJGlobal (), size);
      PetscPrintf(PETSC_COMM_WORLD, "Exiting ...\n\n");
      
      MPI_Finalize ();
      exit (-1);
   }


   DMDACreate2d ( PETSC_COMM_WORLD, DMDA_BOUNDARY_NONE, DMDA_BOUNDARY_NONE, DMDA_STENCIL_BOX,
                  numIGlobal (), numJGlobal (), 
                  numICores, numJCores, 1, 1, 
                  PETSC_NULL, PETSC_NULL, &m_localInfo.da );

   DMDAGetLocalInfo (m_localInfo.da, &m_localInfo);
   DMCreateGlobalVector (m_localInfo.da, &m_vecGlobal);

   calculateNums(this); // calculated because fastcauldron is using them to create its own DA's.
}

/// Create a low res local grid and base its grid distribution on this, high res, local grid.
DistributedGrid::DistributedGrid (const Grid * referenceGrid, double minI, double minJ,
      double maxI, double maxJ, int numI, int numJ) :
   m_globalGrid (minI, minJ, maxI, maxJ, numI, numJ), m_numProcsI (-1), m_numProcsJ (-1), m_numsI (0), m_numsJ (0),
   m_convertingTo (0), m_conversionsToI (0), m_conversionsToJ (0), m_returnsI (0), m_returnsJ (0)
{

   calculateNums(referenceGrid);

   DMDACreate2d (PETSC_COMM_WORLD, DMDA_BOUNDARY_NONE, DMDA_BOUNDARY_NONE, DMDA_STENCIL_BOX,
                 numIGlobal (), numJGlobal (),
                 referenceGrid->numProcsI (), referenceGrid->numProcsJ (), 1, 1,
                 numsI (), numsJ (), &m_localInfo.da);

   DMDAGetLocalInfo (m_localInfo.da, &m_localInfo);

#if 0
   PetscSynchronizedPrintf (PETSC_COMM_WORLD, "Rank %d: xs = %d, xm = %d, ys = %d, ym = %d\n",
	 ddd::GetRank (), m_localInfo.xs, m_localInfo.xm, m_localInfo.ys, m_localInfo.ym);
   PetscSynchronizedPrintf (PETSC_COMM_WORLD, "Rank %d: gxs = %d, gxm = %d, gys = %d, gym = %d\n",
	 ddd::GetRank (), m_localInfo.gxs, m_localInfo.gxm, m_localInfo.gys, m_localInfo.gym);
   PetscSynchronizedFlush (PETSC_COMM_WORLD);
#endif

   DMCreateGlobalVector (m_localInfo.da, &m_vecGlobal);
}

void DistributedGrid::calculateNums(const Grid * referenceGrid)
{
   int procI, procJ;

   PetscMPIInt rank;

   MPI_Comm_rank (PETSC_COMM_WORLD, &rank);

   // processor-specific numbers of nodes in the i and j directions, respectively

   int sizeI = referenceGrid->numProcsI () * sizeof (PetscInt);
   int sizeJ = referenceGrid->numProcsJ () * sizeof (PetscInt);

   PetscMalloc (sizeI, &m_numsI);
   PetscMalloc (sizeJ, &m_numsJ);

   for (procI = 0; procI < referenceGrid->numProcsI (); ++procI) m_numsI[procI] = 0;
   for (procJ = 0; procJ < referenceGrid->numProcsJ (); ++procJ) m_numsJ[procJ] = 0;

   int scaleI = Round (deltaI () / referenceGrid->deltaI ());
   int scaleJ = Round (deltaJ () / referenceGrid->deltaJ ());

   assert (scaleI >= 1);
   assert (scaleJ >= 1);

   int i, j;
   for (i = 0; i < numIGlobal (); ++i)
   {
      unsigned int refI, refJ;
      convertToGrid (* referenceGrid, i, 0, refI, refJ, false);
#if 0
      PetscPrintf (PETSC_COMM_WORLD, "I: %d ==> %d\n", i, refI);
#endif
      if ((referenceGrid->onLowISide () || int (refI) >= referenceGrid->firstI ()) &&
	  (referenceGrid->onHighISide () || int (refI) <= referenceGrid->lastI ()))
      {
#if 0
	 fprintf (stderr, "Rank %d: onLowISide = %d, onHighISide = %d\n",
	       rank,
	       referenceGrid->onLowISide (),  referenceGrid->onHighISide ());

	 fprintf (stderr, "Rank %d: (refI (%d) >= firstI (%d)) = %d\n",
	       rank, refI,  referenceGrid->firstI (), refI >= referenceGrid->firstI ());
	 fprintf (stderr, "Rank %d: (refI (%d) >= lastI (%d)) = %d\n",
	       rank, refI,  referenceGrid->lastI (), refI >= referenceGrid->lastI ());

	 fprintf (stderr, "Rank %d, I (refI): %d (%d) -> %d\n", rank, j, refI, rank / referenceGrid->numProcsI ());
#endif
	 m_numsI[rank % referenceGrid->numProcsI ()]++;
      }
   }

   for (j = 0; j < numJGlobal (); ++j)
   {
      unsigned int refI, refJ;
      convertToGrid (* referenceGrid, 0, j, refI, refJ, false);
#if 0
      PetscPrintf (PETSC_COMM_WORLD, "J: %d ==> %d\n", j, refJ);
#endif
      if ((referenceGrid->onLowJSide () || int (refJ) >= referenceGrid->firstJ ()) &&
	  (referenceGrid->onHighJSide () || int (refJ) <= referenceGrid->lastJ ()))
      {
#if 0
	 fprintf (stderr, "Rank %d: onLowJSide = %d, onHighJSide = %d\n",
	       rank,
	       referenceGrid->onLowJSide (),  referenceGrid->onHighJSide ());

	 fprintf (stderr, "Rank %d: (refJ (%d) >= firstJ (%d)) = %d\n",
	       rank, refJ,  referenceGrid->firstJ (), refJ >= referenceGrid->firstJ ());
	 fprintf (stderr, "Rank %d: (refJ (%d) >= lastJ (%d)) = %d\n",
	       rank, refJ,  referenceGrid->lastJ (), refJ >= referenceGrid->lastJ ());

	 fprintf (stderr, "Rank %d, J (refJ): %d (%d) -> %d\n", rank, j, refJ, rank / referenceGrid->numProcsI ());
#endif
	 m_numsJ[rank / referenceGrid->numProcsI ()]++;
      }
   }

   int numNodesI = 0, numNodesJ = 0;

   for (procI = 0; procI < referenceGrid->numProcsI (); ++procI)
   {
      PetscInt tmpNumI;
      MPI_Allreduce (&m_numsI[procI], &tmpNumI, 1, MPI_INT, MPI_MAX, PETSC_COMM_WORLD);
      m_numsI[procI] = tmpNumI;
      numNodesI += m_numsI[procI];
#if 0
      PetscPrintf (PETSC_COMM_WORLD, "I: %d <- %d (%d)\n", procI, tmpNumI, numNodesI);
#endif
   }

   for (procJ = 0; procJ < referenceGrid->numProcsJ (); ++procJ)
   {
      PetscInt tmpNumJ;
      MPI_Allreduce (&m_numsJ[procJ], &tmpNumJ, 1, MPI_INT, MPI_MAX, PETSC_COMM_WORLD);
      m_numsJ[procJ] = tmpNumJ;
      numNodesJ += m_numsJ[procJ];
#if 0
      PetscPrintf (PETSC_COMM_WORLD, "J: %d <- %d (%d) \n", procJ, tmpNumJ, numNodesJ);
#endif
   }

   assert (numNodesI == numIGlobal ());
   assert (numNodesJ == numJGlobal ());
}

DistributedGrid::~DistributedGrid (void)
{
   VecDestroy (&m_vecGlobal);
   DMDestroy (&m_localInfo.da);

   if (m_numsI) PetscFree (m_numsI);
   if (m_numsJ) PetscFree (m_numsJ);

   if (m_conversionsToI) delete [] m_conversionsToI;
   if (m_conversionsToJ) delete [] m_conversionsToJ;

   if (m_returnsI) delete [] m_returnsI;
   if (m_returnsJ) delete [] m_returnsJ;
}

double DistributedGrid::minI (void) const
{
   return minI (false);
}

double DistributedGrid::minI (bool withGhosts) const
{
   return minIGlobal () + firstI (withGhosts) * deltaI ();
}

double DistributedGrid::minJ (void) const
{
   return minJ (false);
}

double DistributedGrid::minJ (bool withGhosts) const
{
   return minJGlobal () + firstJ (withGhosts) * deltaJ ();
}

double DistributedGrid::maxI (void) const
{
   return maxI (false);
}

double DistributedGrid::maxI (bool withGhosts) const
{
   return minIGlobal () + lastI (withGhosts) * deltaI ();
}

double DistributedGrid::maxJ (void) const
{
   return maxJ (false);
}

double DistributedGrid::maxJ (bool withGhosts) const
{
   return minJGlobal () + lastJ (withGhosts) * deltaJ ();
}

int DistributedGrid::numI (void) const
{
   return numI (false);
}

int DistributedGrid::numI (bool withGhosts) const
{
   return withGhosts ? m_localInfo.gxm : m_localInfo.xm;
}

int DistributedGrid::numJ (void) const
{
   return numJ (false);
}

int DistributedGrid::numJ (bool withGhosts) const
{
   return withGhosts ? m_localInfo.gym : m_localInfo.ym;
}

int DistributedGrid::firstI (void) const
{
   return firstI (false);
}

int DistributedGrid::firstI (bool withGhosts) const
{
   return withGhosts ? m_localInfo.gxs : m_localInfo.xs;
}

int DistributedGrid::firstJ (void) const
{
   return firstJ (false);
}

int DistributedGrid::firstJ (bool withGhosts) const
{
   return withGhosts ? m_localInfo.gys : m_localInfo.ys;
}

int DistributedGrid::lastI (void) const
{
   return lastI (false);
}

int DistributedGrid::lastI (bool withGhosts) const
{
   return withGhosts ? (m_localInfo.gxs + m_localInfo.gxm - 1) : (m_localInfo.xs + m_localInfo.xm - 1);
}

int DistributedGrid::lastJ (void) const
{
   return lastJ (false);
}

int DistributedGrid::lastJ (bool withGhosts) const
{
   return withGhosts ? (m_localInfo.gys + m_localInfo.gym - 1) : (m_localInfo.ys + m_localInfo.ym - 1);
}

double DistributedGrid::minIGlobal (void) const
{
   return m_globalGrid.minI ();
}

double DistributedGrid::minJGlobal (void) const
{
   return m_globalGrid.minJ ();
}

double DistributedGrid::maxIGlobal (void) const
{
   return m_globalGrid.maxI ();
}

double DistributedGrid::maxJGlobal (void) const
{
   return m_globalGrid.maxJ ();
}

int DistributedGrid::numIGlobal (void) const
{
   return m_globalGrid.numI ();
}

int DistributedGrid::numJGlobal (void) const
{
   return m_globalGrid.numJ ();
}

double DistributedGrid::deltaIGlobal (void) const
{
   return m_globalGrid.deltaI ();
}

double DistributedGrid::deltaJGlobal (void) const
{
   return m_globalGrid.deltaJ ();
}

bool DistributedGrid::onLowISide (void) const
{
   return firstI (false) == 0;
}

bool DistributedGrid::onHighISide (void) const
{
   return lastI (false) == numIGlobal () - 1;
}

bool DistributedGrid::onLowJSide (void) const
{
   return firstJ (false) == 0;
}

bool DistributedGrid::onHighJSide (void) const
{
   return lastJ (false) == numJGlobal () - 1;
}

double DistributedGrid::deltaI (void) const
{
   return m_globalGrid.deltaI ();
}

double DistributedGrid::deltaJ (void) const
{
   return m_globalGrid.deltaJ ();
}

int DistributedGrid::numProcsI (void) const
{
   if (m_numProcsI < 0)
   {
   DMDAGetInfo( m_localInfo.da, PETSC_NULL,
                PETSC_NULL, PETSC_NULL, PETSC_NULL, 
                &m_numProcsI, PETSC_NULL, PETSC_NULL,
                PETSC_NULL,PETSC_NULL,
                PETSC_NULL,PETSC_NULL,PETSC_NULL,
                PETSC_NULL);
   }

   return m_numProcsI;
}

int DistributedGrid::numProcsJ (void) const
{
   if (m_numProcsJ < 0)
   {
   DMDAGetInfo( m_localInfo.da, PETSC_NULL,
                PETSC_NULL, PETSC_NULL, PETSC_NULL, 
                PETSC_NULL, &m_numProcsJ, PETSC_NULL,
                PETSC_NULL,PETSC_NULL,
                PETSC_NULL,PETSC_NULL,PETSC_NULL,
                PETSC_NULL);
   }
   return m_numProcsJ;
}

PetscInt * DistributedGrid::numsI (void) const
{
   return (m_numsI ? m_numsI : PETSC_NULL);
}

PetscInt * DistributedGrid::numsJ (void) const
{
   return (m_numsJ ? m_numsJ : PETSC_NULL);
}

bool DistributedGrid::isGridPoint (unsigned int i, unsigned int j) const
{
   return m_globalGrid.isGridPoint (i, j);
}

bool DistributedGrid::getGridPoint (double posI, double posJ, unsigned int & i, unsigned int & j) const
{
   return m_globalGrid.getGridPoint (posI, posJ, i, j);
}

// Added by V.R. Ambati (13/07/2011):
// getGridPoint(....) is an abstract overloaded function. Some of its implementations are
// missing in the distributed version. Missing are implemented by simply
// throwing an error. return m_globalGrid.getGridPoint (posI, posJ, i, j) may
// also work here but I am not sure. 
bool DistributedGrid::getGridPoint (double posI, double posJ, double & i, double & j) const
{
   throw "bool Grid::getGridPoint (double posI, double posJ, double & i, double & j) const is not implemented in distributed version. Use bool Grid::getGridPoint (double posI, double posJ, unsigned int & i, unsigned int & j) instead.";
}

double DistributedGrid::getSurface (unsigned int i, unsigned int j) const
{
   return m_globalGrid.getSurface (i, j);
}

bool DistributedGrid::getPosition (unsigned int i, unsigned int j, double &posI, double &posJ) const
{
   return m_globalGrid.getPosition (i, j, posI, posJ);
}
   
bool DistributedGrid::getPosition (double i, double j, double &posI, double &posJ) const
{
   return m_globalGrid.getPosition (i, j, posI, posJ);
}

bool DistributedGrid::convertToGrid (const Grid & toGrid,
      unsigned int fromI, unsigned int fromJ,
      unsigned int & toI, unsigned int & toJ, bool useCaching) const
{
   if (false && useCaching)
   {
      if (m_convertingTo != & toGrid)
      {
	 if (m_conversionsToI) delete [] m_conversionsToI;

	 m_conversionsToI = new unsigned int [numI ()];
	 for (int i = numI () - 1; i >= 0; i--)
	    m_conversionsToI[i] = MAXINT;

	 if (m_conversionsToJ) delete [] m_conversionsToJ;
	 m_conversionsToJ = new unsigned int [numJ ()];
	 for (int j = numJ () - 1; j >= 0; j--)
	    m_conversionsToJ[j] = MAXINT;

	 if (m_returnsI) delete [] m_returnsI;
	 m_returnsI = new bool [numI ()];

	 if (m_returnsJ) delete [] m_returnsJ;
	 m_returnsJ = new bool [numJ ()];

	 m_convertingTo = & toGrid;
      }

      int arrayI = fromI - firstI ();
      int arrayJ = fromJ - firstJ ();
      if (arrayI >= 0 && arrayI < numI () && arrayJ >= 0 && fromJ < numJ ())
      {
	 if (m_conversionsToI[arrayI] == MAXINT || m_conversionsToJ[arrayJ] == MAXINT)
	 {
	    m_returnsI[arrayI] = m_returnsJ[arrayJ] =
	       m_globalGrid.convertToGrid (dynamic_cast<const DistributedGrid &>(toGrid).getGlobalGrid (), fromI, fromJ,
		     m_conversionsToI[arrayI], m_conversionsToJ[arrayJ]);
	 }

	 toI = m_conversionsToI[arrayI];
	 toJ = m_conversionsToJ[arrayJ];

	 return (m_returnsI[arrayI] && m_returnsJ[arrayJ]);
      }
   }
   return m_globalGrid.convertToGrid (dynamic_cast<const DistributedGrid &>(toGrid).getGlobalGrid (), fromI, fromJ, toI, toJ);
}

bool DistributedGrid::convertToGrid (const Grid & toGrid,
      unsigned int fromI, unsigned int fromJ,
      double & toI, double & toJ, bool useCaching) const
{
   if (false && useCaching)
   {
      if (m_convertingTo != & toGrid)
      {
	 if (m_conversionsToI) delete [] m_conversionsToI;

	 m_conversionsToI = new unsigned int [numI ()];
	 for (int i = numI () - 1; i >= 0; i--)
	    m_conversionsToI[i] = MAXINT;

	 if (m_conversionsToJ) delete [] m_conversionsToJ;
	 m_conversionsToJ = new unsigned int [numJ ()];
	 for (int j = numJ () - 1; j >= 0; j--)
	    m_conversionsToJ[j] = MAXINT;

	 if (m_returnsI) delete [] m_returnsI;
	 m_returnsI = new bool [numI ()];

	 if (m_returnsJ) delete [] m_returnsJ;
	 m_returnsJ = new bool [numJ ()];

	 m_convertingTo = & toGrid;
      }

      int arrayI = fromI - firstI ();
      int arrayJ = fromJ - firstJ ();
      if (arrayI >= 0 && arrayI < numI () && arrayJ >= 0 && fromJ < numJ ())
      {
	 if (m_conversionsToI[arrayI] == MAXINT || m_conversionsToJ[arrayJ] == MAXINT)
	 {
	    m_returnsI[arrayI] = m_returnsJ[arrayJ] =
	       m_globalGrid.convertToGrid (dynamic_cast<const DistributedGrid &>(toGrid).getGlobalGrid (), fromI, fromJ,
		     m_conversionsToI[arrayI], m_conversionsToJ[arrayJ]);
	 }

	 toI = m_conversionsToI[arrayI];
	 toJ = m_conversionsToJ[arrayJ];

	 return (m_returnsI[arrayI] && m_returnsJ[arrayJ]);
      }
   }
   return m_globalGrid.convertToGrid (dynamic_cast<const DistributedGrid &>(toGrid).getGlobalGrid (), fromI, fromJ, toI, toJ);
}


unsigned int DistributedGrid::getRank (unsigned int i, unsigned int j) const
{
   unsigned int rankLocal = 0, rankGlobal;

   PetscMPIInt rank;
   MPI_Comm_rank (PETSC_COMM_WORLD, &rank);

   if (i >= firstI () && i <= lastI () &&
	 j >= firstJ () && j <= lastJ ())
   {
      rankLocal = rank;
   }

   MPI_Allreduce (&rankLocal, &rankGlobal, 1, MPI_INT, MPI_MAX, PETSC_COMM_WORLD);

   return rankGlobal;
}

double DistributedGrid::getDistance (unsigned int i1, unsigned int j1, unsigned int i2, unsigned int j2) const
{
   return m_globalGrid.getDistance (i1, j1, i2, j2);
}

const GlobalGrid & DistributedGrid::getGlobalGrid (void) const
{
   return m_globalGrid;
}

const DMDALocalInfo & DistributedGrid::getLocalInfo (void) const
{
   return m_localInfo;
}

const Vec & DistributedGrid::getVec (void) const
{
   return m_vecGlobal;
}

void DistributedGrid::asString (string & str) const
{
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "Grid:";
   buf << " numI = " << numI () << ", numJ = " << numJ ();
   buf << ", minI = " << minI () << ", minJ = " << minJ ();
   buf << ", deltaI = " << deltaI () << ", deltaJ = " << deltaJ () << endl;

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}

void DistributedGrid::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void DistributedGrid::printDistributionOn (MPI_Comm comm) const
{
   int i, j;

   // i indices
   PetscPrintf (comm, "   | ");

   for (i = 0; i < numIGlobal (); i++)
   {
      PetscPrintf (comm, "%4d", i);
   }
   PetscPrintf (comm, "\n");

   // table line
   PetscPrintf (comm, "-----");

   for (i = 0; i < numIGlobal (); i++)
   {
      PetscPrintf (comm, "----");
   }

   PetscPrintf (comm, "\n");
   PetscPrintf (comm, "   | ");
   PetscPrintf (comm, "\n");

   for (j = 0; j < numJGlobal (); j++)
   {
      // j indices
      PetscPrintf (comm, "%2d | ", j);

      for (i = 0; i < numIGlobal (); i++)
      {
	 // int pointRank = 0;
	 int pointRank = getRank (i, j);

	 PetscPrintf (comm, "%4d", pointRank);
      }
      PetscPrintf (comm, "\n");
   } 
   PetscPrintf (comm, "\n");
}

bool DataAccess::Interface::operator==(const Grid & Grid1, const Grid & Grid2)
{
   return ( 
            Grid1.numI()  == Grid2.numI()   && 
            Grid1.numJ()  == Grid2.numJ()   &&
            Grid1.firstI()== Grid2.firstI() && 
            Grid1.firstJ()== Grid2.firstJ() &&
	    Grid1.lastI() == Grid2.lastI()  && 
            Grid1.lastJ() == Grid2.lastJ() 
          );
	
}
/*bool Grid::operator==(const Grid & Grid1) const
{
   return ( 
             numI()  == Grid1.numI()   && 
             numJ()  == Grid1.numJ()   &&
             firstI()== Grid1.firstI() && 
             firstJ()== Grid1.firstJ() &&
	     lastI() == Grid1.lastI()  && 
             lastJ() == Grid1.lastJ() 
          );
	
}*/
		
