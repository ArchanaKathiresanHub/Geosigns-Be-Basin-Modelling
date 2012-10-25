#ifndef _PARALLELDATAACCESS_GLOBALGRID_H_
#define _PARALLELDATAACCESS_GLOBALGRID_H_

#include <string>
using namespace std;

namespace DataAccess
{
   namespace Interface
   {
      /// Class describing regular grids.
      /// Objects of this class describe the input and output grids of a Cauldron project.
      class GlobalGrid
      {
public:
         /// create a new global grid
         GlobalGrid (double minI, double minJ, double maxI, double maxJ, int numI, int numJ);
	 virtual ~GlobalGrid (void);

         /// Checks whether the given point is a grid point
         bool isGridPoint (int i, int j) const;

         /// return the leftmost grid coordinate value
         double minI (void) const;
         /// return the bottommost grid coordinate value
         double minJ (void) const;
         /// return the rightmost grid coordinate value
         double maxI (void) const;
         /// return the topmost grid coordinate value
         double maxJ (void) const;
         /// return the horizontal resolution
         int numI (void) const;
         /// return the vertical resolution
         int numJ (void) const;

         /// return the distance between two vertical gridlines
         double deltaI (void) const;
         /// return the distance between two horizontal gridlines
         double deltaJ (void) const;

         /// return the gridpoint with the given real-world coordinates
         bool getGridPoint (double posI, double posJ, unsigned int &i, unsigned int &j) const;

         /// return the point in the grid with the given real-world coordinates
         /// the point can be a fractional point
         bool getGridPoint (double posI, double posJ, double &i, double &j) const;

         /// get the real-world position of the given gridpoint
         bool getPosition (unsigned int i, unsigned int j, double &posI, double &posJ) const;

         /// get the real world position of the given point in the grid
         bool getPosition (double i, double j, double &posI, double &posJ) const;

         /// convert grid point to grid point of specified grid
         bool convertToGrid (const GlobalGrid & toGrid, unsigned int fromI, unsigned int fromJ,
	       unsigned int &toI, unsigned int &toJ) const;

         /// convert grid point to grid point of specified grid
         /// the point can be a fractional point
         bool convertToGrid (const GlobalGrid & toGrid, unsigned int fromI, unsigned int fromJ,
	       double &toI, double &toJ) const;

         /// get the realworld surface at the given grid coordinates
         double getSurface (int i, int j) const;
         /// get the real-world distance between the two given gridpoints
         double getDistance (int i1, int j1, int i2, int j2) const;

	 virtual void asString (string &) const;

private:
         const double m_minI;
         const double m_minJ;
         const double m_maxI;
         const double m_maxJ;
         const int m_numI;
         const int m_numJ;

         const double m_deltaI;
         const double m_deltaJ;
         const double m_surface;
      };
   }
}

#endif // _PARALLELDATAACCESS_GLOBALGRID_H_
