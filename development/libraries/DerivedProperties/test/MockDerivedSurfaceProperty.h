//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _DERIVED_PROPERTIES__MOCK_DERIVEDSURFACEPPTY_H_
#define _DERIVED_PROPERTIES__MOCK_DERIVEDSURFACEPPTY_H_

#include "../src/DerivedSurfaceProperty.h"

#include "MockProperty.h"
#include "MockSnapshot.h"
#include "MockSurface.h"
#include "MockGrid.h"


namespace DataModel {

   /// @brief Describes a derived surface property.
   ///
   /// This class should be used in unit tests to create a derived surface property.
   class MockDerivedSurfaceProperty {

   public :

      /// @param firstI The first I index
      /// @param firstJ The first J index
      /// @param firstGhostI The first gosth I index
      /// @param firstGhostJ The first gosth J index
      /// @param lastI The last I index
      /// @param lastJ The last J index
      /// @param lastGhostI The last gosth I index
      /// @param lastGhostJ The last gosth J index
      /// @param surfaceName The name of the surface which owns the property
      /// @param propertyName The name of the property
      /// @param snapshotAge The age of the snapshot
      /// @param surfaceValue The inital value of the derived surface property (at every node)
      MockDerivedSurfaceProperty( const int firstI,
                                  const int firstJ,
                                  const int firstGhostI,
                                  const int firstGhostJ,
                                  const int lastI,
                                  const int lastJ,
                                  const int lastGhostI,
                                  const int lastGhostJ,
                                  const std::string surfaceName,
                                  const std::string propertyName,
                                  const double snapshotAge,
                                  const double surfaceValue );

      ~MockDerivedSurfaceProperty() {};

      const DerivedProperties::DerivedSurfacePropertyPtr getMockderivedSurfacePropertyPtr() const { return m_derivedSurfacePropertyPtr; };

   private :

      DataModel::MockGrid             m_mockGrid;
      const DataModel::MockSurface    m_mockSurface;
      const DataModel::MockSnapshot   m_mockSnapshot;
      const DataModel::MockProperty   m_mockProperty;

      DerivedProperties::DerivedSurfacePropertyPtr m_derivedSurfacePropertyPtr;

   };

}

#endif // _DERIVED_PROPERTIES__MOCK_DERIVEDSURFACEPPTY_H_