//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "CrustalThicknessCalculator.h"


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
#include "Interface/Formation.h"
#include "Interface/Interface.h"
#include "Interface/PaleoProperty.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"

// Crustal Thickness library
#include "DensityCalculator.h"
#include "LinearFunction.h"
#include "TotalTectonicSubsidenceCalculator.h"
#include "PaleowaterdepthCalculator.h"
#include "PaleowaterdepthResidualCalculator.h"
#include "Validator.h"

// Parallel _Hdf5 library
#include "h5_parallel_file_types.h"
#include "h5merge.h"

// utilitites
#include "LogHandler.h"

#include <typeinfo>       // operator typeid

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
   : DataAccess::Mining::ProjectHandle( database, name, accessMode, objectFactory ) {

   m_outputOptions = 0;
   m_debug = false;
   m_applySmoothing = true;
   m_smoothRadius = 0;

}
 
//------------------------------------------------------------//

CrustalThicknessCalculator::~CrustalThicknessCalculator () {
}

//------------------------------------------------------------//

CrustalThicknessCalculator* CrustalThicknessCalculator::CreateFrom( const string& inputFileName, ObjectFactory* factory ) {


   if ( m_crustalThicknessCalculator == 0 ) {
      m_crustalThicknessCalculator = (CrustalThicknessCalculator*)Interface::OpenCauldronProject( inputFileName, "rw", factory );

   }
   m_projectFileName = inputFileName;

   return m_crustalThicknessCalculator;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::loadSnapshots( ) {

   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Loading snpashots from stratigraphy";
   Interface::FormationList* formations = getFormations();
   Interface::FormationList::const_iterator formationIter;

   m_snapshots.push_back( 0.0 ); // add present day
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #time 0.0Ma loaded";

   for (formationIter = formations->begin(); formationIter != formations->end(); ++formationIter) {
      const Interface::Formation * formation = (*formationIter);

      if (formation != 0) {
         const Interface::Surface * topSurface = formation->getBottomSurface();
         m_snapshots.push_back( topSurface->getSnapshot()->getTime() );
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #time " << topSurface->getSnapshot()->getTime() << "Ma loaded";
      }
   }
   delete formations;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::initialise() {
   ///1. Initialise CTC instance
   bool started = CrustalThicknessCalculator::getInstance().startActivity( CrustalThicknessCalculatorActivityName,
      CrustalThicknessCalculator::getInstance().getHighResolutionOutputGrid(),
      true );
   if (!started) {
      throw CtcException() << "Can not start CrustalThicknessCalculator activity.";
   }

   ///2. Initialise GeoPhysics ProjectHandle
   started = GeoPhysics::ProjectHandle::initialise();
   if (!started) {
      throw CtcException() << "Can not start CrustalThicknessCalculator because geophysics project handle cannot be initialised.";
   }

   ///3. Initialise InterfaceInput
   setFormationLithologies( false, true );
   if (m_crustalThicknessData.size() != 1) {
      if (m_crustalThicknessData.size() == 0) {
         throw CtcException() << "The CrustalThicknessData table in the ProjectFile is empty.";
      }
      else {
         throw CtcException() << "Too many records for CrustalThicknessData table in the ProjectFile.";
      }
   }

   m_inputData = dynamic_cast<InterfaceInput *>(m_crustalThicknessData[0]);
   if (!m_inputData)
   {
      throw CtcException() << "Could not create the input interface from crustal thickness data.";
   }
   m_inputData->loadInputDataAndConfigurationFile( "InterfaceData.cfg" );

   updateValidNodes( m_inputData );

   ///4. Load smoothing radius
   m_smoothRadius = m_inputData->getSmoothRadius();
   m_applySmoothing = (m_smoothRadius > 0);
   /// @todo is it needed? --> to simplify in future requirement
   setAdditionalOptionsFromCommandLine();

   ///5. Initialise InterfaceOutput
   setRequestedOutputProperties( m_outputData );
   if (m_debug) {
      m_outputData.setAllMapsToOutput( true );
   }

   ///6. Load snapshots from stratigraphy
   loadSnapshots();
   std::sort( m_snapshots.begin(), m_snapshots.end());

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

void CrustalThicknessCalculator::setRequestedOutputProperties( InterfaceOutput & theOutput )
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
   theOutput.setMapsToOutput( mohoMap, topBasaltMap,
      thicknessBasaltMap, thicknessCrustMap, thicknessCrustMeltOnset, WLSadjustedMap, RDAadjustedMap,
      cumSedimentBackstrip, WLSMap, isostaticBathymetry, PaleowaterdepthResidual,
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
   Interface::IdentityFunctor identity;
   GridMap* previousTTS = 0;
   std::shared_ptr<GridMap> prensentDayPressureTTS;
   std::shared_ptr<GridMap> presentDayTTS;
   Validator validator( *this );
   unsigned int firstI = m_inputData->firstI();
   unsigned int firstJ = m_inputData->firstJ();
   unsigned int lastI  = m_inputData->lastI();
   unsigned int lastJ  = m_inputData->lastJ();

   for (k = 0; k < m_snapshots.size(); ++k) {

      const double age = m_snapshots[k];
      const Snapshot * theSnapshot = (const Snapshot *)findSnapshot( age );

      /// 1. Load P/T data fot this snapshot
      m_inputData->loadTopAndBottomOfSediments( m_crustalThicknessCalculator, age, m_inputData->getBaseRiftSurfaceName() );
      const DataModel::AbstractProperty* depthProperty = m_inputData->loadDepthProperty();
      m_inputData->loadDepthData( m_crustalThicknessCalculator, depthProperty, age );
      const DataModel::AbstractProperty* pressureProperty = m_inputData->loadPressureProperty();
      m_inputData->loadPressureData( m_crustalThicknessCalculator, pressureProperty, age );

      /// 2. Create the maps for this snapshot
      if (!m_outputData.createSnapShotOutputMaps( m_crustalThicknessCalculator, theSnapshot, m_inputData->getTopOfSedimentSurface() )) {
         throw CtcException() << "Cannot allocate output maps.";
      }

      m_inputData->retrieveData();
      m_outputData.retrieveData();
      if (previousTTS != nullptr) {
         previousTTS->retrieveData();
      }

      /// 3. Compute the backstripped density and thickness, the backtrip and the compensation
      DensityCalculator densityCalculator( firstI, firstJ, lastI, lastJ,
                                           m_inputData->getBackstrippingMantleDensity(),
                                           m_inputData->getWaterDensity(),
                                           m_inputData->getPressureBasement(),
                                           m_inputData->getPressureWaterBottom(),
                                           m_inputData->getDepthBasement(),
                                           m_inputData->getDepthWaterBottom(),
                                           m_outputData, validator );
      densityCalculator.compute();
      if (!m_debug) m_outputData.disableBackstripOutput( m_crustalThicknessCalculator, m_inputData->getBotOfSedimentSurface(), theSnapshot );

      /// 4. Compute the Total Tectonic Subsidence (only if we have a SDH at this snapshot)
      if (asSurfaceDepthHistory( age )){
         TotalTectonicSubsidenceCalculator TTScalculator( firstI, firstJ, lastI, lastJ,
                                                          age, densityCalculator.getAirCorrection(),
                                                          m_seaBottomDepth,
                                                          m_outputData, validator );
         TTScalculator.compute();
      }
      const Interface::Property* pressureInterfaceProperty = findProperty( "Pressure" );
      GridMap* currentPressureTTS = m_inputData->loadPropertyDataFromDepthMap( this, m_outputData.getMap( WLSMap ), pressureInterfaceProperty, theSnapshot );
      if (age == 0.0){
         presentDayTTS          = std::shared_ptr<GridMap>( this->getFactory()->produceGridMap( nullptr, 0, m_outputData.getMap( WLSMap ), identity ) );
         prensentDayPressureTTS = std::shared_ptr<GridMap>( this->getFactory()->produceGridMap( nullptr, 0, currentPressureTTS,            identity ) );;
      }
      
      /// 5. Compute the Paleowaterdepth
      PaleowaterdepthCalculator PWDcalculator( firstI, firstJ, lastI, lastJ,
                                               m_inputData->getBackstrippingMantleDensity(),
                                               m_inputData->getWaterDensity(),
                                               presentDayTTS.get(),
                                               m_outputData, validator,
                                               m_inputData->getPressureMantleAtPresentDay(),
                                               m_inputData->getPressureMantle(),
                                               prensentDayPressureTTS.get(), currentPressureTTS );
      PWDcalculator.compute();
      
      // 6. Compute the PaleowaterdepthResidual (only if we have a SDH at this snapshot and if we are not at present day)
      if (asSurfaceDepthHistory( age ) and age!=0.0){
         PaleowaterdepthResidualCalculator PWDRcalculator( firstI, firstJ, lastI, lastJ,
                                                           age, m_seaBottomDepth,
                                                           m_outputData, validator );
         PWDRcalculator.compute();
      }

      ///6. Smooth the TTS and PWD map
      if (m_applySmoothing) {
         LogHandler( LogHandler::INFO_SEVERITY ) << "Applying spatial smoothing with radius = " << m_smoothRadius << " @ snapshot " << age <<".";
         bool status = movingAverageSmoothing( m_outputData.getMap( WLSMap ) );
         if (!status) {
            throw CtcException() << "Failed to smooth WLS map.";
         }
         status = movingAverageSmoothing( m_outputData.getMap( isostaticBathymetry ) );
         if (!status) {
            throw CtcException() << "Failed to smooth isostaticBathymetry map.";
         }
      }

      ///7. Computes the thinning factor and crusltal thicknesses (to be refactored in future requirement)
      double WLS;
      double WLS_adjusted, TF, Moho, RDA_adjusted, crustalThickness, basaltThickness, ECT;
      double topBasalt;

      for (i = firstI; i <= lastI; ++i) {
         for (j = firstJ; j <= lastJ; ++j) {
            if (!m_inputData->defineLinearFunction( m_LF, i, j ) || !getNodeIsValid( i, j )) {
               m_outputData.setAllMapsUndefined( i, j );
            }
            else {

               m_outputData[slopePreMelt]      = m_LF.getM1();
               m_outputData[slopePostMelt]     = m_LF.getM2();
               m_outputData[interceptPostMelt] = m_LF.getC2();

               m_outputData[estimatedCrustDensityMap] = m_inputData->getEstimatedCrustDensity();
               m_outputData[TFOnsetMap]               = m_inputData->getTFOnset();
               m_outputData[TFOnsetLinMap]            = m_inputData->getTFOnsetLin();
               m_outputData[TFOnsetMigMap]            = m_inputData->getTFOnsetMig();
               m_outputData[PTaMap]                   = m_inputData->getPTa();
               m_outputData[basaltDensityMap]         = m_inputData->getMagmaticDensity();
               m_outputData[WLSOnsetMap]              = m_inputData->getWLSonset();
               m_outputData[WLSCritMap]               = m_inputData->getWLScrit();
               m_outputData[WLSExhumeMap]             = m_inputData->getWLSexhume();
               m_outputData[WLSExhumeSerpMap]         = m_inputData->getWLSexhumeSerp();
               m_outputData[thicknessCrustMeltOnset]  = m_inputData->getInitialCrustThickness() * (1 - m_inputData->getTFOnsetLin());

               WLS = m_outputData.getMapValue( WLSMap, i, j );
               m_outputData[incTectonicSubsidence] = WLS;
               if (previousTTS != 0) {
                  if (previousTTS->getValue( i, j ) != Interface::DefaultUndefinedMapValue) {
                     m_outputData[incTectonicSubsidence] = WLS - previousTTS->getValue( i, j );
                  }
                  else {
                     m_outputData[incTectonicSubsidence] = Interface::DefaultUndefinedMapValue;
                  }
               }

               if (WLS != Interface::DefaultUndefinedMapValue) {

                  WLS_adjusted = WLS - m_inputData->getDeltaSLValue( i, j );
                  RDA_adjusted = m_LF.getWLS_crit() - WLS_adjusted;

                  TF = m_LF.getCrustTF( WLS_adjusted );
                  crustalThickness = (TF < 1 ? m_inputData->getInitialCrustThickness() * (1 - TF) : 0);

                  if (WLS >= m_outputData[WLSExhumeMap]) basaltThickness = 0;
                  else basaltThickness = m_LF.getBasaltThickness( WLS_adjusted );

                  topBasalt = crustalThickness + m_inputData->getDepthBasement()->get(i,j);
                  Moho = topBasalt + basaltThickness;
               }

               if (WLS == Interface::DefaultUndefinedMapValue || m_inputData->getWLScrit() < m_inputData->getWLSonset()) {
                  // if WLS_crit < Wls_onset, set all mandatory outputs to Undefined value
                  m_outputData[WLSadjustedMap]          = Interface::DefaultUndefinedMapValue;
                  m_outputData[RDAadjustedMap]          = Interface::DefaultUndefinedMapValue;
                  m_outputData[TFMap]                   = Interface::DefaultUndefinedMapValue;
                  m_outputData[topBasaltMap]            = Interface::DefaultUndefinedMapValue;
                  m_outputData[thicknessCrustMap]       = Interface::DefaultUndefinedMapValue;
                  m_outputData[thicknessBasaltMap]      = Interface::DefaultUndefinedMapValue;
                  m_outputData[mohoMap]                 = Interface::DefaultUndefinedMapValue;
                  m_outputData[ECTMap]                  = Interface::DefaultUndefinedMapValue;
                  m_outputData[thicknessCrustMeltOnset] = Interface::DefaultUndefinedMapValue;
               }
               else {
                  m_outputData[WLSadjustedMap]     = WLS_adjusted;
                  m_outputData[RDAadjustedMap]     = RDA_adjusted;
                  m_outputData[TFMap]              = TF;
                  m_outputData[topBasaltMap]       = topBasalt;
                  m_outputData[thicknessCrustMap]  = crustalThickness;
                  m_outputData[thicknessBasaltMap] = basaltThickness;
                  m_outputData[mohoMap]            = Moho;

                  if (m_inputData->getInitialLithosphereThickness() != 0.0 &&
                     crustalThickness != Interface::DefaultUndefinedMapValue &&
                     basaltThickness != Interface::DefaultUndefinedMapValue) {

                     ECT = crustalThickness + basaltThickness * (m_inputData->getInitialCrustThickness() / m_inputData->getInitialLithosphereThickness());

                  }
                  else {
                     ECT = Interface::DefaultUndefinedMapValue;
                  }
                  m_outputData[ECTMap] = ECT;
               }

               // now put all values into the correspondent maps
               m_outputData.setValuesToMaps( i, j );
            }
         }
      }
      
      m_inputData->restoreData();
      m_outputData.restoreData();
      m_outputData.saveOutput( m_crustalThicknessCalculator, m_debug, m_outputOptions, theSnapshot );

      if (previousTTS != 0) {
         previousTTS->restoreData();
      }
      previousTTS = m_outputData.getMap( WLSMap );

      // Save properties to disk.
      m_crustalThicknessCalculator->continueActivity();
   }

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
              LogHandler( LogHandler::WARNING_SEVERITY ) << "Undefined num value in the smoothing algorithm!. Will be set to 1.";
              num = 1;
           } else if( num == 0 ) {
              LogHandler( LogHandler::WARNING_SEVERITY ) << "Zero num value in the smoothing algorithm! Will be set to 1.";
              num = 1;
           } else  if( num < 0 ) {
              LogHandler( LogHandler::WARNING_SEVERITY ) << "Negative num value in the smoothing algorithm! Will be set to 1.";
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


  delete[] columnMap[0];
  delete[] columnMap[1];
  delete[] columnMap;

  delete[] numberMapCollect[0];
  delete[] numberMapCollect[1];
  delete[] numberMapCollect;

  delete sumMap;
  delete numberMap;

  return status;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::updateValidNodes( const InterfaceInput* theInterfaceData ) {

   addUndefinedAreas( theInterfaceData->getT0Map     () );
   addUndefinedAreas( theInterfaceData->getTRMap     () );
   addUndefinedAreas( theInterfaceData->getHCuMap    () );
   addUndefinedAreas( theInterfaceData->getHLMuMap   () );
   addUndefinedAreas( theInterfaceData->getDeltaSLMap() );
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