#include <assert.h>
#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <sstream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include<strstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <sstream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include "mangle.h"

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/Formation.h"

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

static double minus (double a, double b)
{
   return a - b;
}

Formation::Formation (ProjectHandle * projectHandle, Record * record) : 
   DAObject (projectHandle, record), m_top (0), m_bottom (0),
   m_fluidType(0)
{

   // It is up to the the derived classes to initialise correctly the member components of this class.
   const bool recordFromStratIOTbl = ( m_record != 0 and m_record->getTable ()->name () == "StratIoTbl" );

   m_lithoType1 = 0;
   m_lithoType2 = 0;
   m_lithoType3 = 0;

   m_allochthonousLithology = 0;

   m_sourceRock1 = 0;
   m_sourceRock2 = 0;

   // Moved to project handle::load-formations.
#if 0
   if ( recordFromStratIOTbl and isSourceRock ())
   {
      // m_sourceRock = (SourceRock *) m_projectHandle->findSourceRock (getName ());
      m_sourceRock1 = (SourceRock *) m_projectHandle->findSourceRock (getSourceRockType1Name ());

      if( getEnableSourceRockMixing() ) {
         m_sourceRock2 = (SourceRock *) m_projectHandle->findSourceRock (getSourceRockType2Name ());
      } else {
         m_sourceRock2 = 0; 
      }
   }
   else
   {
      m_sourceRock1 = 0;
      m_sourceRock2 = 0;
   }

#endif


#if 0
   cerr << "Added Formation " << ( recordFromStratIOTbl ? getName () : "Basement formation" ) << endl;
#endif

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

const string & Formation::getMangledName (void) const
{
   return m_mangledName;
}

const string & Formation::getTopSurfaceName (void) const
{
   return m_top->getName ();
}

const string & Formation::getBottomSurfaceName (void) const
{
   return m_bottom->getName ();
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

   if ((gridMap = (const GridMap *) getChild (ThicknessMap)) != 0) return gridMap;
   else if ((gridMap = loadThicknessMap ()) != 0) return gridMap;
   else if ((gridMap = computeThicknessMap ()) != 0) return gridMap;
   else return 0;
}

const GridMap * Formation::getMixingHIMap (void) const
{
   const GridMap * gridMap;

   if ((gridMap = (const GridMap *) getChild (MixingHIMap)) != 0) return gridMap;
   else if ((gridMap = loadMixingHIMap ()) != 0) return gridMap;
   else return 0;
}

GridMap * Formation::loadMixingHIMap (void) const
{
   double value;
   GridMap * gridMap = 0;

   const string &mixingHIGridMapId = getSourceRockMixingHIGrid (m_record);
   if (mixingHIGridMapId.length () != 0)
   {
      gridMap = m_projectHandle->loadInputMap ("StratIoTbl", mixingHIGridMapId);
   }
   else if ((value = getSourceRockMixingHI()) != RecordValueUndefined)
   {
      //const Grid * grid = m_projectHandle->getInputGrid ();
      const Grid * grid = m_projectHandle->getActivityOutputGrid();
      if (!grid) grid = (Grid *) m_projectHandle->getInputGrid ();
      gridMap = m_projectHandle->getFactory ()->produceGridMap (this, MixingHIMap, grid, value);
      assert (gridMap == getChild (MixingHIMap));
   }
   return gridMap;
}

GridMap * Formation::loadThicknessMap (void) const
{
   double thickness;
   GridMap * gridMap = 0;

   if ((thickness = getThickness (m_record)) != RecordValueUndefined)
   {
      //const Grid * grid = m_projectHandle->getInputGrid ();
      const Grid * grid = m_projectHandle->getActivityOutputGrid();
      if (!grid) grid = (Grid *) m_projectHandle->getInputGrid ();
      gridMap = m_projectHandle->getFactory ()->produceGridMap (this, ThicknessMap, grid, thickness);
      assert (gridMap == getChild (ThicknessMap));
   }
   else
   {
      const string &thicknessGridMapId = getThicknessGrid (m_record);
      if (thicknessGridMapId.length () != 0)
      {
	 gridMap = m_projectHandle->loadInputMap ("StratIoTbl", thicknessGridMapId);
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

   if (!topDepthMap) return 0;
   if (!bottomDepthMap) return 0;

   GridMap * thicknessMap = m_projectHandle->getFactory ()->produceGridMap (this, ThicknessMap, bottomDepthMap, topDepthMap, ::minus);

   return thicknessMap;
}


/// Return the first lithotype of this Formation
const LithoType * Formation::getLithoType1 (void) const
{
   if (!m_lithoType1)
      m_lithoType1 = (LithoType const *) m_projectHandle->findLithoType (getLithotype1 (m_record));
   return m_lithoType1;
}

/// Return the percentage map of the first lithotype
const GridMap * Formation::getLithoType1PercentageMap (void) const
{
   double percentage;

   if (!getLithoType1 ())
      return 0;

   GridMap * gridMap;
   if ((gridMap = (GridMap *) getChild (LithoType1Map)) == 0)
   {
      const string & lithoTypeGridMapId = getPercent1Grid (m_record);

      if (lithoTypeGridMapId.length () != 0)
      {
	 gridMap = m_projectHandle->loadInputMap ("StratIoTbl", lithoTypeGridMapId);
      }
      else if ((percentage = getPercent1 (m_record)) != RecordValueUndefined)
      {
         //const Grid *grid = m_projectHandle->getInputGrid ();
	 const Grid * grid = m_projectHandle->getActivityOutputGrid();
	 if (!grid) grid = (Grid *) m_projectHandle->getInputGrid ();
         gridMap = m_projectHandle->getFactory ()->produceGridMap (this, LithoType1Map, grid, percentage);

         assert (gridMap == getChild (LithoType1Map));
      }
   }
   return gridMap;
}

/// Return the second lithotype of this Formation
const LithoType * Formation::getLithoType2 (void) const
{
   if (!m_lithoType2)
      m_lithoType2 = (LithoType const *) m_projectHandle->findLithoType (getLithotype2 (m_record));
   return m_lithoType2;
}

/// Return the percentage map of the second lithotype
const GridMap * Formation::getLithoType2PercentageMap (void) const
{
   double percentage;

   if (!getLithoType2 ())
      return 0;

   GridMap * gridMap;
   if ((gridMap = (GridMap *) getChild (LithoType2Map)) == 0)
   {
      if (getLithoType3 () != 0)
      {
	 const string & lithoTypeGridMapId = getPercent2Grid (m_record);

	 if (lithoTypeGridMapId.length () != 0)
	 {
	    gridMap = m_projectHandle->loadInputMap ("StratIoTbl", lithoTypeGridMapId);
	 }
	 else if ((percentage = getPercent2 (m_record)) != RecordValueUndefined)
         {
            //const Grid *grid = m_projectHandle->getInputGrid ();
	    const Grid * grid = m_projectHandle->getActivityOutputGrid();
	    if (!grid) grid = (Grid *) m_projectHandle->getInputGrid ();
            gridMap = m_projectHandle->getFactory ()->produceGridMap (this, LithoType2Map, grid, percentage);

            assert (gridMap == getChild (LithoType2Map));
         }
      }
      else
      {
         //const Grid *grid = m_projectHandle->getInputGrid ();
	 const Grid * grid = m_projectHandle->getActivityOutputGrid();
	 if (!grid) grid = (Grid *) m_projectHandle->getInputGrid ();
         GridMap *onehundred = m_projectHandle->getFactory ()->produceGridMap (0, 0, grid, 100);
         gridMap =
               (GridMap *) m_projectHandle->getFactory ()->produceGridMap (this, LithoType2Map, onehundred, (GridMap *) getLithoType1PercentageMap (),::minus);
         delete onehundred;
      }
   }
   return gridMap;
}

/// Return the third lithotype of this Formation
const LithoType * Formation::getLithoType3 (void) const
{
   if (!m_lithoType3)
      m_lithoType3 = (LithoType const *) m_projectHandle->findLithoType (getLithotype3 (m_record));
   return m_lithoType3;
}

/// Return the percentage map of the third lithotype
const GridMap * Formation::getLithoType3PercentageMap (void) const
{
   if (!getLithoType3 ())
      return 0;

   GridMap * gridMap;
   if ((gridMap = (GridMap *) getChild (LithoType3Map)) == 0)
   {
         //const Grid *grid = m_projectHandle->getInputGrid ();
	 const Grid * grid = m_projectHandle->getActivityOutputGrid();
	 if (!grid) grid = (Grid *) m_projectHandle->getInputGrid ();
         GridMap *onehundred = (GridMap *) m_projectHandle->getFactory ()->produceGridMap (0, 0, grid, 100);
         GridMap * result = (GridMap *) m_projectHandle->getFactory ()->produceGridMap (0, 0, onehundred, (GridMap *) getLithoType1PercentageMap (), ::minus);
         delete onehundred;
         gridMap = m_projectHandle->getFactory ()->produceGridMap (this, LithoType3Map, result, (GridMap *) getLithoType2PercentageMap (), ::minus);
         delete result;
   }
   return gridMap;
}

/// return the list of reservoirs in this formation.
ReservoirList * Formation::getReservoirs (void) const
{
   return m_projectHandle->getReservoirs (this);
}

/// return the list of MobileLayers in this formation.
MobileLayerList * Formation::getMobileLayers (void) const
{
   return m_projectHandle->getMobileLayers (this);
}

// Return the allochthonous lithology pointer.
const AllochthonousLithology * Formation::getAllochthonousLithology (void) const {

   if ( m_allochthonousLithology == 0 and hasAllochthonousLithology ())
   {

      const bool recordFromStratIOTbl = ( m_record != 0 and m_record->getTable ()->name () == "StratIoTbl" );

      if ( recordFromStratIOTbl ) {
         m_allochthonousLithology = (const AllochthonousLithology *) m_projectHandle->findAllochthonousLithology ( getName ());
      }

   }

   return m_allochthonousLithology;
}

/// return the list of reservoirs in this formation.
FaultCollectionList * Formation::getFaultCollections (void) const
{
   return m_projectHandle->getFaultCollections (this);
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

bool Formation::hasConstrainedOverpressure (void) const
{
   return database::getHydroSand (m_record) == 1;
}

ConstrainedOverpressureIntervalList* Formation::getConstrainedOverpressureIntervalList () const {
   return m_projectHandle->getConstrainedOverpressureIntervalList ( this );
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
   GridMap * faultMap = m_projectHandle->getFactory ()->produceGridMap (0, 0, localGrid, NoFault, 1);
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

const FluidType* Formation::getFluidType () const
{
   if ( m_fluidType == 0 ) {
      m_fluidType = getProjectHandle ()->findFluid ( database::getFluidtype ( m_record ));
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
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

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
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}

bool FormationLessThan::operator ()( const Formation* f1,
                                     const Formation* f2 ) const {
   // Use the depo-sequence number here, since the surfaces may not be assigned at the point
   // at which we would like to do the sort. We should sort into ascending age, which means 
   // descending depo-sequence number.
   return f1->getDepositionSequence () > f2->getDepositionSequence ();
}
