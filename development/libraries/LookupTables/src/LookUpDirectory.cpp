//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// std library
#include <vector>
#include <math.h>

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

#include "LookUpDirectory.h"

#include "CO2-H2O.h"
#include "CO2-H2O-NaCl1.h"
#include "CO2-H2O-NaCl2.h"
#include "CO2-H2O-NaCl4.h"
#include "CO2-H2O-NaCl6.h"
#include "H2S-H2O.h"
#include "H2S-H2O-NaCl1.h"
#include "H2S-H2O-NaCl2.h"
#include "H2S-H2O-NaCl4.h"
#include "H2S-H2O-NaCl6.h"

namespace TSR_Tables {

#define LENGTH(x) (sizeof(x)/sizeof(*x))
#define VECTOR(x) x, x+LENGTH(x)

}

TSR_Tables::LookUpDirectory::LookUpDirectory ()
{
   for (int i = 0; i < TSR_Tables::NumberOfTables; ++ i) {
      m_lookupTables[i] = 0;
   }

   m_tableData[H2S_H2Om] = TSR_Tables::H2S_H2O;
   m_tableData[H2S_H2O_NaCl1m] = TSR_Tables::H2S_H2O_NaCl1;
   m_tableData[H2S_H2O_NaCl2m] = TSR_Tables::H2S_H2O_NaCl2;
   m_tableData[H2S_H2O_NaCl4m] = TSR_Tables::H2S_H2O_NaCl4;
   m_tableData[H2S_H2O_NaCl6m] = TSR_Tables::H2S_H2O_NaCl6;
   m_tableData[CO2_H2Om] = TSR_Tables::CO2_H2O;
   m_tableData[CO2_H2O_NaCl1m] = TSR_Tables::CO2_H2O_NaCl1;
   m_tableData[CO2_H2O_NaCl2m] = TSR_Tables::CO2_H2O_NaCl2;
   m_tableData[CO2_H2O_NaCl4m] = TSR_Tables::CO2_H2O_NaCl4;
   m_tableData[CO2_H2O_NaCl6m] = TSR_Tables::CO2_H2O_NaCl6;
   
}

TSR_Tables::LookUpDirectory::~LookUpDirectory ()
{
   for (int i = 0; i < TSR_Tables::NumberOfTables; ++ i) {
      if (  m_lookupTables[i] != 0 ) {
         delete m_lookupTables[i];

         m_lookupTables [i] = 0;
      }
   }
}

const TSR_Tables::LookUp * TSR_Tables::LookUpDirectory::getTable( TSR_Table aTable ) 
{
   if( m_lookupTables[aTable] == 0 ) {
      allocateTable( aTable );
   }
   return m_lookupTables[aTable];
}

bool TSR_Tables::LookUpDirectory::getValueForSalinity( const double salinity, ComponentId id,
                                                       const double pressure, const double temperature, 
                                                       double & tableValue )
{
   
   const double molal = convertSalinityToMolal ( salinity );
   const double minOffset = 0.05;

   const double minSalinity = floor( molal );
   const double maxSalinity = ceil( molal );

   const TSR_Tables::TSR_Table minTableName = getLowTableName( minSalinity, id );

   if( minTableName == NumberOfTables) {
      return false;
   }
   if( maxSalinity - minSalinity < minOffset ) {
      getTable( minTableName )->get(  pressure, temperature, tableValue );
      return true;
   }
 
   const TSR_Tables::TSR_Table maxTableName = getUpTableName( maxSalinity, id );

   if( maxTableName == NumberOfTables ) {
      return false;
   }

   double minValue, maxValue;

   getTable( minTableName )->get( pressure, temperature, minValue );
   getTable( maxTableName )->get( pressure, temperature, maxValue );

   //   cout << "Min: " << minTableName << ", val = " << minValue.first << ", Max: " << maxTableName << " val = " << maxValue.first << endl;
   
   const double scaleValue = ( molal - minSalinity ) / ( maxSalinity - minSalinity );

   tableValue = minValue + ( maxValue - minValue ) * scaleValue;

   return true;
}

TSR_Tables::TSR_Table TSR_Tables::LookUpDirectory::getTableName ( const double salinity, ComponentId id )
{
   const double molal = convertSalinityToMolal ( salinity );

   return getLowTableName ( molal, id );
}

TSR_Tables::TSR_Table TSR_Tables::LookUpDirectory::getUpTableName ( const double molal, ComponentId id )
{

   if( id == ComponentId::H2S ) {
      if( molal <= 1 ) return H2S_H2O_NaCl1m;
      if( molal <= 2 ) return H2S_H2O_NaCl2m;
      if( molal <= 4 ) return H2S_H2O_NaCl4m;
      if( molal <= 6 ) return H2S_H2O_NaCl6m;
      return H2S_H2O_NaCl6m;
   }
   if( id == ComponentId::COX ) {
      if( molal <= 1 ) return CO2_H2O_NaCl1m;
      if( molal <= 2 ) return CO2_H2O_NaCl2m;
      if( molal <= 4 ) return CO2_H2O_NaCl4m;
      if( molal <= 6 ) return CO2_H2O_NaCl6m;
      return CO2_H2O_NaCl6m;
   }
   return NumberOfTables;
}

TSR_Tables::TSR_Table TSR_Tables::LookUpDirectory::getLowTableName ( const double molal, ComponentId id )
{
   if( id == ComponentId::H2S ) {
      if( molal < 1 ) return H2S_H2Om;
      if( molal < 2 ) return H2S_H2O_NaCl1m;
      if( molal < 4 ) return H2S_H2O_NaCl2m;
      if( molal < 6 ) return H2S_H2O_NaCl4m;
      return H2S_H2O_NaCl6m;
   }
   if( id == ComponentId::COX ) {
      if( molal < 1 ) return CO2_H2Om;
      if( molal < 2 ) return CO2_H2O_NaCl1m;
      if( molal < 4 ) return CO2_H2O_NaCl2m;
      if( molal < 6 ) return CO2_H2O_NaCl4m;
      return CO2_H2O_NaCl6m;
   }
   return NumberOfTables;
}


void TSR_Tables::LookUpDirectory::allocateTable( TSR_Table aTable )
{
   size_t tableSize = LENGTH(TSR_Tables::press) * LENGTH(TSR_Tables::temp);
   m_lookupTables[aTable] = new LookUp(std::vector<double>(VECTOR(TSR_Tables::press)),
                                       std::vector<double>(VECTOR(TSR_Tables::temp)), 
                                       std::vector<double>(m_tableData[aTable], m_tableData[aTable] + tableSize ));

}



