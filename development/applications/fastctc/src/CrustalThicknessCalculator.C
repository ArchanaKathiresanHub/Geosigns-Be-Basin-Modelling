#include <sstream>
#include <petsc.h>
#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
using namespace database;

#include "Interface/CrustalThicknessData.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"

#include "CrustalThicknessCalculator.h"
#include "DensityCalculator.h"
#include "LinearFunction.h"

#include "h5_parallel_file_types.h"
#include "h5merge.h"

//------------------------------------------------------------//

CrustalThicknessCalculator* CrustalThicknessCalculator::m_crustalThicknessCalculator = 0;
string CrustalThicknessCalculator::m_projectFileName = "";
string CrustalThicknessCalculator::m_outputFileName = "";

//------------------------------------------------------------//
void displayTime ( const double timeToDisplay, const char * msgToDisplay ) {

   int hours   = (int)(  timeToDisplay / 3600.0 );
   int minutes = (int)(( timeToDisplay - (hours * 3600.0) ) / 60.0 );
   int seconds = (int)(  timeToDisplay - hours * 3600.0 - minutes * 60.0 );
   
   PetscPrintf ( PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n" );
   PetscPrintf ( PETSC_COMM_WORLD, "%s: %d hours %d minutes %d seconds\n", msgToDisplay, hours, minutes, seconds );

}
//------------------------------------------------------------//

CrustalThicknessCalculator::CrustalThicknessCalculator (database::Database * database, const std::string & name, const std::string & accessMode) 
   : Interface::ProjectHandle (database, name, accessMode) {

   
   m_outputOptions = 0;
   m_debug = false;
   m_applySmoothing = true;
}
 
//------------------------------------------------------------//

CrustalThicknessCalculator::~CrustalThicknessCalculator () {
}

//------------------------------------------------------------//

CrustalThicknessCalculator* CrustalThicknessCalculator::CreateFrom ( const string& inputFileName ) {


   if ( m_crustalThicknessCalculator == 0 ) {
      m_crustalThicknessCalculator = (CrustalThicknessCalculator*) Interface::OpenCauldronProject ( inputFileName, "rw" );

   }
   m_projectFileName = inputFileName;

   return m_crustalThicknessCalculator;
}

//------------------------------------------------------------//

void CrustalThicknessCalculator::finalise ( const bool saveResults ) {


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
   theOutput.setMapsToOutput(mohoMap, thicknessBasaltMap, WLSadjustedMap, RDAadjustedMap, thicknessCrustMap, ECTMap, thicknessCrustMeltOnset, topBasaltMap,
                             numberOfOutputMaps);
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

   bool started = CrustalThicknessCalculator::getInstance().startActivity (  CrustalThicknessCalculatorActivityName, 
                                                                             CrustalThicknessCalculator::getInstance().getHighResolutionOutputGrid (),
									     true);

   if( !started ) {
      string s = "Can not start CrustalThicknessCalculator";
      throw s;
   }

   LinearFunction theLF;
   DensityCalculator theDensityCalculator;
   InterfaceOutput theOutput;
  
   if( m_crustalThicknessData.size() != 1 ) {
      string s;
      if( m_crustalThicknessData.size() == 0 ) {
         s = "The CrustalThicknessData table in the ProjectFile is empty.";
      } else {
         s = "Too many records for CrustalThicknessData table in the ProjectFile.";
      }
      throw s;
   }

   InterfaceInput &theInterfaceData = dynamic_cast<InterfaceInput &>( * m_crustalThicknessData[0] );
   theInterfaceData.loadInputDataAndConfigurationFile( "InterfaceData.cfg" );

   //   theInterfaceData->defineLinearFunction ( theLF, m_age );

   theDensityCalculator.loadData( m_crustalThicknessCalculator, theInterfaceData.getBaseRiftSurfaceName() );

   // theOutput.setMapsToOutput(CrustalThicknessInterface::mohoMap, CrustalThicknessInterface::thicknessBasaltMap, CrustalThicknessInterface::numberOfOutputMaps);

   setRequestedOutputProperties( theOutput );

   if(m_debug) {
      theOutput.setAllMapsToOutput( true );
   }
   //   if( !theOutput.allocateOutputMaps( m_crustalThicknessCalculator ) ) {

   bool toOutputWLS = theOutput.getOutputMask( WLSMap );

   const double age = 0.0;
   const Snapshot * theSnapshot = (const Snapshot *) findSnapshot (age);
   
   if( !theOutput.createSnapShotOutputMaps( m_crustalThicknessCalculator, theSnapshot ) ) {
      string s = "Can not allocate output maps.";
      throw s;
   };
  

   if( m_applySmoothing && !toOutputWLS ) {
      theOutput.allocateOutputMap( m_crustalThicknessCalculator, WLSMap );
   }

   theInterfaceData.retrieveData();
   theDensityCalculator.retrieveData();
   theOutput.retrieveData();
   ////

   unsigned lastI, lastJ, firstI, firstJ;

   firstI = theInterfaceData.firstI();
   firstJ = theInterfaceData.firstJ();
   lastI  = theInterfaceData.lastI();
   lastJ  = theInterfaceData.lastJ();
   
   //   const Interface::Grid * grid = m_crustalThicknessCalculator->getActivityOutputGrid ();

   unsigned int i, j;
   double sedimentDensity, sedimentThickness, WLS, WLS_adjusted, TF, Moho, RDA_adjusted, crustalThickness, basaltThickness, ECT;
   double topBasalt, posI, posJ;

   bool status = true;
   stringstream errorMsg;

    for ( i = firstI; i <= lastI; ++ i ) {
      for ( j = firstJ; j <= lastJ; ++ j ) { 
         if (!theInterfaceData.defineLinearFunction ( theLF, i, j )) {
            //            theOutput[WLSMap] = Interface::DefaultUndefinedMapValue;
            WLS = Interface::DefaultUndefinedMapValue;
         } else {
            theDensityCalculator.computeNode( i, j );
            //            theOutput[WLSMap] = theDensityCalculator.getWLS( theInterfaceData.getBackstrippingMantleDensity(), theInterfaceData.getDensityDifference() );
            WLS = theDensityCalculator.getWLS( theInterfaceData.getBackstrippingMantleDensity(), theInterfaceData.getDensityDifference() );
         }
         theOutput.setMapValue( WLSMap, i, j, WLS );
      }
   }
   if( m_applySmoothing ) {
      //apply smooth algorithm
      bool status = movingAverageSmoothing( theOutput.getMap( WLSMap ) );
      if( !status ) {
         string s = "Failed to smooth WLS map.";
         throw s;
      }
   }

   for ( i = firstI; i <= lastI; ++ i ) {
      for ( j = firstJ; j <= lastJ; ++ j ) { 
         if (!theInterfaceData.defineLinearFunction ( theLF, i, j )) {
            // errorMsg << "I = " << i << "; J = " << j << endl;
            // status = false;
            // break;
            theOutput.setAllMapsUndefined( i, j );
            
         } else {
            
            ///// Check for undefined value?  
            theDensityCalculator.computeNode( i, j );
            sedimentDensity = theDensityCalculator.getSedimentDensity();
            
            theOutput[slopePreMelt] = theLF.getM1();
            theOutput[slopePostMelt] = theLF.getM2();
            theOutput[interceptPostMelt] = theLF.getC2();
            
            theOutput[sedimentDensityMap] = sedimentDensity;
            theOutput[estimatedCrustDensityMap] = theInterfaceData.getEstimatedCrustDensity();
            theOutput[TFOnsetMap] = theInterfaceData.getTFOnset();
            theOutput[TFOnsetLinMap] = theInterfaceData.getTFOnsetLin();
            theOutput[TFOnsetMigMap] = theInterfaceData.getTFOnsetMig();
            theOutput[PTaMap] = theInterfaceData.getPTa();
            theOutput[basaltDensityMap] = theInterfaceData.getMagmaticDensity();
            theOutput[WLSOnsetMap] = theInterfaceData.getWLSonset();
            theOutput[WLSCritMap] = theInterfaceData.getWLScrit();
            theOutput[WLSExhumeMap] = theInterfaceData.getWLSexhume();
            theOutput[WLSExhumeSerpMap] = theInterfaceData.getWLSexhumeSerp();
            theOutput[thicknessCrustMeltOnset] = theInterfaceData.getInitialCrustThickness() * (1 - theInterfaceData.getTFOnsetLin());
            
            WLS = theOutput.getMapValue(WLSMap, i, j);
            theOutput[WLSMap] = WLS;
            
            if ( WLS == Interface::DefaultUndefinedMapValue ) { 
               WLS_adjusted = Interface::DefaultUndefinedMapValue; 
               RDA_adjusted = Interface::DefaultUndefinedMapValue; 
               TF =  Interface::DefaultUndefinedMapValue; 
               crustalThickness = Interface::DefaultUndefinedMapValue; 
               basaltThickness = Interface::DefaultUndefinedMapValue; 
               Moho = Interface::DefaultUndefinedMapValue; 
               topBasalt = Interface::DefaultUndefinedMapValue;  
            } else {
               if( theInterfaceData.getDeltaSLValue(i, j) == Interface::DefaultUndefinedMapValue ) {
                  WLS_adjusted = Interface::DefaultUndefinedMapValue; 
               } else {
                  WLS_adjusted = WLS - theInterfaceData.getDeltaSLValue(i, j); 
               }
               RDA_adjusted = theLF.getWLS_crit() - WLS_adjusted;
               TF = theLF.getCrustTF(WLS_adjusted);
               crustalThickness = (TF < 1 ? theInterfaceData.getInitialCrustThickness() * (1 - TF) : 0);
               
               if(WLS >= theOutput[WLSExhumeMap]) basaltThickness = 0; 
               else basaltThickness = theLF.getBasaltThickness( WLS_adjusted );

               if( theDensityCalculator.getTopBasementDepthValue() != Interface::DefaultUndefinedMapValue ) { 
                  topBasalt = crustalThickness + theDensityCalculator.getTopBasementDepthValue();
                  Moho = topBasalt + basaltThickness;
               } else {
                  Moho = Interface::DefaultUndefinedMapValue; 
                  topBasalt = Interface::DefaultUndefinedMapValue; 
               }
            }

            if(  WLS == Interface::DefaultUndefinedMapValue || theInterfaceData.getWLScrit() < theInterfaceData.getWLSonset() ) {
               // if WLS_crit < Wls_onset, set all mandatory outputs to Undefined value
               theOutput[WLSadjustedMap] = Interface::DefaultUndefinedMapValue; 
               theOutput[RDAadjustedMap] = Interface::DefaultUndefinedMapValue; 
               theOutput[TFMap] = Interface::DefaultUndefinedMapValue;
               theOutput[topBasaltMap] = Interface::DefaultUndefinedMapValue;
               theOutput[thicknessCrustMap] = Interface::DefaultUndefinedMapValue;
               theOutput[thicknessBasaltMap] = Interface::DefaultUndefinedMapValue;
               theOutput[mohoMap] = Interface::DefaultUndefinedMapValue;
               theOutput[ECTMap] = Interface::DefaultUndefinedMapValue; 
               theOutput[thicknessCrustMeltOnset] = Interface::DefaultUndefinedMapValue;  
            } else {
               theOutput[WLSadjustedMap] = WLS_adjusted;
            
               theOutput[RDAadjustedMap] = RDA_adjusted;
            
               theOutput[TFMap] = TF;
            
               // Calculate top basalt map
               theOutput[topBasaltMap] = topBasalt;

               // Calculate crustal thickness
               theOutput[thicknessCrustMap] = crustalThickness;
            
               // Calculate basalt thickness
               theOutput[thicknessBasaltMap] = basaltThickness;
             
               theOutput[mohoMap] = Moho;
            
               if( theInterfaceData.getInitialLithosphereThickness() != 0.0 &&
                   crustalThickness != Interface::DefaultUndefinedMapValue &&
                   basaltThickness != Interface::DefaultUndefinedMapValue ) {
                  
                  ECT = crustalThickness + basaltThickness * (theInterfaceData.getInitialCrustThickness() /  theInterfaceData.getInitialLithosphereThickness());
                  if( ECT < 1000 ) ECT = 1000;
                  
               } else {
                  ECT = Interface::DefaultUndefinedMapValue;
               }
               theOutput[ECTMap] = ECT;
            } 

            // now put all values into the correspondent maps
            theOutput.setValuesToMaps( i, j );
         }
      }
   }
   if( !status ) {
      string s = "Linear function can not be defined due to wrong rift event age at " + errorMsg.str();
      throw s;
   }


   ///
   theInterfaceData.restoreData();
   theDensityCalculator.restoreData();
   theOutput.restoreData();

   if( m_debug ) {
      if( m_outputOptions & XYZ ) {
         if( m_crustalThicknessCalculator->getSize() > 1 ) {
            cout << "Can not save maps in XYZ format in parallel. Run with nprocs = 1." << endl;
         } else {
            theOutput.saveXYZOutputMaps( m_crustalThicknessCalculator );
         }
      }
      if( m_outputOptions & SUR ) {
         if( m_crustalThicknessCalculator->getSize() > 1 ) {
            cout << "Can not save maps in XYZ format in parallel. Run with nprocs = 1." << endl;
         } else {
            theOutput.saveExcelSurfaceOutputMaps( m_crustalThicknessCalculator );
         }
      }
      if( m_outputOptions & HDF ) {
         theOutput.saveOutputMaps( m_crustalThicknessCalculator, theSnapshot );
      }
   } else if( m_outputOptions & XYZ ) {
      theOutput.saveXYZOutputMaps( m_crustalThicknessCalculator);
   } else  if( m_outputOptions & HDF ) {
      theOutput.saveOutputMaps( m_crustalThicknessCalculator, theSnapshot );
   }
   if( !toOutputWLS && m_applySmoothing ) {
      theOutput.deleteOutputMap( WLSMap );      
   }
}

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
bool CrustalThicknessCalculator::movingAverageSmoothing( GridMap * aMap) {

 
  const Interface::Grid * grid = getActivityOutputGrid ();
 
  bool status = true;

  GridMap * sumMap = getFactory()->produceGridMap(0, 0, grid, Interface::DefaultUndefinedMapValue, 1);    

  if( sumMap == 0 ) {
     return false;
  } 

  GridMap * numberMap = getFactory()->produceGridMap(0, 0, grid, Interface::DefaultUndefinedMapValue, 1 );  
  
  if( numberMap == 0 ) {
     delete sumMap;
     return false;
  }
  const unsigned int averageRadius = 5;
 
  //  cout << "Applying moving average smoothing to WLS (radius = " << averageRadius << ")" << endl;

  unsigned int lastI, lastJ, firstI, firstJ;
  unsigned int i, j, jj, ii, ii1;
  double val;
  bool undefValue;
  int num = 0;

  //aMap->retrieveData(); already retreived
  sumMap->retrieveData();
  numberMap->retrieveData();

  firstI = aMap->firstI();
  firstJ = aMap->firstJ();
  lastI  = aMap->lastI();
  lastJ  = aMap->lastJ();

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

  double multVal;

  for( i = firstI; i <= lastI; ++ i ) {
     
     for ( j = firstJ; j <= lastJ; ++ j ) { 
        val = sumMap->getValue( i, j );
        num = numberMap->getValue( i, j );

        if( val != Interface::DefaultUndefinedMapValue && aMap->getValue(i, j ) != Interface::DefaultUndefinedMapValue ) {
           if( num == Interface::DefaultUndefinedMapValue ) {
              cout << "WARNING --- Undefined num value in smoothing algorithm!" << endl;
              num = 1;
           } else if( num == 0 ) {
              cout << "WARNING --- Zero num value in smoothing algorithm!" << endl;
              num = 1;
           } else  if( num < 0 ) {
              cout << "WARNING --- Negative num value in smoothing algorithm!" << endl;
              num = 1;
           } 
           multVal = 1.0 / (double)num;
           aMap->setValue( i, j, val  * multVal );
        }
     }
  }


  // needs to add average across the borders
  sumMap->restoreData();
  numberMap->restoreData();
  // aMap->restoreData(); will be restored at the end

  delete columnMap;
  delete sumMap;
  delete numberMap;
  delete numberMapCollect;

  return status;
}
//------------------------------------------------------------//
bool CrustalThicknessCalculator::mergeOutputFiles ( ) {

   if( ! H5_Parallel_PropertyList::isOneFilePerProcessEnabled() ) return true;
  
   PetscBool noFileCopy = PETSC_FALSE;

   PetscOptionsHasName( PETSC_NULL, "-nocopy", &noFileCopy );

   PetscLogDouble merge_Start_Time;
   PetscTime( &merge_Start_Time );

   string fileName = CrustalThicknessCalculatorActivityName + "_Results.HDF" ; 
   string filePathName = getProjectPath () + "/" + getOutputDir () + "/" + fileName;

   bool status = mergeFiles ( allocateFileHandler( PETSC_COMM_WORLD, filePathName, H5_Parallel_PropertyList::getTempDirName(), ( noFileCopy ? CREATE : REUSE )));
   
   if( status ) {
      status = H5_Parallel_PropertyList::copyMergedFile( filePathName );
   }
   if( status ) {
      PetscLogDouble merge_End_Time;
      PetscTime( &merge_End_Time );
      
      displayTime( merge_End_Time - merge_Start_Time, "Merging of output files" );
   } else {
      PetscPrintf ( PETSC_COMM_WORLD, "  MeSsAgE ERROR Could not copy the file %s.\n", filePathName.c_str() );
   }
   return status;
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

   PetscOptionsHasName (PETSC_NULL, "-nosmooth", &isDefined);
   if (isDefined) {
      m_applySmoothing = false;
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
