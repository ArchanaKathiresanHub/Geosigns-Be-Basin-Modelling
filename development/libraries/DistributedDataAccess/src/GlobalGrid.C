#include <math.h>

#include <iostream>
#include <sstream>
using namespace std;


#include "GlobalGrid.h"
#include "petscvec.h"
#include "petscdmda.h"

#define Square(a)  ((a)*(a))

int MyRound (double a)
{
   if (a > -0.5)
      return (int) (a + 0.5);
   else
      return (int) (a + 0.5) - 1;
}

using namespace DataAccess;
using namespace Interface;

GlobalGrid::GlobalGrid (double minI, double minJ,
      double maxI, double maxJ, int numI, int numJ) :
   m_minI (minI), m_minJ (minJ), m_maxI (maxI), m_maxJ (maxJ), m_numI (numI), m_numJ (numJ),
   m_deltaI ((m_maxI - m_minI) / (m_numI - 1)), m_deltaJ ((m_maxJ - m_minJ) / (m_numJ - 1)),
   m_surface (m_deltaI * m_deltaJ)
{
}

GlobalGrid::~GlobalGrid (void)
{
}

double GlobalGrid::minI (void) const
{
   return m_minI;
}

double GlobalGrid::minJ (void) const
{
   return m_minJ;
}

double GlobalGrid::maxI (void) const
{
   return m_maxI;
}

double GlobalGrid::maxJ (void) const
{
   return m_maxJ;
}

int GlobalGrid::numI (void) const
{
   return m_numI;
}

int GlobalGrid::numJ (void) const
{
   return m_numJ;
}

double GlobalGrid::deltaI (void) const
{
   return m_deltaI;
}

double GlobalGrid::deltaJ (void) const
{
   return m_deltaJ;
}

bool GlobalGrid::isGridPoint (int i, int j) const
{
   return (i < m_numI && j < m_numJ);
}

bool GlobalGrid::getGridPoint (double posI, double posJ, unsigned int & i, unsigned int & j) const
{
   double tmpI = (posI - m_minI) / m_deltaI;
   double tmpJ = (posJ - m_minJ) / m_deltaJ;

   i = MyRound (tmpI);
   j = MyRound (tmpJ);

   return isGridPoint (i, j);
}

bool GlobalGrid::getGridPoint (double posI, double posJ, double & i, double & j) const
{
   i = (posI - m_minI) / m_deltaI;
   j = (posJ - m_minJ) / m_deltaJ;

   unsigned int tmpI = (unsigned int) i;
   unsigned int tmpJ = (unsigned int) j;

   bool ret = (i >= 0 && j >= 0);
   
   if (ret)
      ret = isGridPoint (tmpI, tmpJ);

   if (ret && (double) i > tmpI) 
      ret = isGridPoint (tmpI + 1, tmpJ);

   if (ret && (double) j > tmpJ) 
      ret = isGridPoint (tmpI, tmpJ + 1);

   return ret;
}

bool GlobalGrid::convertToGrid (const GlobalGrid & toGrid,
      unsigned int fromI, unsigned int fromJ,
      unsigned int & toI, unsigned int & toJ) const
{
   double posI, posJ;
   if (!getPosition (fromI, fromJ, posI, posJ)) return false;

   bool result = toGrid.getGridPoint (posI, posJ, toI, toJ);

#if 0
   PetscPrintf (PETSC_COMM_WORLD,
	 "fromNumI = %d, fromNumJ = %d, fromI = %d, fromJ = %d, posI = %lf, posJ = %lf, toNumI = %d, toNumJ = %d, toI = %d, toJ = %d\n",
	 numI (), numJ (), fromI, fromJ, posI, posJ, toGrid.numI (), toGrid.numJ (), toI, toJ);
#endif
   return result;
}

bool GlobalGrid::convertToGrid (const GlobalGrid & toGrid,
      unsigned int fromI, unsigned int fromJ,
      double & toI, double & toJ) const
{
   double posI, posJ;
   if (!getPosition (fromI, fromJ, posI, posJ)) return false;

   bool result = toGrid.getGridPoint (posI, posJ, toI, toJ);

#if 0
   PetscPrintf (PETSC_COMM_WORLD,
	 "fromI = %d, fromJ = %d, posI = %lf, posJ = %lf, toI = %lf, toJ = %lf\n",
	 fromI, fromJ, posI, posJ, toI, toJ);
#endif
   return result;
}

double GlobalGrid::getSurface (int i, int j) const
{
   double surface = m_surface;

   if (i == 0 || i == numI () - 1)
      surface /= 2;

   if (j == 0 || j == numJ () - 1)
      surface /= 2;

   return surface;
}

bool GlobalGrid::getPosition (unsigned int i, unsigned int j, double &posI, double &posJ) const
{
   if (!isGridPoint (i, j))
      return false;

   posI = i * m_deltaI + m_minI;
   posJ = j * m_deltaJ + m_minJ;

   return true;
}
   
bool GlobalGrid::getPosition (double i, double j, double &posI, double &posJ) const
{
   if (i < 0 || j < 0) return false;

   if (!isGridPoint ((int) i, (int) j))
      return false;

   if (i > (int) i && !isGridPoint ((int) i + 1, (int) j))
      return false;

   if (j > (int) j && !isGridPoint ((int) i, (int) j + 1))
      return false;

   if (i > (int) i && j > (int) j && !isGridPoint ((int) i + 1, (int) j + 1))
      return false;


   posI = i * m_deltaI + m_minI;
   posJ = j * m_deltaJ + m_minJ;

   return true;
}

double GlobalGrid::getDistance (int i1, int j1, int i2, int j2) const
{
   if (!isGridPoint (i1, j1))
      return -1;
   if (!isGridPoint (i2, j2))
      return -1;

   double distI = (i2 - i1) * m_deltaI;
   double distJ = (j2 - j1) * m_deltaJ;

   return sqrt (Square (distI) + Square (distJ));
}

void GlobalGrid::asString (string & str) const
{
   ostringstream buf;

   buf << "GlobalGrid:";
   buf << " numI = " << numI () << ", numJ = " << numJ ();
   buf << ", minI = " << minI () << ", minJ = " << minJ ();
   buf << ", deltaI = " << deltaI () << ", deltaJ = " << deltaJ () << endl;

   str = buf.str ();
}
