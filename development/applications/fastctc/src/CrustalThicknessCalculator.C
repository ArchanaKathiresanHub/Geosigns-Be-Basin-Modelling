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
#include <sstream>

// petsc library
#include <petsc.h>

// TableIO library
#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;

// DataAccess library
#include "Interface/CrustalThicknessData.h"
#include "Interface/DistributedGridMap.h"
#include "Interface/Interface.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"

// Crustal Thickness library
#include "CrustalThicknessCalculator.h"
#include "LinearFunction.h"

// Parallel _Hdf5 library
#include "h5_parallel_file_types.h"
#include "h5merge.h"
//------------------------------------------------------------//

CrustalThicknessCalculator* CrustalThicknessCalculator::m_crustalThicknessCalculator = 0;
string CrustalThicknessCalculator::m_projectFileName = "";
string CrustalThicknessCalculator::m_outputFileName = "";
const string CrustalThicknessCalculatorActivityName = "CrustalThicknessCalculator";

//------------------------------------------------------------//
void displayTime( const double timeToDisplay, const char * msgToDisplay ) {

   int hours   = (int)(timeToDisplay / 3600.0);
   int minutes = (int)((timeToDisplay - (hours * 3600.0)) / 60.0);
   int seconds = (int)(timeToDisplay - hours * 3600.0 - minutes * 60.0);

   PetscPrintf( PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n" );
   PetscPrintf( PETSC_COMM_WORLD, "%s: %d hours %d minutes %d seconds\n", msgToDisplay, hours, minutes, seconds );

}

CrustalThicknessCalculator::CrustalThicknessCalculator( database::Database * database, const std::string & name, const std::string & accessMode, ObjectFactory* objectFactory )
   : GeoPhysics::ProjectHandle( database, name, accessMode, objectFactory ) {

   m_outputOptions = 0;
   m_debug = false;
   m_applySmoothing = true;
   m_smoothRadius = 0;

   PetscBool onlyPrimary = PETSC_FALSE;
   PetscOptionsHasName( PETSC_NULL, "-primaryOnly", &onlyPrimary );

   if( onlyPrimary ) {
      setPrimary( true );
   }
   PetscOptionsHasName( PETSC_NULL, "-primaryDouble", &onlyPrimary );

   if( onlyPrimary ) {
      setPrimaryDouble( true );
   }
}
 
//------------------------------------------------------------//

CrustalThicknessCalculator::~CrustalThicknessCalculator () {
}

//------------------------------------------------------------//

CrustalThicknessCalculator* CrustalThicknessCalculator::CreateFrom ( const string& inputFileName ) {


   if ( m_crustalThicknessCalculator == 0 ) {
      m_crustalThicknessCalculator = (CrustalThicknessCalculator*)Interface::OpenCauldronProject( inputFileName, "rw", m_crustalThicknessCalculator->m_factory );

   }
   m_projectFileName = inputFileName;

   return m_crustalThicknessCalculator;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::finalise ( const bool saveResults ) {

   m_crustalThicknessCalculator->setSimulationDetails ( "fastctc", "Default", "" );
   m_crustalThicknessCalculator->finishActivity ();

   if( saveResults ) {
      if( ! CrustalThicknessCalculator::getInstance ().mergeOutputFiles ()) {
         PetscPrintf ( PETSC_COMM_WORLD, "  MeSsAgE ERROR Unable to merge output files\n");
      }
   }
   if ( saveResults && m_crustalThicknessCalculator->getRank() == 0 ) {
      if( m_outputFileName.length() == 0 ) {
         m_crustalThicknessCalculator->saveToFile(m_projectFileName);
      } else {
         m_crustalThicknessCalculator->saveToFile(m_outputFileName);
      }
   }
   delete m_crustalThicknessCalculator;
   m_crustalThicknessCalculator = 0;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::setRequestedOutputProperties( InterfaceOutput & theOutput  )
{
   Interface::ModellingMode theMode = getModellingMode();
   string theModellingMode = "3d";

   if( Interface::MODE1D == theMode ) {
      theModellingMode = "1d";
   }
   
   Table * timeIoTbl = getTable ("FilterTimeIoTbl");
   Table::iterator tblIter;

   for (tblIter = timeIoTbl->begin (); tblIter != timeIoTbl->end (); ++ tblIter) {
      Record * filterTimeIoRecord = * tblIter;
      const string & outPutOption = database::getOutputOption(filterTimeIoRecord);
      const string & modellingMode = database::getModellingMode(filterTimeIoRecord);
      const string & propertyName = database::getPropertyName (filterTimeIoRecord);

      if(outPutOption != "None" && modellingMode == theModellingMode) {

         outputMaps index = CrustalThicknessInterface::getPropertyId ( propertyName );
         if( index != numberOfOutputMaps ) {
            theOutput.setMapToOutput( index ); 
         }

      }  
   }
   // set default output properties
   theOutput.setMapsToOutput( mohoMap, thicknessBasaltMap, WLSadjustedMap, RDAadjustedMap, thicknessCrustMap, ECTMap, 
                              thicknessCrustMeltOnset, topBasaltMap, incTectonicSubsidence, WLSMap, isostaticBathymetry,
                              numberOfOutputMaps );
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::deleteCTCPropertyValues()
{
   Interface::MutablePropertyValueList::iterator propertyValueIter;

   propertyValueIter = m_propertyValues.begin ();

   while (propertyValueIter != m_propertyValues.end ()) {
      Interface::PropertyValue * propertyValue = * propertyValueIter;

      if(CrustalThicknessInterface::getPropertyId (propertyValue->getProperty()->getName()) != numberOfOutputMaps ) {

         propertyValueIter = m_propertyValues.erase (propertyValueIter);

         if(propertyValue->getRecord ()) {
            propertyValue->getRecord ()->getTable ()->deleteRecord (propertyValue->getRecord ());
         }

         delete propertyValue;
      } else {
         ++ propertyValueIter;
      }

   }

}

//------------------------------------------------------------//

void CrustalThicknessCalculator::run() {

   unsigned int i, j, k;
   bool started = CrustalThicknessCalculator::getInstance().startActivity( CrustalThicknessCalculatorActivityName,
      CrustalThicknessCalculator::getInstance().getHighResolutionOutputGrid(),
      true );

   if (!started) {
      string s = "Can not start CrustalThicknessCalculator";
      throw s;
   }
   started = GeoPhysics::ProjectHandle::initialise();

   if (!started) {
      string s = "Can not start CrustalThicknessCalculator";
      throw s;
   }
   setFormationLithologies( false, true );

   LinearFunction theLF;
   DensityCalculator theDensityCalculator;
   InterfaceOutput theOutput;

   if (m_crustalThicknessData.size() != 1) {
      string s;
      if (m_crustalThicknessData.size() == 0) {
         s = "The CrustalThicknessData table in the ProjectFile is empty.";
      }
      else {
         s = "Too many records for CrustalThicknessData table in the ProjectFile.";
      }
      throw s;
   }

   InterfaceInput &theInterfaceData = dynamic_cast<InterfaceInput &>(*m_crustalThicknessData[0]);
   theInterfaceData.loadInputDataAndConfigurationFile( "InterfaceData.cfg" );

   m_validNodes.reallocate( getActivityOutputGrid() );
   m_currentValidNodes.reallocate( getActivityOutputGrid() );

   initialiseValidNodes( theInterfaceData );

   m_smoothRadius = theInterfaceData.getSmoothRadius();

   setAdditionalOptionsFromCommandLine();

   m_applySmoothing = (m_smoothRadius > 0);
   if (m_applySmoothing) {
      PetscPrintf( PETSC_COMM_WORLD, "Applying spatial smoothing with radius = %d\n", m_smoothRadius );
   }

   setRequestedOutputProperties( theOutput );

   if (m_debug) {
      theOutput.setAllMapsToOutput( true );
   }
   //   if( !theOutput.allocateOutputMaps( m_crustalThicknessCalculator ) ) {

   m_DensityCalculator.loadSnapshots( m_crustalThicknessCalculator );
   if (!m_DensityCalculator.setDensities( theInterfaceData.getBackstrippingMantleDensity(), theInterfaceData.getWaterDensity() )) {
      string s;
      s = "BackstrippingMantleDensity = WaterDensity. Check the constants in theconfiguration file.";
      throw s;
   }

   snapshotsList snapshots = m_DensityCalculator.getSnapshots();

   std::sort( snapshots.begin(), snapshots.end(), std::greater<int>() );

   GridMap *previousWLS = 0;

   GridMap *presentDayWLS = calculatePresentDayWLS( theInterfaceData );

   if (presentDayWLS == 0) {
      string s = "Cannot calculate present day WLS map.";
      throw s;
   };

   presentDayWLS->retrieveData();

   for (k = 0; k < snapshots.size(); ++k) {

      const double age = snapshots[k];

      const Interface::Property * depthProperty = m_DensityCalculator.loadDepthProperty( m_crustalThicknessCalculator, age, theInterfaceData.getBaseRiftSurfaceName() );
      try {
         m_DensityCalculator.loadDepthData( m_crustalThicknessCalculator, depthProperty, age );
      }
      catch (std::string& s) {
         PetscPrintf( PETSC_COMM_WORLD, "\n %s \n\n", s.c_str() );
         continue;
      }

      const Interface::Property * pressureProperty = m_DensityCalculator.loadPressureProperty( m_crustalThicknessCalculator, age );
      try {
         m_DensityCalculator.loadPressureData( m_crustalThicknessCalculator, pressureProperty, age );
      }
      catch (std::string& s) {
         PetscPrintf( PETSC_COMM_WORLD, "\n %s \n\n", s.c_str() );
         continue;
      }

      const Snapshot * theSnapshot = (const Snapshot *)findSnapshot( age );

      if (!theOutput.createSnapShotOutputMaps( m_crustalThicknessCalculator, theSnapshot, m_DensityCalculator.getTopOfSedimentSurface() )) {
         string s = "Cannot allocate output maps.";
         throw s;
      };

      theInterfaceData.retrieveData();
      m_DensityCalculator.retrieveData();
      theOutput.retrieveData();

      initialiseCurrentValidNodes();

      if (previousWLS != 0) {
         previousWLS->retrieveData();
      }
      ////

      unsigned firstI = theInterfaceData.firstI();
      unsigned firstJ = theInterfaceData.firstJ();
      unsigned lastI = theInterfaceData.lastI();
      unsigned lastJ = theInterfaceData.lastJ();

      double sedimentDensity, sedimentThickness, WLS, WLS_adjusted, TF, Moho, RDA_adjusted, crustalThickness, basaltThickness, ECT;
      double posI, posJ, backStrip, topBasalt, isoBathymetry, compensation;

      bool status = true;
      stringstream errorMsg;

      for (i = firstI; i <= lastI; ++i) {
         for (j = firstJ; j <= lastJ; ++j) {
            if (!theInterfaceData.defineLinearFunction( m_LF, i, j ) || !getNodeIsValid( i, j )) {
               WLS = Interface::DefaultUndefinedMapValue;
               backStrip = Interface::DefaultUndefinedMapValue;
               sedimentThickness = Interface::DefaultUndefinedMapValue;
               isoBathymetry = Interface::DefaultUndefinedMapValue;
               compensation = Interface::DefaultUndefinedMapValue;

               theOutput.setMapValue( WLSMap, i, j, WLS );
            }
            else {
               m_DensityCalculator.computeNode( i, j );

               WLS = m_DensityCalculator.getWLS();
               theOutput.setMapValue( WLSMap, i, j, WLS );

               backStrip = m_DensityCalculator.getBackstrip();
               sedimentThickness = m_DensityCalculator.getSedimentThickness();
               compensation = sedimentThickness - backStrip;

               if (presentDayWLS->getValue( i, j ) != Interface::DefaultUndefinedMapValue) {
                  // smoothed map of IB
                  isoBathymetry = presentDayWLS->getValue( i, j ) - backStrip;
               }
               else {
                  isoBathymetry = Interface::DefaultUndefinedMapValue;
               }
            }
            theOutput.setMapValue( cumSedimentBackstrip, i, j, backStrip );
            theOutput.setMapValue( cumSedimentThickness, i, j, sedimentThickness );
            theOutput.setMapValue( isostaticBathymetry, i, j, isoBathymetry );
            theOutput.setMapValue( cumBasementCompensation, i, j, compensation );
         }
      }

      if (m_applySmoothing) {

         bool status = movingAverageSmoothing( theOutput.getMap( WLSMap ) );
         if (!status) {
            string s = "Failed to smooth WLS map.";
            throw s;
         }
         status = movingAverageSmoothing( theOutput.getMap( WLSMap ) );
         if (!status) {
            string s = "Failed to smooth WLS map.";
            throw s;
         }
      }
      if (m_applySmoothing) {

         bool status = movingAverageSmoothing( theOutput.getMap( isostaticBathymetry ) );
         if (!status) {
            string s = "Failed to smooth isostaticBathymetry map.";
            throw s;
         }
         status = movingAverageSmoothing( theOutput.getMap( isostaticBathymetry ) );
         if (!status) {
            string s = "Failed to smooth isostaticBathymetry map.";
            throw s;
         }
      }

      for (i = firstI; i <= lastI; ++i) {
         for (j = firstJ; j <= lastJ; ++j) {
            if (!theInterfaceData.defineLinearFunction( m_LF, i, j ) || !getNodeIsValid( i, j )) {
               // errorMsg << "I = " << i << "; J = " << j << endl;
               // status = false;
               // break;
               theOutput.setAllMapsUndefined( i, j );

            }
            else {

               ///// Check for undefined value?  
               m_DensityCalculator.computeNode( i, j );
               sedimentDensity = m_DensityCalculator.getSedimentDensity();

               theOutput[slopePreMelt]      = m_LF.getM1();
               theOutput[slopePostMelt]     = m_LF.getM2();
               theOutput[interceptPostMelt] = m_LF.getC2();

               theOutput[sedimentDensityMap]       = sedimentDensity;
               theOutput[estimatedCrustDensityMap] = theInterfaceData.getEstimatedCrustDensity();
               theOutput[TFOnsetMap]               = theInterfaceData.getTFOnset();
               theOutput[TFOnsetLinMap]            = theInterfaceData.getTFOnsetLin();
               theOutput[TFOnsetMigMap]            = theInterfaceData.getTFOnsetMig();
               theOutput[PTaMap]                   = theInterfaceData.getPTa();
               theOutput[basaltDensityMap]         = theInterfaceData.getMagmaticDensity();
               theOutput[WLSOnsetMap]              = theInterfaceData.getWLSonset();
               theOutput[WLSCritMap]               = theInterfaceData.getWLScrit();
               theOutput[WLSExhumeMap]             = theInterfaceData.getWLSexhume();
               theOutput[WLSExhumeSerpMap]         = theInterfaceData.getWLSexhumeSerp();
               theOutput[thicknessCrustMeltOnset]  = theInterfaceData.getInitialCrustThickness() * (1 - theInterfaceData.getTFOnsetLin());

               WLS = theOutput.getMapValue( WLSMap, i, j );
               theOutput[WLSMap] = WLS;

               theOutput[incTectonicSubsidence] = WLS;

               if (previousWLS != 0) {
                  if (previousWLS->getValue( i, j ) != Interface::DefaultUndefinedMapValue) {
                     theOutput[incTectonicSubsidence] = WLS - previousWLS->getValue( i, j );
                  }
                  else {
                     theOutput[incTectonicSubsidence] = Interface::DefaultUndefinedMapValue;
                  }
               }

               theOutput[cumSedimentBackstrip]    = theOutput.getMapValue( cumSedimentBackstrip, i, j );
               theOutput[cumSedimentThickness]    = theOutput.getMapValue( cumSedimentThickness, i, j );
               theOutput[cumBasementCompensation] = theOutput.getMapValue( cumBasementCompensation, i, j );
               theOutput[isostaticBathymetry]     = theOutput.getMapValue( isostaticBathymetry, i, j );

               if (WLS != Interface::DefaultUndefinedMapValue) {

                  WLS_adjusted = WLS - theInterfaceData.getDeltaSLValue( i, j );
                  RDA_adjusted = m_LF.getWLS_crit() - WLS_adjusted;

                  TF = m_LF.getCrustTF( WLS_adjusted );
                  crustalThickness = (TF < 1 ? theInterfaceData.getInitialCrustThickness() * (1 - TF) : 0);

                  if (WLS >= theOutput[WLSExhumeMap]) basaltThickness = 0;
                  else basaltThickness = m_LF.getBasaltThickness( WLS_adjusted );

                  topBasalt = crustalThickness + m_DensityCalculator.getTopBasementDepthValue();
                  Moho = topBasalt + basaltThickness;
               }

               if (WLS == Interface::DefaultUndefinedMapValue || theInterfaceData.getWLScrit() < theInterfaceData.getWLSonset()) {
                  // if WLS_crit < Wls_onset, set all mandatory outputs to Undefined value
                  theOutput[WLSadjustedMap]          = Interface::DefaultUndefinedMapValue;
                  theOutput[RDAadjustedMap]          = Interface::DefaultUndefinedMapValue;
                  theOutput[TFMap]                   = Interface::DefaultUndefinedMapValue;
                  theOutput[topBasaltMap]            = Interface::DefaultUndefinedMapValue;
                  theOutput[thicknessCrustMap]       = Interface::DefaultUndefinedMapValue;
                  theOutput[thicknessBasaltMap]      = Interface::DefaultUndefinedMapValue;
                  theOutput[mohoMap]                 = Interface::DefaultUndefinedMapValue;
                  theOutput[ECTMap]                  = Interface::DefaultUndefinedMapValue;
                  theOutput[thicknessCrustMeltOnset] = Interface::DefaultUndefinedMapValue;
               }
               else {
                  theOutput[WLSadjustedMap]     = WLS_adjusted;
                  theOutput[RDAadjustedMap]     = RDA_adjusted;
                  theOutput[TFMap]              = TF;
                  theOutput[topBasaltMap]       = topBasalt;
                  theOutput[thicknessCrustMap]  = crustalThickness;
                  theOutput[thicknessBasaltMap] = basaltThickness;
                  theOutput[mohoMap]            = Moho;

                  if (theInterfaceData.getInitialLithosphereThickness() != 0.0 &&
                     crustalThickness != Interface::DefaultUndefinedMapValue &&
                     basaltThickness != Interface::DefaultUndefinedMapValue) {

                     ECT = crustalThickness + basaltThickness * (theInterfaceData.getInitialCrustThickness() / theInterfaceData.getInitialLithosphereThickness());
                     ECT = (ECT < theInterfaceData.getECTmin() ? theInterfaceData.getECTmin() : ECT);

                  }
                  else {
                     ECT = Interface::DefaultUndefinedMapValue;
                  }
                  theOutput[ECTMap] = ECT;
               }

               // now put all values into the correspondent maps
               theOutput.setValuesToMaps( i, j );
            }
         }
      }
      if (!status) {
         string s = "Linear function can not be defined due to wrong rift event age at " + errorMsg.str();
         throw s;
      }
      ///
      theInterfaceData.restoreData();
      m_DensityCalculator.restoreData();
      theOutput.restoreData();

      theOutput.saveOutput( m_crustalThicknessCalculator, m_debug, m_outputOptions, theSnapshot );

      if (previousWLS != 0) {
         previousWLS->restoreData();
      }
      previousWLS = theOutput.getMap( WLSMap );

      // Save properties to disk.
      m_crustalThicknessCalculator->continueActivity();
   }

   presentDayWLS->restoreData();

   // if( ! theOutput.updateIsoBathymetryMaps ( m_crustalThicknessCalculator, snapshots  )) {
   //    string s = "The present day WLS map cannot be found - the Isostatic Bathymetry map cannot be computed.";
   //    throw s;
   // };

   delete presentDayWLS;
}

//------------------------------------------------------------//

GridMap * CrustalThicknessCalculator::calculatePresentDayWLS( InterfaceInput & theInterfaceData ) {

   GridMap * WLSmap = m_crustalThicknessCalculator->getFactory ()->produceGridMap (0, 0, m_crustalThicknessCalculator->getActivityOutputGrid (),
                                                                                   DefaultUndefinedMapValue, 1);

   if( WLSmap != 0 ) {
      const Interface::Property * depthProperty = m_DensityCalculator.loadDepthProperty(  m_crustalThicknessCalculator, 0.0, theInterfaceData.getBaseRiftSurfaceName() );
      m_DensityCalculator.loadDepthData( m_crustalThicknessCalculator, depthProperty, 0.0 );
      const Interface::Property * pressureProperty = m_DensityCalculator.loadPressureProperty(  m_crustalThicknessCalculator, 0.0 );
      m_DensityCalculator.loadPressureData( m_crustalThicknessCalculator, pressureProperty, 0.0 );
     
      LinearFunction m_LF;

      WLSmap->retrieveData();
      theInterfaceData.retrieveData();
      m_DensityCalculator.retrieveData();
 
      initialiseCurrentValidNodes();
      
      unsigned firstI = WLSmap->firstI();
      unsigned firstJ = WLSmap->firstJ();
      unsigned lastI  = WLSmap->lastI();
      unsigned lastJ  = WLSmap->lastJ();
      unsigned i, j;
      double WLS;

      for (i = firstI; i <= lastI; ++i) {
         for (j = firstJ; j <= lastJ; ++j) {
            if (!theInterfaceData.defineLinearFunction( m_LF, i, j ) || !getNodeIsValid( i, j )) {
               WLS = Interface::DefaultUndefinedMapValue;
            }
            else {
               m_DensityCalculator.computeNode( i, j );

               WLS = m_DensityCalculator.getWLS();
            }
            WLSmap->setValue( i, j, WLS );

         }
      }

      bool status = true;

      if (false && m_applySmoothing) {
         status = movingAverageSmoothing( WLSmap );
         if (status) {
            status = movingAverageSmoothing( WLSmap );
         }
      }
      WLSmap->restoreData();
      theInterfaceData.restoreData();
      m_DensityCalculator.restoreData();

      if (!status) return 0;
   }

   return WLSmap;
}


//------------------------------------------------------------//
//
//
//            j, yr  ---->     ---> sum
//              _____  
//              | | | 
//              -----
//              | | | 
//      |       -----   |
//   xr | i     | | |   | col
//      V       -----   V
//              | | |
//              ----- 
//              | | |
//              -----
//
//
bool CrustalThicknessCalculator::movingAverageSmoothing( GridMap * aWLSMap) {

 
  const Interface::Grid * grid = getActivityOutputGrid ();
 
  bool status = true;

  GridMap * sumMap =  getFactory()->produceGridMap(0, 0, grid, Interface::DefaultUndefinedMapValue, 1);    

  if( sumMap == 0 ) {
     return false;
  } 

  GridMap * numberMap = getFactory()->produceGridMap(0, 0, grid, Interface::DefaultUndefinedMapValue, 1 );  
  
  if( numberMap == 0 ) {
     delete sumMap;
     return false;
  }
  const unsigned int averageRadius = m_smoothRadius;

  unsigned int lastI, lastJ, firstI, firstJ;
  unsigned int i, j, jj, ii, ii1;
  double val;
  bool undefValue;
  int  num = 0;
    
  Interface::DistributedGridMap * aMap = dynamic_cast<Interface::DistributedGridMap *> ( aWLSMap );

  const bool ghostNodes = true;

     //aMap->retrieveData(); already retreived
  if( aMap->retrieved () && ghostNodes ) {
     aMap->restoreData();
     aMap->retrieveData( ghostNodes );
  }
  sumMap->retrieveData( ghostNodes );
  numberMap->retrieveData( ghostNodes );

  firstI = aMap->firstI( ghostNodes );
  firstJ = aMap->firstJ( ghostNodes );
  lastI  = aMap->lastI( ghostNodes );
  lastJ  = aMap->lastJ( ghostNodes );

#if 0
  if(( lastI - firstI ) > 2 * averageRadius ) {
     lastI  = lastI  - averageRadius;
     firstI = firstI + averageRadius;
  }
  if(( lastJ - firstJ ) > 2 * averageRadius ) {
     lastJ  = lastJ  - averageRadius;
     firstJ = firstJ + averageRadius;
  }
#endif

  double ** columnMap = new double *[2];
  double ** numberMapCollect = new double* [2];
  columnMap[0] = new double [lastJ - firstJ + 1]; 
  columnMap[1] = new double [lastJ - firstJ + 1];
  numberMapCollect[0] = new double [lastJ - firstJ + 1];
  numberMapCollect[1] = new double [lastJ - firstJ + 1];

  unsigned int xr = lastI - firstI;
  if( xr > averageRadius ) xr = averageRadius;

  unsigned int yr = lastJ - firstJ;
  if( yr > averageRadius ) yr = averageRadius;

  const int lastYr = ( lastJ - yr > 1 ? lastJ - yr - 1 : 0 );
  const int lastXr = ( lastI - xr > 1 ? lastI - xr - 1 : 0 );


//
//  1.  Init the col sums: go along the first row and collect all column sums up to xr    
//                j  ---->     
//              _____  
//         i    |x|x|  col[0][j], numberMapCollect[0][j]
//          |   -----
//          |   |x|x| 
//          |   -----   
//          V   |x|x|   
//        xr    -----   
//              | | |
//              ----- 
//              | | |
//              -----
//
  for( j = firstJ, jj = 0; j <= lastJ; ++ j, ++ jj ) {
     num = 0;
     val = 0;
     undefValue = true;
     for( i = firstI; i <= firstI + xr; ++ i ) {
        if( aMap->getValue( i, j ) != Interface::DefaultUndefinedMapValue ) {
           val += aMap->getValue( i, j );
           ++ num;
           undefValue = false;
        }
     }
     if( undefValue ) {
        columnMap[0][jj] = Interface::DefaultUndefinedMapValue;
        numberMapCollect[0][jj] = Interface::DefaultUndefinedMapValue;
     } else {
        columnMap[0][jj] = val;
        numberMapCollect[0][jj] = num;
     }
  }

//
//  2. For each row the first step is to init the first sum: sum up column sums of first yr columns.
//            j ----- yr    
//              _____  
//              |x| | --> collect sum 
//              -----
//              | | | 
//      |       -----   
//      | i     | | |   
//      V       -----   
//              | | |
//              ----- 
//              | | |
//              -----
//
  ii  = 0;
  ii1 = 1;

  for( i = firstI; i <= lastI; ++ i ) {
     val = 0;
     undefValue = true;
     num = 0;
     for ( j = firstJ, jj = 0; j <= firstJ + yr; ++ j, ++ jj ) { 
        if( columnMap[ii][jj] != Interface::DefaultUndefinedMapValue ) {
           val += columnMap[ii][jj];
           num += numberMapCollect[ii][jj];
           undefValue = false;
        }
     }
     if( !undefValue ) {
        sumMap->setValue( i, firstJ, val );
        numberMap->setValue( i, firstJ, num );
     } else {
        sumMap->setValue( i, firstJ, Interface::DefaultUndefinedMapValue );
        numberMap->setValue( i, firstJ, Interface::DefaultUndefinedMapValue );
     }
     for ( j = firstJ, jj = 0; j <= lastJ; ++ j, ++ jj ) { 
        num = 0;
        val = 0;
        undefValue = true;
        if( j < lastJ ) {
           // moving the window from left to right, update the sums 
           if( sumMap->getValue( i, j ) != Interface::DefaultUndefinedMapValue ) {
              // collect the sum for sum[j+1] (next in the row) 
              val = sumMap->getValue( i, j );
              num = numberMap->getValue( i, j );
              undefValue = false;
           } 
           if( j <= lastYr && lastYr != 0 ) {
               if( columnMap[ii][jj + yr + 1] != Interface::DefaultUndefinedMapValue ) {
                 // update the sum: add the next column from the right
                 val += columnMap[ii][jj + yr + 1];
                 num += numberMapCollect[ii][jj + yr + 1];
                 undefValue = false;
             }
           }
           if( j >= firstJ + yr ) {
              if( columnMap[ii][jj - yr] != Interface::DefaultUndefinedMapValue ) {
                 // update the sum: substruct the column from the left
                 val -= columnMap[ii][jj - yr];
                 num -= numberMapCollect[ii][jj - yr];
                 undefValue = false;         
              }       
           }
           if( !undefValue ) {
              sumMap->setValue( i, j + 1, val );
              numberMap->setValue( i, j + 1, num );
           } else {
              sumMap->setValue( i, j + 1, Interface::DefaultUndefinedMapValue );
              numberMap->setValue( i, j + 1, Interface::DefaultUndefinedMapValue );
          }
        }
        if( i < lastI ) {
           // update the column sum in the row below
           undefValue = true;
           val = 0;
           num = 0;
           if( columnMap[ii][jj] != Interface::DefaultUndefinedMapValue ) {
              val = columnMap[ii][jj];
              num = numberMapCollect[ii][jj];
              undefValue = false; 
           } 
           if( i <= lastXr && lastXr != 0 ) {
              if( aMap->getValue( i + xr + 1, j ) != Interface::DefaultUndefinedMapValue ) {
                 val += aMap->getValue( i + xr + 1, j );
                 ++ num;
                 undefValue = false; 
              }
           }
           if( i >= firstI + xr ) {
              if( aMap->getValue( i - xr, j ) != Interface::DefaultUndefinedMapValue ) {
                 val -= aMap->getValue( i - xr, j );
                 -- num;
                 undefValue = false;
              }
           }
           if( undefValue ) {
              columnMap[ii1][jj] = Interface::DefaultUndefinedMapValue;
              numberMapCollect[ii1][jj] = Interface::DefaultUndefinedMapValue;
           } else {
              columnMap[ii1][jj] = val;
              numberMapCollect[ii1][jj] = num;
           }
        }
     }
     if ( ii == 0 ) { ii = 1; ii1 = 0; }
     else { ii = 0; ii1 = 1; }
  }
  // copy smoothed map to aMap. 

  int footPrintI = ( ghostNodes ? ( lastI - firstI > 4 ? 2 : 0 ) : 0 );
  int footPrintJ = ( ghostNodes ? ( lastJ - firstJ > 4 ? 2 : 0 ) : 0 );

  double multVal;

  for( i = firstI + footPrintI; i <= lastI - footPrintI; ++ i ) {
     
     for ( j = firstJ + footPrintJ; j <= lastJ - footPrintJ; ++ j ) { 
        val = sumMap->getValue( i, j );
        num = numberMap->getValue( i, j );

        if( val != Interface::DefaultUndefinedMapValue && aMap->getValue(i, j ) != Interface::DefaultUndefinedMapValue ) {
           if( num == Interface::DefaultUndefinedMapValue ) {
              cout << "WARNING --- Undefined num value in the smoothing algorithm!" << endl;
              num = 1;
           } else if( num == 0 ) {
              cout << "WARNING --- Zero num value in the smoothing algorithm!" << endl;
              num = 1;
           } else  if( num < 0 ) {
              cout << "WARNING --- Negative num value in the smoothing algorithm!" << endl;
              num = 1;
           } 
           multVal = 1.0 / (double)num;
           aMap->setValue( i, j, val  * multVal );
        }
     }
  }


  // needs to add average across the borders
  sumMap->restoreData( );
  numberMap->restoreData( );

  if ( ghostNodes ) {
     aMap->restoreData();
     aMap->retrieveData();
  }
  // aMap->restoreData(); will be restored in the end

  delete columnMap;
  delete sumMap;
  delete numberMap;
  delete numberMapCollect;

  return status;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::initialiseValidNodes( const InterfaceInput &theInterfaceData ) {

   m_validNodes.fill( true );

   addUndefinedAreas( m_validNodes, theInterfaceData.getT0Map() );
   addUndefinedAreas( m_validNodes, theInterfaceData.getTRMap() );
   addUndefinedAreas( m_validNodes, theInterfaceData.getHCuMap() );
   addUndefinedAreas( m_validNodes, theInterfaceData.getHLMuMap() );
   addUndefinedAreas( m_validNodes, theInterfaceData.getDeltaSLMap() );
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::initialiseCurrentValidNodes () {

   m_currentValidNodes.fill ( true );

   addUndefinedAreas( m_currentValidNodes, m_DensityCalculator.getDepthBasementMap() );
   addUndefinedAreas( m_currentValidNodes, m_DensityCalculator.getDepthWaterBottomMap() );
   addUndefinedAreas( m_currentValidNodes, m_DensityCalculator.getPressureBasementMap() );
   addUndefinedAreas( m_currentValidNodes, m_DensityCalculator.getPressureWaterBottomMap() );
}

//------------------------------------------------------------//

bool CrustalThicknessCalculator::mergeOutputFiles ( ) {

#ifdef _MSC_VER
	return true;
#else
	if( ! H5_Parallel_PropertyList::isOneFilePerProcessEnabled() ) return true;

   PetscBool noFileCopy = PETSC_FALSE;

   PetscOptionsHasName( PETSC_NULL, "-nocopy", &noFileCopy );

   PetscLogDouble merge_Start_Time;
   PetscTime( &merge_Start_Time );

   string fileName = CrustalThicknessCalculatorActivityName + "_Results.HDF";
   string filePathName = getProjectPath() + "/" + getOutputDir() + "/" + fileName;

   bool status = mergeFiles( allocateFileHandler( PETSC_COMM_WORLD, filePathName, H5_Parallel_PropertyList::getTempDirName(), (noFileCopy ? CREATE : REUSE) ) );

   if (status) {
      status = H5_Parallel_PropertyList::copyMergedFile( filePathName );
   }
   if (status) {
      PetscLogDouble merge_End_Time;
      PetscTime( &merge_End_Time );

      displayTime( merge_End_Time - merge_Start_Time, "Merging of output files" );
   }
   else {
      PetscPrintf( PETSC_COMM_WORLD, "  MeSsAgE ERROR Could not copy the file %s.\n", filePathName.c_str() );
   }
   return status;
#endif
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::addUndefinedAreas ( BooleanLocal2DArray &validNodes, const Interface::GridMap* theMap ) {

   if ( theMap == 0 ) {
      return;
   }

   unsigned int i;
   unsigned int j;

   bool dataIsRetrieved = theMap->retrieved ();

   if ( not dataIsRetrieved ) {
      theMap->retrieveGhostedData ();
   }

   for (i = validNodes.first( 0 ); i <= validNodes.last( 0 ); ++i) {

      for (j = validNodes.first( 1 ); j <= validNodes.last( 1 ); ++j) {

         if (theMap->getValue( i, j ) == theMap->getUndefinedValue()) {
            validNodes( i, j ) = false;
         }

      }
   }

   if ( not dataIsRetrieved ) {
      // If the data was not retrived then restore the map back to its original state.
      theMap->restoreData ( false, true );
   }

}

//------------------------------------------------------------//

bool CrustalThicknessCalculator::parseCommandLine() {

   PetscBool isDefined = PETSC_FALSE;
   
   H5_Parallel_PropertyList::setOneFilePerProcessOption();
   PetscOptionsHasName (PETSC_NULL, "-xyz", &isDefined);
   if (isDefined) {
      m_outputOptions |= XYZ;
   }

   PetscOptionsHasName (PETSC_NULL, "-sur", &isDefined);
   if (isDefined) {
      m_outputOptions |= SUR;
   }

   PetscOptionsHasName (PETSC_NULL, "-debug", &isDefined);
   if (isDefined) {
      m_debug = true;
   }

   PetscOptionsHasName (PETSC_NULL, "-hdf", &isDefined);
   if (isDefined) {
      m_outputOptions |= HDF;
   }
   char outputFileName[128];
   outputFileName[0] = '\0';

   PetscOptionsGetString (PETSC_NULL, "-save", outputFileName, 128, &isDefined);
   if(isDefined) {
      m_outputFileName = outputFileName;
   }
   return true;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::setAdditionalOptionsFromCommandLine() {

   PetscBool isDefined = PETSC_FALSE;
   int radius;

   PetscOptionsHasName (PETSC_NULL, "-nosmooth", &isDefined);
   if (isDefined) {
      m_applySmoothing = false;
      m_smoothRadius = 0;
   }

   PetscOptionsGetInt ( PETSC_NULL, "-smooth", &radius, &isDefined );
   if (isDefined) {
      m_applySmoothing = ( radius > 0 );
      m_smoothRadius = radius;
   }

}