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

#include "Formation.h"

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
#include "Interface.h"

using namespace DataAccess;
using namespace Interface;

static double minus (double a, double b)
{
   return a - b;
}

Formation::Formation (ProjectHandle& projectHandle, Record * record) :
   DAObject (projectHandle, record), m_top (nullptr), m_bottom (nullptr),
   m_fluidType(nullptr)
{

   // It is up to the the derived classes to initialise correctly the member components of this class.
   const bool recordFromStratIOTbl = ( m_record != nullptr and m_record->getTable ()->name () == "StratIoTbl" );

   m_lithoType1 = nullptr;
   m_lithoType2 = nullptr;
   m_lithoType3 = nullptr;

   m_allochthonousLithology = nullptr;

   m_sourceRock1 = nullptr;
   m_sourceRock2 = nullptr;
   m_igneousIntrusion = nullptr;

   m_igneousIntrusion = 0;

   if ( recordFromStratIOTbl ) {
      m_mangledName = utilities::mangle (getName ());
   }

   if ( recordFromStratIOTbl ) {
      m_kind = SEDIMENT_FORMATION;
   } else {
      // If the record is not from the strat-io-table then it is assumed that
      // the formation is a part of the basement.
      m_kind = BASEMENT_FORMATION;
   }

}


Formation::~Formation (void)
{
//    delete m_fluidType;
}

FormationKind Formation::kind () const {
   return m_kind;
}

const string & Formation::getName (void) const
{
   return database::getLayerName (m_record);
}

const string & Formation::getSourceRockType1Name (void) const
{
   return database::getSourceRockType1 (m_record);
}

void Formation::setSourceRock1 ( const SourceRock * sr1 ) {
   m_sourceRock1 = sr1;
}

void Formation::setSourceRock2 ( const SourceRock * sr2 ) {
   m_sourceRock2 = sr2;
}


const string & Formation::getSourceRockType2Name (void) const
{
   return database::getSourceRockType2 (m_record);
}

double Formation::getSourceRockMixingHI (void) const
{
   return database::getSourceRockMixingHI (m_record);
}

double Formation::getSourceRockMixingHC (void) const
{
   return database::getSourceRockMixingHC (m_record);
}

unsigned int Formation::getElementRefinement () const {

   int refinement = database::getElementRefinementZ ( m_record );

   if ( refinement == DefaultUndefinedScalarValue ) {
      refinement = 1;
   }

   return static_cast<unsigned int>(refinement);
}

bool Formation::getIncludeInDarcy () const {
   return database::getIncludeInDarcy (m_record) == 1;
}

bool Formation::getEnableSourceRockMixing(void) const
{
   return ( database::getEnableSourceRockMixing (m_record) == 1 );
}

const string &  Formation::getSourceRockMixingHIGridName(void) const
{
   return getSourceRockMixingHIGrid (m_record);
}

const string &  Formation::getSourceRockMixingHCGridName(void) const
{
   return getSourceRockMixingHCGrid (m_record);
}

const string & Formation::getMangledName (void) const
{
   return m_mangledName;
}

const string & Formation::getTopSurfaceName (void) const
{

   if ( m_top != nullptr) {
      return m_top->getName ();
   } else {
      return NullString;
   }

}

const string & Formation::getBottomSurfaceName (void) const
{

   if ( m_bottom != nullptr) {
      return m_bottom->getName ();
   } else {
      return NullString;
   }

}

void Formation::setTopSurface (Surface * surface)
{
   m_top = surface;
}

void Formation::setBottomSurface (Surface * surface)
{
   m_bottom = surface;
}

const Surface * Formation::getTopSurface (void) const
{
   return m_top;
}

const Surface * Formation::getBottomSurface (void) const
{
   return m_bottom;
}

const GridMap * Formation::getInputThicknessMap (void) const
{
   const GridMap * gridMap;

   if ((gridMap = (const GridMap *) getChild (ThicknessMap)) != nullptr) return gridMap;
   else if ((gridMap = loadThicknessMap ()) != nullptr) return gridMap;
   else if ((gridMap = computeThicknessMap ()) != nullptr) return gridMap;
   else return nullptr;
}

const GridMap * Formation::getMixingHIMap (void) const
{
   const GridMap * gridMap;

   if ((gridMap = (const GridMap *) getChild (MixingHIMap)) != nullptr) return gridMap;
   else if ((gridMap = loadMixingHIMap ()) != nullptr) return gridMap;
   else return nullptr;
}

GridMap * Formation::loadMixingHIMap (void) const
{
   double value;
   GridMap * gridMap = nullptr;

   const string &mixingHIGridMapId = getSourceRockMixingHIGrid (m_record);
   if (mixingHIGridMapId.length () != 0)
   {
      gridMap = getProjectHandle().loadInputMap ("StratIoTbl", mixingHIGridMapId);
   }
   else if ((value = getSourceRockMixingHI()) != RecordValueUndefined)
   {
      const Grid * grid = getProjectHandle().getActivityOutputGrid();
      if (!grid) grid = (Grid *) getProjectHandle().getInputGrid ();
      gridMap = getProjectHandle().getFactory ()->produceGridMap (this, MixingHIMap, grid, value);
      assert (gridMap == getChild (MixingHIMap));
   }
   return gridMap;
}

const GridMap * Formation::getMixingHCMap (void) const
{
   const GridMap * gridMap;

   if ((gridMap = (const GridMap *) getChild (MixingHCMap)) != nullptr) return gridMap;
   else if ((gridMap = loadMixingHCMap ()) != nullptr) return gridMap;
   else return nullptr;
}

GridMap * Formation::loadMixingHCMap (void) const
{
   double value;
   GridMap * gridMap = nullptr;

   const string &mixingHCGridMapId = getSourceRockMixingHCGrid (m_record);
   if (mixingHCGridMapId.length () != 0)
   {
      gridMap = getProjectHandle().loadInputMap ("StratIoTbl", mixingHCGridMapId);
   }
   else if ((value = getSourceRockMixingHC()) != RecordValueUndefined)
   {
      const Grid * grid = getProjectHandle().getActivityOutputGrid();
      if (!grid) grid = (Grid *) getProjectHandle().getInputGrid ();
      gridMap = getProjectHandle().getFactory ()->produceGridMap (this, MixingHCMap, grid, value);
      assert (gridMap == getChild (MixingHCMap));
   }
   return gridMap;
}

GridMap * Formation::loadThicknessMap (void) const
{
   double thickness;
   GridMap * gridMap = nullptr;

   if ((thickness = getThickness (m_record)) != RecordValueUndefined)
   {
      const Grid * grid = getProjectHandle().getActivityOutputGrid();
      if (!grid) grid = (Grid *) getProjectHandle().getInputGrid ();
      gridMap = getProjectHandle().getFactory ()->produceGridMap (this, ThicknessMap, grid, thickness);
      assert (gridMap == getChild (ThicknessMap));
   }
   else
   {
      const string &thicknessGridMapId = getThicknessGrid (m_record);
      if (thicknessGridMapId.length() != 0)
      {
          gridMap = getProjectHandle().loadInputMap("StratIoTbl", thicknessGridMapId);
          // If a thickness depth map referenced in the project file cannot be found throw runtime ERROR
          if (gridMap == nullptr)
             throw std::runtime_error( "Basin_Error: Could not open input thickness map for Layer " + getName() + "\n" );
      }
   }
   return gridMap;
}

GridMap * Formation::computeThicknessMap (void) const
{
   const Surface * topSurface = (Surface *) getTopSurface ();
   const Surface * bottomSurface = (Surface *) getBottomSurface ();

   const GridMap * topDepthMap = (GridMap *) topSurface->getInputDepthMap ();
   const GridMap * bottomDepthMap = (GridMap *) bottomSurface->getInputDepthMap ();

   if (!topDepthMap) return nullptr;
   if (!bottomDepthMap) return nullptr;

   GridMap * thicknessMap = getProjectHandle().getFactory ()->produceGridMap (this, ThicknessMap, bottomDepthMap, topDepthMap, ::minus);

   return thicknessMap;
}


/// Return the first lithotype of this Formation
const LithoType * Formation::getLithoType1 (void) const
{
   if (!m_lithoType1)
      m_lithoType1 = (LithoType const *) getProjectHandle().findLithoType (getLithotype1 (m_record));
   return m_lithoType1;
}

/// Return the percentage map of the first lithotype
const GridMap * Formation::getLithoType1PercentageMap (void) const
{
   double percentage;

   if (!getLithoType1 ())
      return nullptr;

   GridMap * gridMap = (GridMap *)getChild(LithoType1Map);
   if (gridMap == nullptr)
   {
     const string & lithoTypeGridMapId = getPercent1Grid(m_record);

		 if (lithoTypeGridMapId.length() != 0)
		 {
			 gridMap = getProjectHandle().loadInputMap("StratIoTbl", lithoTypeGridMapId);
		 }
		 else if ((percentage = getPercent1(m_record)) != RecordValueUndefined)
		 {
			 const Grid * grid = getProjectHandle().getActivityOutputGrid();
			 if (!grid) grid = (Grid *)getProjectHandle().getInputGrid();
			 gridMap = getProjectHandle().getFactory()->produceGridMap(this, LithoType1Map, grid, percentage);

			 assert(gridMap == getChild(LithoType1Map));
		 }
	 }
	 return gridMap;
}

/// Return the second lithotype of this Formation
const LithoType * Formation::getLithoType2 (void) const
{
   if (!m_lithoType2)
      m_lithoType2 = (LithoType const *) getProjectHandle().findLithoType (getLithotype2 (m_record));
   return m_lithoType2;
}

/// Return the percentage map of the second lithotype
const GridMap * Formation::getLithoType2PercentageMap (void) const
{
   double percentage;

   if (!getLithoType2 ())
      return nullptr;

   GridMap* gridMap = (GridMap *)getChild(LithoType2Map);

   if (gridMap == nullptr)
   {
       if (getLithoType3() != nullptr)
       {
           const string & lithoTypeGridMapId = getPercent2Grid(m_record);

           if (lithoTypeGridMapId.length() != 0)
           {
               gridMap = getProjectHandle().loadInputMap("StratIoTbl", lithoTypeGridMapId);
           }
           else if ((percentage = getPercent2(m_record)) != RecordValueUndefined)
           {
               const Grid * grid = getProjectHandle().getActivityOutputGrid();
               if (!grid) grid = (Grid *)getProjectHandle().getInputGrid();
               gridMap = getProjectHandle().getFactory()->produceGridMap(this, LithoType2Map, grid, percentage);

               assert(gridMap == getChild(LithoType2Map));
           }
       }
       else
       {
           const Grid * grid = getProjectHandle().getActivityOutputGrid();
           if (!grid) grid = (Grid *)getProjectHandle().getInputGrid();
           GridMap *onehundred = getProjectHandle().getFactory()->produceGridMap(nullptr, 0, grid, 100);
           gridMap =
               (GridMap *)getProjectHandle().getFactory()->produceGridMap(this, LithoType2Map, onehundred, (GridMap *)getLithoType1PercentageMap(), ::minus);
           delete onehundred;
       }
   }
   return gridMap;
}

/// Return the third lithotype of this Formation
const LithoType * Formation::getLithoType3 (void) const
{
   if (!m_lithoType3)
      m_lithoType3 = (LithoType const *) getProjectHandle().findLithoType (getLithotype3 (m_record));
   return m_lithoType3;
}

/// Return the percentage map of the third lithotype
const GridMap * Formation::getLithoType3PercentageMap (void) const
{
   if (!getLithoType3 ())
      return nullptr;

   GridMap * gridMap = (GridMap *)getChild(LithoType3Map);

   if (gridMap == nullptr)
   {
       const Grid * grid = getProjectHandle().getActivityOutputGrid();
       if (!grid) grid = (Grid *)getProjectHandle().getInputGrid();
       GridMap *onehundred = (GridMap *)getProjectHandle().getFactory()->produceGridMap(0, 0, grid, 100);
       GridMap * result = (GridMap *)getProjectHandle().getFactory()->produceGridMap(0, 0, onehundred, (GridMap *)getLithoType1PercentageMap(), ::minus);
       delete onehundred;
       gridMap = getProjectHandle().getFactory()->produceGridMap(this, LithoType3Map, result, (GridMap *)getLithoType2PercentageMap(), ::minus);
       delete result;
   }

   return gridMap;
}

/// return the list of reservoirs in this formation.
ReservoirList * Formation::getReservoirs (void) const
{
   return getProjectHandle().getReservoirs (this);
}

/// return the list of MobileLayers in this formation.
MobileLayerList * Formation::getMobileLayers (void) const
{
   return getProjectHandle().getMobileLayers (this);
}

// Return the allochthonous lithology pointer.
const AllochthonousLithology * Formation::getAllochthonousLithology (void) const {

   if ( m_allochthonousLithology == 0 and hasAllochthonousLithology ())
   {

      const bool recordFromStratIOTbl = ( m_record != 0 and m_record->getTable ()->name () == "StratIoTbl" );

      if ( recordFromStratIOTbl )
      {
         m_allochthonousLithology = (const AllochthonousLithology *) getProjectHandle().findAllochthonousLithology ( getName ());
      }
   }

   return m_allochthonousLithology;
}

/// return the list of fault collections in this formation.
FaultCollectionList * Formation::getFaultCollections (void) const
{
   return getProjectHandle().getFaultCollections (this);
}

/// tell whether this Formation is a mobile layer
bool Formation::isMobileLayer (void) const
{
   return database::getMobileLayer (m_record) == 1;
}

/// tell whether this Formation has allochthonous lithology defined.
bool Formation::hasAllochthonousLithology (void) const
{
   return database::getHasAllochthonLitho (m_record) == 1;
}

/// tell whether this Formation is a source rock
bool Formation::isSourceRock (void) const
{
   return database::getSourceRock (m_record) == 1;
}

/// Return the SourceRock1 of this Formation
const SourceRock * Formation::getSourceRock1 (void) const
{
   return m_sourceRock1;
}

/// Return the SourceRock2 of this Formation
const SourceRock * Formation::getSourceRock2 (void) const
{
	 return m_sourceRock2;
}

bool Formation::getIsIgneousIntrusion () const {
	if( m_kind == SEDIMENT_FORMATION ){
		return (database::getIsIgneousIntrusion ( m_record ) == 1);
	}
	else return false;
}

double Formation::getIgneousIntrusionAge () const {
	 return database::getIgneousIntrusionAge ( m_record );
}

void Formation::setIgneousIntrusionEvent ( IgneousIntrusionEvent* igneousIntrusion ) {
	 m_igneousIntrusion = igneousIntrusion;
}

const IgneousIntrusionEvent* Formation::getIgneousIntrusionEvent () const {
	 return m_igneousIntrusion;
}


bool Formation::hasConstrainedOverpressure (void) const
{
   return database::getHydroSand (m_record) == 1;
}

ConstrainedOverpressureIntervalList* Formation::getConstrainedOverpressureIntervalList () const {
   return getProjectHandle().getConstrainedOverpressureIntervalList ( this );
}

bool Formation::hasChemicalCompaction (void) const
{
   return database::getChemicalCompaction (m_record) == 1;
}


GridMap * Formation::computeFaultGridMap (const Grid * localGrid, const Snapshot * snapshot) const
{
#if 0
   string gridString;
   localGrid->asString (gridString);
   cerr << "Producing FaultGridMap for Formation: " << getName ()
      << " at snapshot " << snapshot->getTime ()
      << " and grid " << gridString << endl;
#endif
   GridMap * faultMap = getProjectHandle().getFactory ()->produceGridMap (0, 0, localGrid, NoFault, 1);
   faultMap->retrieveData ();

   FaultCollectionList * fcList = getFaultCollections ();

   FaultElementCalculator fec;
   fec.computeFaultGridMap (faultMap, fcList, snapshot);

   faultMap->restoreData ();

   delete fcList;

   return faultMap;
}

CBMGenerics::capillarySealStrength::MixModel Formation::getMixModel() const
{
   return database::getMixModel(m_record) == "Layered" ?
      CBMGenerics::capillarySealStrength::Layered :
      CBMGenerics::capillarySealStrength::Homogeneous;
}

const std::string& Formation::getMixModelStr (void) const {
   return database::getMixModel(m_record);
}

float Formation::getLayeringIndex(void) const {
   return static_cast<float>( database::getLayeringIndex(m_record) );
}

const FluidType* Formation::getFluidType () const
{
   if ( m_fluidType == 0 ) {
      m_fluidType = getProjectHandle().findFluid ( database::getFluidtype ( m_record ));
   }

   assert(m_fluidType);
   return m_fluidType;
}

int Formation::getDepositionSequence () const {
   return database::getDepoSequence ( m_record );
}


void Formation::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void Formation::asString (string & str) const
{

   std::ostringstream buf;

   buf << "Formation";
   if (isSourceRock ())
   {
      buf << " (SourceRock)";
   }
   if (isMobileLayer ())
   {
      buf << " (MobileLayer)";
   }
   if ( hasAllochthonousLithology ())
   {
     buf << " (allochthonous-lithology)";
   }
   buf << ": name = " << getName ();
   buf << ", top surface name = " << getTopSurface ()->getName ();
   buf << ", bottom surface name = " << getBottomSurface ()->getName ();
   buf << endl;
   if (isSourceRock ())
   {
      string srcRockString;
      m_sourceRock1->asString (srcRockString);
      buf << srcRockString;
   }

   str = buf.str ();
}

bool FormationLessThan::operator ()( const Formation* f1,
                                     const Formation* f2 ) const {
   // Use the depo-sequence number here, since the surfaces may not be assigned at the point
   // at which we would like to do the sort. We should sort into ascending age, which means
   // descending depo-sequence number.
   return f1->getDepositionSequence () > f2->getDepositionSequence ();
}
