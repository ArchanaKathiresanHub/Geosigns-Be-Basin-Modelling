//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef ABSTRACTDERIVED_PROPERTIES__PROPERTY_SNAPSHOT_CALCULATOR_MAP_H
#define ABSTRACTDERIVED_PROPERTIES__PROPERTY_SNAPSHOT_CALCULATOR_MAP_H

// std library
#include <map>
#include <string>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"

// utilities library
#include "LogHandler.h"

namespace AbstractDerivedProperties {

   /// \brief Implements a double mapping from property -> snapshot -> calculator.
   ///
   /// If the snapshot cannot be found then a search for the null value is made
   /// if this cannot be found then a null calculator will be returned.
   template<typename CalculatorType>
   class PropertySnapshotCalculatorMap {

   public :

      /// \brief Insert a mapping from property->snapshot->calculator.
      void insert ( const DataModel::AbstractProperty* property,
                    const DataModel::AbstractSnapshot* snapshot,
                    const CalculatorType& calculator );

      /// \brief Insert a mapping from property->snapshot->calculator.
      CalculatorType get ( const DataModel::AbstractProperty* property,
                           const DataModel::AbstractSnapshot* snapshot ) const;

      /// \brief Determine if the calculator map contains a calculator for the property.
      bool contains ( const DataModel::AbstractProperty* property,
                      const DataModel::AbstractSnapshot* snapshot ) const;

   private :

      /// \brief A mapping from a snapshot to a calculator.
      typedef std::map<const DataModel::AbstractSnapshot*, CalculatorType> SnapshotCalculatorMap;

      /// \brief A mapping from a property to a SnapshotCalculatorMap.
      typedef std::map<const DataModel::AbstractProperty*, SnapshotCalculatorMap> PropertyCalculatorMap;

      /// \brief Contains the mapping.
      PropertyCalculatorMap m_propertySnapshotCalculatorMap;

   };

}

template<typename CalculatorType>
void AbstractDerivedProperties::PropertySnapshotCalculatorMap<CalculatorType>::insert ( const DataModel::AbstractProperty* property,
                                                                                const DataModel::AbstractSnapshot* snapshot,
                                                                                const CalculatorType& calculator ) {
   m_propertySnapshotCalculatorMap[property][snapshot] = calculator;
}


template<typename CalculatorType>
CalculatorType AbstractDerivedProperties::PropertySnapshotCalculatorMap<CalculatorType>::get ( const DataModel::AbstractProperty* property,
                                                                                       const DataModel::AbstractSnapshot* snapshot ) const {

   typename PropertyCalculatorMap::const_iterator propSsIter = m_propertySnapshotCalculatorMap.find ( property );

   if ( propSsIter != m_propertySnapshotCalculatorMap.end ()) {
      typename SnapshotCalculatorMap::const_iterator ssCalcIterator = propSsIter->second.find ( snapshot );

      if ( ssCalcIterator != propSsIter->second.end ()) {
         // The snapshot was found.
         return ssCalcIterator->second;
      } else if ( snapshot != nullptr ) {
         // The snapshot was not found so search for the null snapshot.
         ssCalcIterator = propSsIter->second.find ( 0 );

         if ( ssCalcIterator != propSsIter->second.end ()) {
            return ssCalcIterator->second;
         }

      }

   }

   if (property){
      std::string snapshotLog = "' @ snapshot " + std::to_string( snapshot ? snapshot->getTime() : -9999 ) + "Ma.";
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "Could not find calculator for derived property '" << property->getName()
         << (snapshot ? snapshotLog : "'.");
   }
   return CalculatorType ();
}


template<typename CalculatorType>
bool AbstractDerivedProperties::PropertySnapshotCalculatorMap<CalculatorType>::contains ( const DataModel::AbstractProperty* property,
                                                                                  const DataModel::AbstractSnapshot* snapshot ) const {
   return get ( property, snapshot ) != nullptr;
}

#endif // ABSTRACTDERIVED_PROPERTIES__PROPERTY_SNAPSHOT_CALCULATOR_MAP_H
