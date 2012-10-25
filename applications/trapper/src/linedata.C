/// Class LineData holds the x points and y points for a graph
/// It allows x,y points to be added easily and then returns them to
/// the QWT::Curve type in the format it needs them in

#include "linedata.h"
#include "includestreams.h"

//
// CTOR / DTOR
//
LineData::LineData ()
   : m_numPoints (0), m_xPoints (0), m_yPoints (0), m_index (startIndex()) 
{}

LineData::LineData (int size)
   : m_index (startIndex())
{
   setSize (size);
}

LineData::LineData (const LineData &rhs)
{
   copy (rhs);
}

LineData::~LineData ()
{
   if ( m_numPoints > 0 ) 
   {
      delete [] m_xPoints; delete [] m_yPoints; 
   } 
}

//
// PUBLIC METHODS
//
/// LineData::setSize sets  potential size for the x and y points
/// the memory is allocated but a variable m_index indicates
/// how many points are actually assigned to
void LineData::setSize (int size)
{
   if ( size > 0 )
   {
      DoubleSmartPointer tempX (new double[size]);
      m_yPoints = new double [size];
      m_xPoints = tempX.release ();
      m_numPoints = size;
   }
   else
   {
      m_numPoints = 0;
      m_xPoints = 0;
      m_yPoints = 0;
   }
}

/// LineData::add adds new points to the line
/// incrementing the m_index variable
/// if the line is full an error message is sent to the console
void LineData::add (double x, double y)
{
   int newIndex = m_index+1;
   
   if ( newIndex < m_numPoints )
   {
      m_xPoints[newIndex] = x;
      m_yPoints[newIndex] = y;
      m_index = newIndex;
   }
   else
   {
      cout << endl << "Error, trying to add to position " << newIndex 
            << ", max allowed = " << m_numPoints << endl; 
   }
}

LineData& LineData::operator= (const LineData& rhs)
{
   copy (rhs);
   return *this;
}

//
// PRIVATE METHODS
//
/// LineData::copy performs a deep copy on x and y points and 
/// other data from one LineData object to another
void LineData::copy (const LineData &rhs)
{
   if ( rhs.m_numPoints > 0 ) 
   {      
      DoubleSmartPointer tempX (new double[rhs.m_numPoints]);
      m_yPoints = new double [rhs.m_numPoints]; 
      m_xPoints = tempX.release ();
      
      copyPoints (rhs.m_xPoints, m_xPoints, rhs.m_index);
      copyPoints (rhs.m_yPoints, m_yPoints, rhs.m_index);
   }
   
   m_numPoints = rhs.m_numPoints;
   m_index = rhs.m_index;
}

void LineData::copyPoints (double *src, double *dest, int size)
{
   for ( int i=0; i < size; ++i )
   {
      dest[i] = src[i];
   }
}
