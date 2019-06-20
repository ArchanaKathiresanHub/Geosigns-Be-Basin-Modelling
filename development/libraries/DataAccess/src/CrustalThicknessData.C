//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "CrustalThicknessData.h"

// TableIO library
#include "cauldronschemafuncs.h"
#include "database.h"

// utilities library
#include "LogHandler.h"

// DataAccess library
#include "Formation.h"
#include "Snapshot.h"
#include "Surface.h"

// DataAccess library
#include "GridMap.h"
#include "ProjectHandle.h"

using namespace database;
using namespace DataAccess;
using namespace Interface;

const std::vector<std::string> CrustalThicknessData::s_MapAttributeNames =
{
   "HCuIni", "HLMuIni"
};

CrustalThicknessData::CrustalThicknessData (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
}


CrustalThicknessData::~CrustalThicknessData ()
{
}

int CrustalThicknessData::getFilterHalfWidth() const
{
   return database::getFilterHalfWidth (m_record);
}

double CrustalThicknessData::getUpperLowerContinentalCrustRatio() const
{
   return database::getUpperLowerContinentalCrustRatio( m_record );
}

double CrustalThicknessData::getUpperLowerOceanicCrustRatio() const
{
   return database::getUpperLowerOceanicCrustRatio( m_record );
}

const std::string& CrustalThicknessData::getSurfaceName() const
{
   return database::getSurfaceName( m_record );
}

GridMap const * CrustalThicknessData::getMap( const DataAccess::Interface::CTCMapAttributeId attributeId ) const{
   return DAObject::getMap<const DataAccess::Interface::CTCMapAttributeId>( attributeId , s_MapAttributeNames );
}

std::vector<double> CrustalThicknessData::getSnapshots() const {
   std::vector<double> snapshots;
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Loading snpashots from stratigraphy:";
   FormationList* formations = getProjectHandle()->getFormations();
   FormationList::const_iterator formationIter;

   snapshots.push_back( 0.0 ); // add present day
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #time 0.0Ma loaded";

   for (formationIter = formations->begin(); formationIter != formations->end(); ++formationIter) {
      const Interface::Formation * formation = (*formationIter);

      if (formation != nullptr) {
         const Surface * botSurface = formation->getBottomSurface();
         snapshots.push_back( botSurface->getSnapshot()->getTime() );
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #time " << botSurface->getSnapshot()->getTime() << "Ma loaded";
      }
   }
   delete formations;
   return snapshots;
}
