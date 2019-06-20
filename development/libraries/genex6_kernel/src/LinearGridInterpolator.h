//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _GENEX6_LIBRARY__LINEAR_GRID_INTERPOLATOR_H_
#define _GENEX6_LIBRARY__LINEAR_GRID_INTERPOLATOR_H_

#include "GridMap.h"
#include "Snapshot.h"

#include "LocalGridInterpolator.h"

#include "SurfaceProperty.h"
#include "FormationProperty.h"
#include "FormationMapProperty.h"
#include "FormationSurfaceProperty.h"

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

      void compute ( const DataAccess::Interface::Snapshot* start, 
                     const AbstractDerivedProperties::SurfacePropertyPtr dataAtStart, 
                     const DataAccess::Interface::Snapshot* end, 
                     const AbstractDerivedProperties::SurfacePropertyPtr dataAtEnd );

      void compute ( const DataAccess::Interface::Snapshot* start, 
                     const AbstractDerivedProperties::FormationPropertyPtr dataAtStart, 
                     const DataAccess::Interface::Snapshot* end, 
                     const AbstractDerivedProperties::FormationPropertyPtr dataAtEnd );

      void compute ( const DataAccess::Interface::Snapshot* start, 
                     const AbstractDerivedProperties::FormationMapPropertyPtr dataAtStart, 
                     const DataAccess::Interface::Snapshot* end, 
                     const AbstractDerivedProperties::FormationMapPropertyPtr dataAtEnd );

      void compute ( const DataAccess::Interface::Snapshot* start, 
                     const AbstractDerivedProperties::FormationSurfacePropertyPtr dataAtStart, 
                     const DataAccess::Interface::Snapshot* end, 
                     const AbstractDerivedProperties::FormationSurfacePropertyPtr dataAtEnd );

      virtual double evaluateProperty(const int &I, const int &J, const double &in_timeInstance) const;

   protected:

      enum BufferType{A, B};

      void setValueFromGridMap(const double &value, const int &GridI,const int &GridJ, BufferType theType);
      double getValueForGrid(const int &GridI, const int &GridJ, BufferType theType) const ;

      void initialize(const DataAccess::Interface::GridMap *theGrid);
      void initialize(const AbstractDerivedProperties::SurfacePropertyPtr theProperty);
      void initialize(const AbstractDerivedProperties::FormationPropertyPtr theProperty);
      void initialize(const AbstractDerivedProperties::FormationMapPropertyPtr theProperty);
      void initialize(const AbstractDerivedProperties::FormationSurfacePropertyPtr theProperty);

      void clear();
      double **allocate2DArray(const int &rows, const int &columns);
      void delete2DArray(double **array, const int &rows);

   private:

      double **m_A;
      double **m_B;
      unsigned int m_firstI;
      unsigned int m_lastI;
      unsigned int m_firstJ;
      unsigned int m_lastJ;
      int m_rows;
      int m_columns;

      LinearGridInterpolator(const LinearGridInterpolator &);
      LinearGridInterpolator &operator=(const LinearGridInterpolator &);

   };


}

#endif // _GENEX6_LIBRARY__LINEAR_GRID_INTERPOLATOR_H_
