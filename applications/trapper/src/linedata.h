/// Class LineData holds the x points and y points for a graph
/// It allows x,y points to be added easily and then returns them to
/// the QWT::Curve type in the format it needs them in

#ifndef __linedata__
#define __linedata__

#include "simplesmartpointer.h"

class LineData
{
public:
   // ctor / dtor
   LineData (); 
   LineData (int size);
   LineData (const LineData& rhs);
   ~LineData (); 

   // public methods
   void setSize (int size);
   inline int size () const;
   inline int used () const;
   void add (double x, double y);  
   inline double* getXPoints (); 
   inline double *getYPoints (); 
   inline const double* getXPoints () const;
   inline const double* getYPoints () const;
   inline void resetIndex (void); 
   inline int startIndex (); 

   // public operators
   LineData& operator= (const LineData &rhs);

private:
   void copyPoints (double *src, double *dest, int size);
   void copy (const LineData &rhs);
   
   // private typedefs
   typedef SimpleSmartPointer <double, Array1dPolicy<double> > DoubleSmartPointer;
   
   // private variables
   int m_numPoints;
   int m_index;
   double *m_xPoints;
   double *m_yPoints;
};
 
// inline methods
int LineData::size () const
{ 
   return m_numPoints; 
}

int LineData::used () const 
{ 
   return m_index+1; 
}

double* LineData::getXPoints ()
{
   return m_xPoints; 
}

double* LineData::getYPoints ()
{
   return m_yPoints; 
}

const double* LineData::getXPoints () const
{
   return m_xPoints; 
}

const double* LineData::getYPoints () const
{
   return m_yPoints; 
}

void LineData::resetIndex (void)
{
   m_index = startIndex(); 
}

int LineData::startIndex ()
{ 
   return -1; 
}

#endif
