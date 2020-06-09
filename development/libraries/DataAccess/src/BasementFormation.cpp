//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// std library
#include <assert.h>
#include <iostream>
#include <sstream>

// utilities library
#include "mangle.h"
#include "ConstantsNumerical.h"

// tableIo library
#include "database.h"
#include "cauldronschemafuncs.h"

// DataAccess library
#include "BasementFormation.h"
#include "SourceRock.h"
#include "LithoType.h"
#include "Surface.h"
#include "Grid.h"
#include "GridMap.h"
#include "Snapshot.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"
#include "FaultElementCalculator.h"
#include "FluidType.h"
#include "AllochthonousLithology.h"

using namespace std;
using namespace database;
using namespace DataAccess;
using namespace Interface;

const std::string BasementFormation::HomogeneousMixtureString = "Homogeneous";

BasementFormation::BasementFormation (ProjectHandle& projectHandle, database::Record* record, const std::string& formationName, const std::string& lithologyName ) :
   Formation (projectHandle, record), m_formationName ( formationName ), m_lithologyName ( lithologyName )
{
   m_mangledName = utilities::mangle ( m_formationName );
}


BasementFormation::~BasementFormation (void)
{
}

const string & BasementFormation::getName (void) const
{
   return m_formationName;
}

const GridMap * BasementFormation::getInputThicknessMap (void) const
{
   const GridMap * gridMap;

   if ((gridMap = (const GridMap *) getChild (ThicknessMap)) != nullptr) return gridMap;
   else if ((gridMap = computeThicknessMap ()) != nullptr) return gridMap;
   else return nullptr;
}

GridMap * BasementFormation::loadThicknessMap(void) const
{
   return nullptr;
}

GridMap * BasementFormation::computeThicknessMap(void) const
{
   return nullptr;
}

const LithoType * BasementFormation::getLithoType1 (void) const {

   if (!m_lithoType1)
      m_lithoType1 = (LithoType const *) getProjectHandle().findLithoType ( m_lithologyName );
   return m_lithoType1;

}

const GridMap * BasementFormation::getLithoType1PercentageMap(void) const
{
   GridMap * gridMap;

   if ((gridMap = (GridMap *)getChild(LithoType1Map)) == nullptr)
   {
      const double percentage = 100.0;

      const Grid * grid = getProjectHandle().getActivityOutputGrid();
      if (!grid) grid = (Grid *)getProjectHandle().getInputGrid();
      gridMap = getProjectHandle().getFactory()->produceGridMap(this, LithoType1Map, grid, percentage);

      assert(gridMap == getChild(LithoType1Map));
   }

   return gridMap;
}

/// Return the second lithotype of this BasementFormation
const LithoType * BasementFormation::getLithoType2(void) const
{
   return nullptr;
}

/// Return the percentage map of the second lithotype
const GridMap * BasementFormation::getLithoType2PercentageMap(void) const
{
   return nullptr;
}

/// Return the third lithotype of this BasementFormation
const LithoType * BasementFormation::getLithoType3(void) const
{
   return nullptr;
}

/// Return the percentage map of the third lithotype
const GridMap * BasementFormation::getLithoType3PercentageMap(void) const
{
   return nullptr;
}

/// return the list of reservoirs in this formation.
ReservoirList * BasementFormation::getReservoirs (void) const
{
   return nullptr;
}

/// return the list of MobileLayers in this formation.
MobileLayerList * BasementFormation::getMobileLayers (void) const
{
   return nullptr;
}

// Return the allochthonous lithology pointer.
const AllochthonousLithology * BasementFormation::getAllochthonousLithology (void) const {
  return nullptr;
}

/// return the list of reservoirs in this formation.
FaultCollectionList * BasementFormation::getFaultCollections (void) const
{
   return nullptr;
}

/// tell whether this BasementFormation is a mobile layer
bool BasementFormation::isMobileLayer (void) const
{
   return false;
}

/// tell whether this BasementFormation has allochthonous lithology defined.
bool BasementFormation::hasAllochthonousLithology (void) const
{
   return false;
}

/// tell whether this BasementFormation is a source rock
bool BasementFormation::isSourceRock (void) const
{
   return false;
}

bool BasementFormation::hasConstrainedOverpressure () const {
   return false;
}

bool BasementFormation::hasChemicalCompaction () const {
   return false;
}

/// Return the SourceRock of this BasementFormation
const SourceRock * BasementFormation::getSourceRock (void) const
{
   return nullptr;
}

GridMap * BasementFormation::computeFaultGridMap (const Grid * , const Snapshot * ) const
{
   return nullptr;
}

CBMGenerics::capillarySealStrength::MixModel BasementFormation::getMixModel() const
{
   return CBMGenerics::capillarySealStrength::Homogeneous;
}

const std::string& BasementFormation::getMixModelStr (void) const {
   return HomogeneousMixtureString;
}

const FluidType* BasementFormation::getFluidType (void) const
{
   return nullptr;
}

unsigned int BasementFormation::getElementRefinement () const {
   return DefaultBasementElementRefinement;
}

float BasementFormation::getLayeringIndex(void) const {
   return Utilities::Numerical::IbsNoDataValue;
}

int BasementFormation::getDepositionSequence () const {
   return int ( DefaultUndefinedScalarValue );
}
