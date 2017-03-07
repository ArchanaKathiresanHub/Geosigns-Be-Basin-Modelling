//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <assert.h>
#include <iostream>
#include <sstream>
using namespace std;

#include "mangle.h"

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/BasementFormation.h"
#include "Interface/SourceRock.h"
#include "Interface/LithoType.h"
#include "Interface/Surface.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "Interface/ObjectFactory.h"
#include "Interface/ProjectHandle.h"
#include "Interface/FaultElementCalculator.h"
#include "Interface/FluidType.h"
#include "Interface/AllochthonousLithology.h"

using namespace DataAccess;
using namespace Interface;

const std::string BasementFormation::HomogeneousMixtureString = "Homogeneous";

BasementFormation::BasementFormation (ProjectHandle * projectHandle, database::Record* record, const std::string& formationName, const std::string& lithologyName ) :
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

   if ((gridMap = (const GridMap *) getChild (ThicknessMap)) != 0) return gridMap;
   else if ((gridMap = computeThicknessMap ()) != 0) return gridMap;
   else return 0;
}

GridMap * BasementFormation::loadThicknessMap (void) const
{
   return 0;
}

GridMap * BasementFormation::computeThicknessMap (void) const
{
   return 0;
}

const LithoType * BasementFormation::getLithoType1 (void) const {

   if (!m_lithoType1)
      m_lithoType1 = (LithoType const *) m_projectHandle->findLithoType ( m_lithologyName );
   return m_lithoType1;

}

const GridMap * BasementFormation::getLithoType1PercentageMap (void) const
{
   GridMap * gridMap;

   if ((gridMap = (GridMap *) getChild (LithoType1Map)) == 0)
   {
      const double percentage = 100.0;

      const Grid * grid = m_projectHandle->getActivityOutputGrid();
      if (!grid) grid = (Grid *) m_projectHandle->getInputGrid ();
      gridMap = m_projectHandle->getFactory ()->produceGridMap (this, LithoType1Map, grid, percentage);

      assert (gridMap == getChild (LithoType1Map));
   }

   return gridMap;
}

/// Return the second lithotype of this BasementFormation
const LithoType * BasementFormation::getLithoType2 (void) const
{
   return 0;
}

/// Return the percentage map of the second lithotype
const GridMap * BasementFormation::getLithoType2PercentageMap (void) const
{
   return 0;
}

/// Return the third lithotype of this BasementFormation
const LithoType * BasementFormation::getLithoType3 (void) const
{
   return 0;
}

/// Return the percentage map of the third lithotype
const GridMap * BasementFormation::getLithoType3PercentageMap (void) const
{
   return 0;
}

/// return the list of reservoirs in this formation.
ReservoirList * BasementFormation::getReservoirs (void) const
{
   return 0;
}

/// return the list of MobileLayers in this formation.
MobileLayerList * BasementFormation::getMobileLayers (void) const
{
   return 0;
}

// Return the allochthonous lithology pointer.
const AllochthonousLithology * BasementFormation::getAllochthonousLithology (void) const {
  return false;
}

/// return the list of reservoirs in this formation.
FaultCollectionList * BasementFormation::getFaultCollections (void) const
{
   return 0;
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
   return 0;
}

GridMap * BasementFormation::computeFaultGridMap (const Grid * localGrid, const Snapshot * snapshot) const
{
   return 0;
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
   return 0;
}

unsigned int BasementFormation::getElementRefinement () const {
   return DefaultBasementElementRefinement;
}

float BasementFormation::getLayeringIndex(void) const {
   return -9999;
}

int BasementFormation::getDepositionSequence () const {
   return int ( DefaultUndefinedScalarValue );
}
