//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "MockDerivedSurfaceProperty.h"

namespace DataModel {

   MockDerivedSurfaceProperty::MockDerivedSurfaceProperty( const int firstI,
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
      const double surfaceValue ) :
         m_mockGrid( firstI, firstGhostI, firstJ, firstGhostJ, lastI, lastGhostI, lastJ, lastGhostJ ),
         m_mockSurface( surfaceName ),
         m_mockSnapshot( snapshotAge ),
         m_mockProperty( propertyName )
   {
      m_derivedSurfacePropertyPtr = DerivedProperties::DerivedSurfacePropertyPtr( new DerivedProperties::DerivedSurfaceProperty( &m_mockProperty,
                                                                                                                                 &m_mockSnapshot,
                                                                                                                                 &m_mockSurface,
                                                                                                                                 &m_mockGrid ) );
         //Initialize all grid values to surfaceValue
         for (int i = m_mockGrid.firstI( true ); i < m_mockGrid.lastI( true ) + 1; ++i) {
            for (int j = m_mockGrid.firstJ( true ); j < m_mockGrid.lastJ( true ) + 1; ++j) {
               m_derivedSurfacePropertyPtr->set( i, j, surfaceValue );
            }
         }

   }

}

