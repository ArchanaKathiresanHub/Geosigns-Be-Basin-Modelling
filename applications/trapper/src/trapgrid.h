/// Class TrapGrid handles specifically the reading
/// of the ResRockTrapId_<age>__<ReservoirName>.HDF files
/// found in the OutputDir of a Cauldron Project

#ifndef __trapgrid__
#define __trapgrid__

#include <string>
using namespace std;

class H5_ReadOnly_File;

class TrapGrid
{
public:
   // typedefs
   typedef int PointType;
   typedef int GridType;
   typedef float CoordType;

   // ctor / dtor
   TrapGrid (void); 
   TrapGrid (const TrapGrid& rhs);
   ~TrapGrid (void);

   // operators
   TrapGrid& operator= (const TrapGrid& rhs);

   // methods 
   void readGrid (H5_ReadOnly_File *inFile, const string & dataset);
   
   inline PointType minX (void) const;
   inline PointType maxX (void) const;
   inline PointType minY (void) const; 
   inline PointType maxY (void) const; 
   inline CoordType eastMin (void) const;
   inline CoordType northMin (void) const;
   inline CoordType deltaX (void) const;
   inline CoordType deltaY (void) const;
   inline const string& gridName (void) const;
   const GridType& getVal (const int i, const int j) const; 
 
private:
   void deepCopy (const TrapGrid &rhs);
   void copyGridData (const TrapGrid &rhs);
   void initGridData (const int maxX, const int maxY);
   void copyGridCoords (const TrapGrid &rhs);
   void deleteGridData (void);
   void copy1dDataToGridData (float *gridData, const int maxX, 
                              const int maxY);
   
   // variables
   PointType m_maxX;
   PointType m_maxY;
   CoordType m_eastMin;
   CoordType m_northMin;
   CoordType m_deltaX;
   CoordType m_deltaY;
   GridType **m_gridData;
   string m_gridname;
};

//
// inline methods
//
TrapGrid::PointType TrapGrid::minX (void) const
{
   return 0; 
}

TrapGrid::PointType TrapGrid::maxX (void) const
{
   return m_maxX; 
}

TrapGrid::PointType TrapGrid::minY (void) const
{
   return 0; 
}

TrapGrid::PointType TrapGrid::maxY (void) const
{
   return m_maxY; 
}

TrapGrid::CoordType TrapGrid::eastMin (void) const
{
   return m_eastMin;
}

TrapGrid::CoordType TrapGrid::northMin (void) const
{
   return m_northMin;
}

TrapGrid::CoordType TrapGrid::deltaX (void) const
{
   return m_deltaX;
}

TrapGrid::CoordType TrapGrid::deltaY (void) const
{
   return m_deltaY;
}

const string& TrapGrid::gridName (void) const
{
   return m_gridname; 
}

#endif
