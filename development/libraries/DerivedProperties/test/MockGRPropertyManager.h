//                                                                      
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__GRTESTPROPERTYMANAGER_H
#define DERIVED_PROPERTIES__GRTESTPROPERTYMANAGER_H

#include <vector>

// DataModel
#include "AbstractGrid.h"
#include "AbstractProperty.h"

// Derived Properties src
#include "AbstractPropertyManager.h"

// Derived Properties test
#include "MockProperty.h"


namespace DataModel{
   /// \brief Mock gamma ray test PropertyManager
   ///
   /// defines a mock porosity and calculate the gamma ray value.
   class MockGRPropertyManager final: public AbstractDerivedProperties::AbstractPropertyManager {

   public :
     
      enum DataAccessMode {
         SERIAL,
         DISTRIBUTED
      };
     
      /// \brief Initialize a mock gamma ray test PropertyManager
      /// \param [in]  mode Flag allowing to switch between a sequential(mock grid) and a distributed grid.
      MockGRPropertyManager (DataAccessMode mode);
  
      /// \brief Get the property given the property-name
      /// \details If the name is not found then a null pointer will be returned.
      /// \param [in] name The name of the required property. 
      const DataModel::AbstractProperty* getProperty ( const std::string& name ) const final;
      
      /// \brief Get the grid for the map
      const DataModel::AbstractGrid* getMapGrid () const final;
   
      /// \brief Return whether or not the node is defined
      /// \details  For test purpose this function returns true if i+j is even and false otherwise
      bool getNodeIsValid ( const unsigned int i, const unsigned int j ) const final;
   
   private :
  
      /// \brief List of all available properties
      typedef std::vector <const DataModel::AbstractProperty* > PropertyList;
   
   
      /// \brief Add an abstract property to the list of available properties
      /// \details If a property has been added already then it will not be added a second time.
      void addProperty ( const DataModel::AbstractProperty* property );
   
   
      /// \brief Contains list of all known (mock) properties
      PropertyList m_properties;

      /// \brief Contains the (mock) properties   
      std::vector< std::shared_ptr<MockProperty> > m_mockProperties;

      /// \brief Contains the grid
      std::shared_ptr<DataModel::AbstractGrid> m_mapGrid;

   };
}
#endif // DERIVED_PROPERTIES__GRTESTPROPERTYMANAGER_H
