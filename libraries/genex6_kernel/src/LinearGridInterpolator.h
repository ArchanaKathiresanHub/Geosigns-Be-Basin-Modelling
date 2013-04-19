#ifndef _GENEX6_LIBRARY__LINEAR_GRID_INTERPOLATOR_H_
#define _GENEX6_LIBRARY__LINEAR_GRID_INTERPOLATOR_H_

#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"

#include "LocalGridInterpolator.h"

namespace Genex6 {

   // Put in separate file.
   class LinearGridInterpolator : public LocalGridInterpolator {

   public:

      ///constructor
      LinearGridInterpolator();
   
      ///destructor
      ~LinearGridInterpolator();
   
      void compute ( const DataAccess::Interface::Snapshot* start, 
                     const DataAccess::Interface::GridMap*  dataAtStart, 
                     const DataAccess::Interface::Snapshot* end, 
                     const DataAccess::Interface::GridMap*  dataAtEnd);

      void compute ( const double                          start, 
                     const DataAccess::Interface::GridMap* dataAtStart, 
                     const double                          end, 
                     const DataAccess::Interface::GridMap* dataAtEnd);

      virtual double evaluateProperty(const int &I, const int &J, const double &in_timeInstance) const;

   protected:

      enum BufferType{A, B};

      void setValueFromGridMap(const double &value, const int &GridI,const int &GridJ, BufferType theType);
      double getValueForGrid(const int &GridI, const int &GridJ, BufferType theType) const ;
      void initialize(const DataAccess::Interface::GridMap *theGrid);
      void clear();
      double **allocate2DArray(const int &rows, const int &columns);
      void delete2DArray(double **array, const int &rows);

   private:

      double **m_A;
      double **m_B;
      int m_firstI;
      int m_lastI;
      int m_firstJ;
      int m_lastJ;
      int m_rows;
      int m_columns;

      LinearGridInterpolator(const LinearGridInterpolator &);
      LinearGridInterpolator &operator=(const LinearGridInterpolator &);

   };


}

#endif // _GENEX6_LIBRARY__LINEAR_GRID_INTERPOLATOR_H_
