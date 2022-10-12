//
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MockGRPropertyManager.h"

// 3rdparty
#include "petsc.h"

// DataAccess libraries
#include "DistributedGrid.h"

// Derived Properties src
#include "GammaRayFormationCalculator.h"

// Derived Properties test
#include "MockGrid.h"
#include "MockPorosityCalculator.h"

#include <algorithm>

namespace DataModel{

   struct MPIHelper {
      MPIHelper() {
         PetscInitialize( 0, 0, 0, 0 );
      }

      ~MPIHelper() {
         MPI_Barrier( PETSC_COMM_WORLD );
         PetscFinalize();
      }

      static MPIHelper & instance() {
         static MPIHelper object;
         return object;
      }

      static void barrier() {
         instance();
         MPI_Barrier( PETSC_COMM_WORLD );
      }
   };


   MockGRPropertyManager::MockGRPropertyManager (DataAccessMode mode) {
      // These will come from the project handle.
      m_mockProperties.push_back ( std::make_shared<DataModel::MockProperty>( "Porosity", DataModel::DISCONTINUOUS_3D_PROPERTY ) );
      m_mockProperties.push_back (std::make_shared< DataModel::MockProperty>( "GammaRay", DataModel::DISCONTINUOUS_3D_PROPERTY ) );

      // Add all properties to the property manager.
      for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
        addProperty ( (m_mockProperties [ i ]).get());
      }

      MPIHelper::barrier();
      // This will come from the project handle.
      switch(mode){
         case SERIAL:
           m_mapGrid = std::make_shared<DataModel::MockGrid>( 0, 0, 0, 0, 10, 10, 10, 10 );
           break;
         case DISTRIBUTED:
           m_mapGrid = std::make_shared<DataAccess::Interface::DistributedGrid>( 0, 0, 4, 4, 10, 10, 10, 10 );
           break;
         default:
           m_mapGrid = std::make_shared<DataModel::MockGrid>( 0, 0, 0, 0, 10, 10, 10, 10 );
      }
      MPIHelper::barrier();

      addFormationPropertyCalculator ( AbstractDerivedProperties::FormationPropertyCalculatorPtr ( new MockPorosityCalculator ));
      addFormationPropertyCalculator ( AbstractDerivedProperties::FormationPropertyCalculatorPtr ( new DerivedProperties::GammaRayFormationCalculator() ));
   }

   const DataModel::AbstractGrid* MockGRPropertyManager::getMapGrid () const {
      return m_mapGrid.get();
   }

   const DataModel::AbstractProperty* MockGRPropertyManager::getProperty ( const std::string& name ) const {

      for ( size_t i = 0; i < m_properties.size (); ++i ) {

         if ( m_properties [ i ]->getName () == name ) {
            return m_properties [ i ];
         }

      }

      return 0;
   }

   bool MockGRPropertyManager::getNodeIsValid ( const unsigned int i, const unsigned int j ) const {
     if( (i+j) % 2 == 0){
        return true;
      }else{
        return false;
      }
   }

   void MockGRPropertyManager::addProperty ( const DataModel::AbstractProperty* property ) {

      if ( std::find ( m_properties.begin (), m_properties.end (), property ) == m_properties.end ()) {
         m_properties.push_back ( property );
      }

   }

}
