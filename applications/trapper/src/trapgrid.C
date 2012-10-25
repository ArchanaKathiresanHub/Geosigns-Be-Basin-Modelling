/// Class TrapGrid handles specifically the reading
/// of the ResRockTrapId_<age>__<ReservoirName>.HDF files
/// found in the OutputDir of a Cauldron Project

#include "trapgrid.h"
#include "buffer_readwrite.h"
#include "globalnumbers.h" 
using namespace Null_Values;

//
// ctor / dtor
//
TrapGrid::TrapGrid (void) 
   : m_maxX(0), m_maxY(0), 
   m_eastMin(0), m_northMin(0),
   m_deltaX(0), m_deltaY(0),
   m_gridData(0)
{}

TrapGrid::TrapGrid (const TrapGrid& rhs)
{
   deepCopy (rhs);
}
 
TrapGrid::~TrapGrid (void)
{
   deleteGridData ();
}

TrapGrid& TrapGrid::operator= (const TrapGrid& rhs)
{
   deepCopy (rhs);
   return *this;
}
 
/// TrapGrid::readGrid extracts grid data from a HDF5 grid file
void TrapGrid::readGrid (H5_ReadOnly_File *inFile, const string & dataset)
{
   // define reader and dimensions
   Buffer_ReadWrite reader;
   BufferDimensions dims;

   // get maxX
   PointType *maxX;
   reader.readDataset (inFile, inFile->fileId(), "number in I dimension", 
                       dims, (void**)&maxX);
   m_maxX = maxX[0];
   delete [] maxX;

   // get maxY
   PointType *maxY;
   reader.readDataset (inFile, inFile->fileId(), "number in J dimension", 
                       dims, (void**)&maxY);
   m_maxY = maxY[0];
   delete [] maxY;
   
   // east min
   CoordType *eastMin;
   reader.readDataset (inFile, inFile->fileId(),  "origin in I dimension",
                       dims, (void**)&eastMin);
   m_eastMin = eastMin[0];
   delete []eastMin;
   
   // north min
   CoordType *northMin;
   reader.readDataset (inFile, inFile->fileId(),  "origin in J dimension",
                       dims, (void**)&northMin);
   m_northMin = northMin[0];
   delete []northMin;
   
   // delta X
   CoordType *deltaX;
   reader.readDataset (inFile, inFile->fileId(), "delta in I dimension",
                       dims, (void**)&deltaX);
   m_deltaX = deltaX[0];
   delete []deltaX;
   
    // delta Y
   CoordType *deltaY;
   reader.readDataset (inFile, inFile->fileId(), "delta in J dimension",
                       dims, (void**)&deltaY);
   m_deltaY = deltaY[0];
   delete []deltaY;
   
   // grid name
   char *gridname;
   string fullDataset = "Layer=";
   fullDataset += dataset;

   /*
   hid_t dataId = inFile->openDataset (fullDataset.c_str ());
   reader.readAttribute (inFile, dataId, "GridName", 
                         dims, (void**)&gridname);
   
   // add null to end of string
   gridname[dims[0]] = '\0';
   m_gridname = gridname;
   inFile->closeDataset (dataId);
   delete [] gridname;
   */

   m_gridname = dataset;

   // get grid data
   float *gridData;
   reader.readDataset (inFile, inFile->fileId(), fullDataset.c_str (),
                                    dims, (void**)&gridData);

   // copy the file data to this grid data,
   // converting from 1d to 2d
   copy1dDataToGridData (gridData, m_maxX, m_maxY);

   delete [] gridData;
}

const TrapGrid::GridType&  TrapGrid::getVal (const int i, const int j) const 
{ 
   if ( i < 0 || i >= m_maxX ||
        j < 0 || j >= m_maxY )
   {
      cout << endl <<  "TrapGrid::getVal" 
             << endl << "Error, " << i << "," << j << " outside range" << endl;
      return Null_Int;
   }
   else
   {
      return m_gridData[i][j];
   }
}

void TrapGrid::deepCopy (const TrapGrid &rhs)
{
   copyGridData   (rhs);
   copyGridCoords (rhs);
   m_gridname = rhs.m_gridname;
}

void TrapGrid::copyGridData (const TrapGrid &rhs)
{
   initGridData (rhs.m_maxX, rhs.m_maxY);

   for ( int i = 0; i < rhs.m_maxX; ++i )
   {    
      for ( int j = 0; j < rhs.m_maxY; ++j )
      {
         m_gridData[i][j] = rhs.m_gridData[i][j];
      } 
   } 
}

void TrapGrid::initGridData (const int maxX, const int maxY)
{
   // for exception safety assign memory to 
   // temporary pointer first
   GridType **temp;

   temp = new GridType* [maxX];

   int i, ii;
   try
   {
      for ( i = 0; i < maxX; ++i )
      {
         temp[i] = new GridType [maxY];
      }
   }
   catch (...)
   {
      for ( ii=0; ii < i; ++ii ) delete [] temp[ii];
      delete [] temp;     
   } 
  
   // if no exception occured then can erase old memory
   // and point to new
   deleteGridData ();
   m_gridData = temp;
}

void TrapGrid::copyGridCoords (const TrapGrid &rhs)
{
   m_maxX = rhs.m_maxX;
   m_maxY = rhs.m_maxY;
}

void TrapGrid::deleteGridData (void)
{
   if ( ! m_gridData ) return;
  
   for ( int i = 0; i < m_maxX; ++i )
   {
      delete [] m_gridData [i];
   }

   delete [] m_gridData;

   m_gridData = 0;
}

void TrapGrid::copy1dDataToGridData (float *gridData, 
                                     const int maxX, const int maxY)
{
   initGridData (maxX, maxY);
  
   for ( int i=0; i < maxX; ++i )
   {
      for ( int j=0; j < maxY; ++j )
      {
         m_gridData[i][j] = static_cast<int>(gridData[(i*maxY) + j]);
      }
   } 
}




