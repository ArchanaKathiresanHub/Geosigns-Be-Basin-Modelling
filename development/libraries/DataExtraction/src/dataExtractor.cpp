//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "dataExtractor.h"

#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
#include "database.h"

#include "Formation.h"
#include "Property.h"
#include "Snapshot.h"
#include "Surface.h"

namespace DataExtraction
{

DataExtractor::DataExtractor( )
{
}

std::string DataExtractor::getMapsFileName( const std::string& propertyName ) const
{
  database::Table* timeIoTbl = getProjectHandle()->getTable ( "TimeIoTbl" );
  database::Record* record = 0;
  for ( auto it : *timeIoTbl )
  {
    if ( database::getPropertyName(it) == propertyName )
    {
      record = it;
      break;
    }
  }
  if ( record == 0 )
  {
    return "";
  }

  return database::getMapFileName( record );
}

std::string DataExtractor::getPropertyFormationDataGroupName( const DataAccess::Interface::Formation* formation,
                                                              const DataAccess::Interface::Surface* surface,
                                                              const DataAccess::Interface::Property* property,
                                                              const DataAccess::Interface::Snapshot* snapshot ) const
{
  std::string propertyFormationDataGroup = "/Layer=" + property->getCauldronName() + "_" + std::to_string( static_cast<int>( snapshot->getTime() ) ) + "_";
  if ( surface && property->getPropertyAttribute() != DataModel::FORMATION_2D_PROPERTY )
  {
    propertyFormationDataGroup += surface->getMangledName();
  }
  propertyFormationDataGroup += "_";

  if ( formation && property->getPropertyAttribute() != DataModel::SURFACE_2D_PROPERTY )
  {
    propertyFormationDataGroup += formation->getMangledName();
  }

  return propertyFormationDataGroup;
}

} // namespace DataExtraction
