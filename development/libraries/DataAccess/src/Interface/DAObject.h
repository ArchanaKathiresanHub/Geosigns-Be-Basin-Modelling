//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_DAOBJECT_H
#define INTERFACE_DAOBJECT_H

namespace database
{
   class Database;
   class Table;
   class Record;
}

// DataAccess library
#include "Interface/Parent.h"
#include "Interface/AttributeValue.h"
#include "Interface/GridMap.h"
#include "Interface/ObjectFactory.h"
#include "Interface/ProjectHandle.h"

// std library
#include <assert.h>
#include <string>
#include <exception>

using namespace std;
using DataAccess::Interface::GridMap;

namespace DataAccess
{
   namespace Interface
   {

      /// Classes whose objects need to refer to a TableIO record to get at their data, inherit from this class
      /// as this class contains the code to do so.
      /// An object of this class can also refer back to the ProjectHandle that owns it.
      class DAObject : public Parent
      {
         public:
            DAObject (ProjectHandle * projectHandle, database::Record * record);
            virtual ~DAObject (void);

            /// return the record
            database::Record * getRecord (void) const;
            /// set the record
            void setRecord (database::Record * record);

            /// return the project handle
            ProjectHandle * getProjectHandle (void) const;

            /// return the object factory
            ObjectFactory * getFactory (void) const;

            /// @brief Return the GridMap (mapAttributeNames[attributeId]) associated to one record's attributes
            /// @details If the attribute is a value (so not a string linking to a map)
            ///          then returns a GridMap filled with that value
            ///          If the attribute is a string linking to a map then simply returns the map
            /// @param[in] attributeId The map ID which is also its position in the list of map names
            /// @param[in] mapAttributeNames The list of map names
            template< typename T >
            const GridMap * getMap( const T attributeId, const std::vector<std::string>& mapAttributeNames ) const;

         protected:

            ProjectHandle * m_projectHandle;
            mutable database::Record * m_record;

            AttributeValue & getAttributeValue (const string & attributeName, unsigned int indexOffset) const;
            AttributeType getAttributeType (const string & attributeName) const;
            int getAttributeIndex (const string & attributeName) const;

         private:
            /// @brief Load a the map (mapAttributeNames[attributeId]) from the record
            /// @details If the attribute is a value (so not a string linking to a not a map)
            ///          then returns a GridMap filled with that value
            ///          If the attribute is a string linking to a map then simply returns the map
            /// @param[in] attributeId The map ID which is also its position in the list of map names
            /// @param[in] mapAttributeNames The list of map names
            template< typename T >
            GridMap * loadMap( const T attributeId, const std::vector<std::string>& mapAttributeNames ) const;
      };
   }
}




template< typename T >
const GridMap * DataAccess::Interface::DAObject::getMap( const T attributeId, const std::vector<std::string>& mapAttributeNames ) const{
   const unsigned int attributeIndex = (unsigned int)attributeId;
   const GridMap * gridMap = nullptr;

   if ((gridMap = (GridMap *)getChild( attributeIndex )) == false)
   {
      gridMap = loadMap( attributeId, mapAttributeNames );
   }
   if (gridMap == nullptr){
      throw std::runtime_error( "Basin_Error: Could not load map " + mapAttributeNames[attributeId] );
   }
   return gridMap;
}

template< typename T >
GridMap * DataAccess::Interface::DAObject::loadMap( const T attributeId, const std::vector<std::string>& mapAttributeNames ) const{
   const unsigned int attributeIndex = (unsigned int)attributeId;

   const string attributeGridName = mapAttributeNames[attributeIndex] + "Grid";
   const string& valueGridMapId = m_record->getValue<std::string>( attributeGridName );

   GridMap * gridMap = nullptr;
   // if there is a map specified then create it
   if (valueGridMapId.length() != 0)
   {
      gridMap = m_projectHandle->loadInputMap( m_record->tableName(), valueGridMapId );
   }
   // else if there is a scalar specified then create a map filled with this scalar
   else
   {
      const double value = m_record->getValue<double>( mapAttributeNames[attributeIndex] );
      if (value != RecordValueUndefined)
      {
         const Grid * grid = m_projectHandle->getActivityOutputGrid();
         if (grid == nullptr) grid = (const Grid *)m_projectHandle->getInputGrid();
         gridMap = m_projectHandle->getFactory()->produceGridMap( this, attributeIndex, grid, value );

         assert( gridMap == getChild( attributeIndex ) );
      }
   }
   return gridMap;
}

#endif // INTERFACE_DAOBJECT_H
