//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__OUTPUT_PROPERTY_VALUE__H
#define DERIVED_PROPERTIES__OUTPUT_PROPERTY_VALUE__H

#include "Interface.h"
#include "AbstractProperty.h"
#include "AbstractSurface.h"
#include "AbstractReservoir.h"
#include "AbstractGrid.h"

#include <memory>

namespace  DerivedProperties {

   /// \brief Stores the designated property to be calculated and saved to disk
   class OutputPropertyValue {

   public :

      OutputPropertyValue ( const DataModel::AbstractProperty* property );

      virtual ~OutputPropertyValue () = default;

      /// \brief Get the value of the property at the position i,j,k
      virtual double getValue ( const double i, const double j, const double k ) const = 0;

      /// \brief Get the number of nodes in the z direction
      virtual unsigned int getDepth () const = 0;

      /// \brief Get the property undefined value
      virtual double getUndefinedValue () const;

      /// \brief Get the name of the property
      virtual const string & getName() const = 0;

      /// \brief Get the grid on which the property values are defined.
      virtual const DataModel::AbstractGrid* getGrid () const = 0;

      /// \brief Get the surface for which the property values are defined.
      virtual const DataModel::AbstractSurface* getSurface() const { return nullptr; }

      /// \brief Get the reservoir for which the property values are defined.
      virtual const DataModel::AbstractReservoir* getReservoir() const { return nullptr; }

      /// \brief Get the property
      virtual const DataModel::AbstractProperty* getProperty () const;

      /// \brief Retreive the grid map
      virtual void retrieveData() const = 0;

      /// \brief Restore the grid map
      virtual void restoreData() const = 0;

      /// \brief Determine whether or not the property is a primary property
      virtual bool isPrimary() const = 0;

      /// \brief Determine whether or not the property value has a property
      virtual bool hasProperty() const = 0;

      /// \brief Get the map name if it's not equal to the propetry name
      virtual const string & getMapName() const { return getName(); }

      static std::string s_nullString;
  private :

      const DataModel::AbstractProperty* m_property;

   };

   typedef std::shared_ptr<OutputPropertyValue> OutputPropertyValuePtr;

   inline OutputPropertyValue::OutputPropertyValue ( const DataModel::AbstractProperty* property ) : m_property ( property ) {}

   inline const DataModel::AbstractProperty* OutputPropertyValue::getProperty () const {
      return m_property;
   }

   inline double OutputPropertyValue::getUndefinedValue () const {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }
}

#endif // DERIVED_PROPERTIES__OUTPUT_PROPERTY_VALUE__H
