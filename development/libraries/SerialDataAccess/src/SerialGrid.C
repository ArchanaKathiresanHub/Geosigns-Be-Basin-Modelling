#include <math.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "Interface/SerialGrid.h"

#define Square(a)  ((a)*(a))
#define Round(a) ((int) (((double) a) + 0.5))

using namespace DataAccess;
using namespace Interface;


SerialGrid::SerialGrid (double minI, double minJ,
      double maxI, double maxJ, unsigned int numI, unsigned int numJ) :
   m_minI (minI), m_minJ (minJ), m_maxI (maxI), m_maxJ (maxJ), m_numI (numI), m_numJ (numJ),
   m_deltaI ((m_maxI - m_minI) / (m_numI - 1)), m_deltaJ ((m_maxJ - m_minJ) / (m_numJ - 1)),
   m_surface (m_deltaI * m_deltaJ)
{
   m_numsI = new int [1];
   m_numsI[0] = int(numI);
   m_numsJ = new int [1];
   m_numsJ[0] = int(numJ);
}

SerialGrid::SerialGrid (const Grid * , double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ) :
   m_minI (minI), m_minJ (minJ), m_maxI (maxI), m_maxJ (maxJ), m_numI (numI), m_numJ (numJ),
   m_deltaI ((m_maxI - m_minI) / (m_numI - 1)), m_deltaJ ((m_maxJ - m_minJ) / (m_numJ - 1)),
   m_surface (m_deltaI * m_deltaJ)
{
   m_numsI = new int [1];
   m_numsI[0] = int(numI);
   m_numsJ = new int [1];
   m_numsJ[0] = int(numJ);
}

SerialGrid::~SerialGrid (void)
{
  delete [] m_numsI;
  delete [] m_numsJ;
}

double SerialGrid::minI (void) const
{
   return m_minI;
}

double SerialGrid::minJ (void) const
{
   return m_minJ;
}

double SerialGrid::maxI (void) const
{
   return m_maxI;
}

double SerialGrid::maxJ (void) const
{
   return m_maxJ;
}

int SerialGrid::numI (void) const
{
   return m_numI;
}

int SerialGrid::numJ (void) const
{
   return m_numJ;
}

int SerialGrid::firstI (void) const
{
   return 0;
}

int SerialGrid::firstI (bool) const
{
   return 0;
}

int SerialGrid::firstJ (void) const
{
   return 0;
}

int SerialGrid::firstJ (bool) const
{
   return 0;
}

int SerialGrid::lastI (void) const
{
   return numI () - 1;
}

int SerialGrid::lastI (bool) const
{
   return lastI();
}

int SerialGrid::lastJ (void) const
{
   return numJ () - 1;
}

int SerialGrid::lastJ (bool) const
{
   return lastJ();
}

double SerialGrid::deltaI (void) const
{
   return m_deltaI;
}

double SerialGrid::deltaJ (void) const
{
   return m_deltaJ;
}

double SerialGrid::minIGlobal (void) const
{
   return m_minI;
}

double SerialGrid::minJGlobal (void) const
{
   return m_minJ;
}

double SerialGrid::maxIGlobal (void) const
{
   return m_maxI;
}

double SerialGrid::maxJGlobal (void) const
{
   return m_maxJ;
}

int SerialGrid::numIGlobal (void) const
{
   return m_numI;
}

int SerialGrid::numJGlobal (void) const
{
   return m_numJ;
}

double SerialGrid::deltaIGlobal (void) const
{
   return m_deltaI;
}

double SerialGrid::deltaJGlobal (void) const
{
   return m_deltaJ;
}

int SerialGrid::numProcsI (void) const
{
   return 1;
}

int SerialGrid::numProcsJ (void) const
{
   return 1;
}

int * SerialGrid::numsI (void) const
{
   return m_numsI;
}

int * SerialGrid::numsJ (void) const
{
   return m_numsJ;
}

bool SerialGrid::onLowISide (void) const
{
   return true;
}

bool SerialGrid::onHighISide (void) const
{
   return true;
}

bool SerialGrid::onLowJSide (void) const
{
   return true;
}

bool SerialGrid::onHighJSide (void) const
{
   return true;
}

bool SerialGrid::isGridPoint (unsigned int i, unsigned int j) const
{
   return (i < m_numI && j < m_numJ);
}

bool SerialGrid::getGridPoint (double posI, double posJ, unsigned int & i, unsigned int & j) const
{
   double tmpI = (posI - m_minI) / m_deltaI;
   double tmpJ = (posJ - m_minJ) / m_deltaJ;

   i = Round (tmpI);
   j = Round (tmpJ);

   return isGridPoint (i, j);
}

bool SerialGrid::getGridPoint (double posI, double posJ, double & i, double & j) const
{
   i = (posI - m_minI) / m_deltaI;
   j = (posJ - m_minJ) / m_deltaJ;

   unsigned int tmpI = (unsigned int) i;
   unsigned int tmpJ = (unsigned int) j;

   bool ret = (i >= 0 && j >= 0);
   
   if (ret)
      ret = isGridPoint (tmpI, tmpJ);

   if (ret && i > (double) tmpI) 
      ret = isGridPoint (tmpI + 1, tmpJ);

   if (ret && j > (double) tmpJ) 
      ret = isGridPoint (tmpI, tmpJ + 1);

   return ret;
}


double SerialGrid::getSurface (unsigned int i, unsigned int j) const
{
   double surface = m_surface;

   if (i == 0 || static_cast<int>(i) == numI () - 1)
      surface /= 2;

   if (j == 0 || static_cast<int>(j) == numJ () - 1)
      surface /= 2;

   return surface;
}

bool SerialGrid::getPosition (unsigned int i, unsigned int j, double &posI, double &posJ) const
{
   if (!isGridPoint (i, j))
      return false;

   posI = i * m_deltaI + m_minI;
   posJ = j * m_deltaJ + m_minJ;

   return true;
}
   
bool SerialGrid::getPosition (double i, double j, double &posI, double &posJ) const
{
   if (i < 0 || j < 0) return false;

   if (!isGridPoint ((unsigned int) i, (unsigned int) j))
      return false;

   if (i > (unsigned int) i && !isGridPoint ((unsigned int) i + 1, (unsigned int) j))
      return false;

   if (j > (unsigned int) j && !isGridPoint ((unsigned int) i, (unsigned int) j + 1))
      return false;

   if (i > (unsigned int) i && j > (unsigned int) j && !isGridPoint ((unsigned int) i + 1, (unsigned int) j + 1))
      return false;


   posI = i * m_deltaI + m_minI;
   posJ = j * m_deltaJ + m_minJ;

   return true;
}

bool SerialGrid::convertToGrid (const Grid & toGrid,
      unsigned int fromI, unsigned int fromJ,
      unsigned int & toI, unsigned int & toJ, bool) const
{
   double posI, posJ;
   if (!getPosition (fromI, fromJ, posI, posJ)) return false;

   bool result = toGrid.getGridPoint (posI, posJ, toI, toJ);

   return result;
}

bool SerialGrid::convertToGrid (const Grid & toGrid,
      unsigned int fromI, unsigned int fromJ,
      double & toI, double & toJ, bool) const
{
   double posI, posJ;
   if (!getPosition (fromI, fromJ, posI, posJ)) return false;

   bool result = toGrid.getGridPoint (posI, posJ, toI, toJ);

   return result;
}

double SerialGrid::getDistance (unsigned int i1, unsigned int j1, unsigned int i2, unsigned int j2) const
{
   if (!isGridPoint (i1, j1))
      return -1;
   if (!isGridPoint (i2, j2))
      return -1;

   double distI = (i2 - i1) * m_deltaI;
   double distJ = (j2 - j1) * m_deltaJ;

   return sqrt (Square (distI) + Square (distJ));
}

void SerialGrid::printOn (ostream & ostr) const
{
   ostr << "Grid:";
   ostr << " numI = " << numI () << ", numJ = " << numJ ();
   ostr << ", minI = " << minI () << ", minJ = " << minJ ();
   ostr << ", deltaI = " << deltaI () << ", deltaJ = " << deltaJ () << endl;
}

void SerialGrid::asString (string & str) const
{
   ostringstream buf;

   buf << "Grid:";
   buf << " numI = " << numI () << ", numJ = " << numJ ();
   buf << ", minI = " << minI () << ", minJ = " << minJ ();
   buf << ", deltaI = " << deltaI () << ", deltaJ = " << deltaJ () << endl;

   str = buf.str ();
}

bool SerialGrid::isEqual( const Grid& grid ) const
{
   return(
      this->numI()   == grid.numI   () &&
      this->numJ()   == grid.numJ   () &&
      this->minI()   == grid.minI   () &&
      this->minJ()   == grid.minJ   () &&
      this->deltaI() == grid.deltaI () &&
      this->deltaJ() == grid.deltaJ ()
   );
}
