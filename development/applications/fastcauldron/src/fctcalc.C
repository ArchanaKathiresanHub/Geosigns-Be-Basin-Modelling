#include "fctcalc.h"

#include <iostream>
using namespace std;

// #include "modelwriter.h"
#include "readproperties.h"
#include "fileio.h"
#include "PetscVectors.h"

#include "snapshotdata.h"

#include "layer_iterators.h"

#include "utils.h"
#include "FastcauldronSimulator.h"
#include "StatisticsHandler.h"
#include "Well.h" 

#include "Interface/RunParameters.h"

using namespace Basin_Modelling;

#ifdef ABS
#undef ABS
#endif
#define ABS(x)        ((x) < 0 ? -(x) : (x))

FCTCalc::FCTCalc( AppCtx* Application_Context )
{
   cauldron = Application_Context;

   // Clear the formation property list until the formation properties (e.g. Thickness) are computed in FCTCalc
   m_mapOutputProperties.clear ();

   m_volumeOutputProperties.push_back ( LITHOLOGY );
   m_volumeOutputProperties.push_back ( DEPTH );
   m_volumeOutputProperties.push_back ( VES );
   m_volumeOutputProperties.push_back ( MAXVES );
   m_volumeOutputProperties.push_back ( LITHOSTATICPRESSURE );

   // Required for porosity calculation.
   m_volumeOutputProperties.push_back ( CHEMICAL_COMPACTION ); 
   m_volumeOutputProperties.push_back ( POROSITYVEC );

   if ( FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE )
   {
      m_mapOutputProperties.push_back ( DEPTH );
      m_mapOutputProperties.push_back ( THICKNESS );
   }
   else if ( FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE ) 
   {
      m_mapOutputProperties.push_back ( DEPTH );
      m_mapOutputProperties.push_back ( THICKNESS );
      m_mapOutputProperties.push_back ( MAXVES );
      m_mapOutputProperties.push_back ( VES );
   }
   else if ( FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_DECOMPACTION_MODE )
   {
      m_mapOutputProperties.push_back ( DEPTH );
      m_mapOutputProperties.push_back ( MAXVES );
      m_mapOutputProperties.push_back ( VES );
      m_mapOutputProperties.push_back ( POROSITYVEC );
      m_mapOutputProperties.push_back ( THICKNESS );
      m_mapOutputProperties.push_back ( BULKDENSITYVEC );
      m_mapOutputProperties.push_back ( LITHOSTATICPRESSURE );
   }

}

void FCTCalc::printTestVals()
{
  
   size_t layerNr;
   unsigned int i; 
   unsigned int j;

   for (layerNr = 0; layerNr < cauldron->layers.size(); layerNr++) {
      LayerProps* pLayer = cauldron->layers[layerNr];
    

      for ( i = FastcauldronSimulator::getInstance ().firstI (); i <= FastcauldronSimulator::getInstance ().lastI (); ++i ) {

         for ( j = FastcauldronSimulator::getInstance ().firstJ (); j <= FastcauldronSimulator::getInstance ().lastJ (); ++j ) {

            if ( not cauldron->nodeIsDefined ( i, j )) continue;

            int segmentNr;

            for (segmentNr = pLayer->getMaximumNumberOfElements()-1; segmentNr >= 0; segmentNr--) {
               Polyfunction *polyf = &(pLayer->getSolidThickness ( i, j, segmentNr ));
               Polyfunction::Polypoint::iterator polyfit;

               cout << "   {" << segmentNr << "} ";

               for (polyfit = polyf->getBegin(); polyfit != polyf->getEnd(); polyfit++) {
                  cout << "(" << (*polyfit)->getX() << " , " 
                       << (*polyfit)->getY() << ")" << " ";
               }

               cout << endl;
            }

         }

      }
    
   }

}

void FCTCalc::decompact(){

  if ( FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_DECOMPACTION_MODE ) {
     FastcauldronSimulator::getInstance ().deleteMinorSnapshotsFromSnapshotTable ();
  }
  
  // Perform decompaction
  SnapshotEntrySetIterator it;

  for (it= cauldron->projectSnapshots.majorSnapshotsBegin (); it != cauldron->projectSnapshots.majorSnapshotsEnd (); ++it) {
    
    double SnapShotTime = (*it)->time ();
    
    if (cauldron->DoDecompaction)
      displayProgress(cauldron->debug1, "o Decompacting... ",SnapShotTime);
    else
      displayProgress (cauldron->debug1, "o HighRes Decompacting... ", SnapShotTime);
    
    getGrid ( SnapShotTime, it );
    StatisticsHandler::update ();
    MPI_Barrier(PETSC_COMM_WORLD);

    writeCauldronSnapShotTime((*it)->time ());

#if 0
    if (cauldron->fileOutput == FCMAPandVOLUME) {
      writeCauldronSnapShotTime((*it)->time ());
    } else if (cauldron->fileOutput == FCMAP) {
      writeCauldronSnapShotTime((*it)->time ());
    }
#endif

  }

  {
    Well wells( cauldron );
    wells.Save_Present_Day_Data ();
  }

  if (cauldron->DoDecompaction)
  {
     displayTime(cauldron->debug1,"Decompaction Calculation: ");
  }
  else
  {
     displayTime (cauldron->debug1, "HighRes Decompaction Calculation: ");
  }

}


void FCTCalc::writeCauldronSnapShotTime ( const double time ) {

  if ( ! cauldron->DoHDFOutput ) {
    return;
  }

  if ( cauldron->filterwizard.thicknessCalculationNeeded ()) {
    computeThicknessVectors ( cauldron );
  }

  const Interface::Snapshot* snapshot = FastcauldronSimulator::getInstance ().findOrCreateSnapshot ( time );
  assert ( snapshot != 0 );

  FastcauldronSimulator::getInstance ().saveMapProperties ( m_mapOutputProperties,
                                                            snapshot,
                                                            Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );

  if ( cauldron->filterwizard.thicknessCalculationNeeded ()) {
    deleteThicknessVectors ( cauldron );
  }

  if ( FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_DECOMPACTION_MODE ) {
     cauldron->Retrieve_Lithology_ID ();
     FastcauldronSimulator::getInstance ().saveVolumeProperties ( m_volumeOutputProperties,
                                                                  snapshot,
                                                                  Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
     cauldron->deleteLithologyIDs ();
  }

}

bool FCTCalc::getGrid ( const double                    currentTime,
                        const SnapshotEntrySetIterator& majorSnapshots ) {

  int M = 0, N = 0, P = 0, m = 0, n = 0, p = 0;
  bool status = true;

  status &= cauldron->findActiveElements( currentTime );

  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( cauldron->layers, Descending, Basement_And_Sediments, 
			       Active_And_Inactive_Layers );


  while ( ! Layers.Iteration_Is_Done () ) {

    LayerProps_Ptr Current_Layer = Layers.Current_Layer();

    DMDAGetInfo(*cauldron->mapDA, PETSC_NULL,
                &M, &N, &P, &m, &n, &p,
                PETSC_NULL,PETSC_NULL,PETSC_NULL,PETSC_NULL,PETSC_NULL,PETSC_NULL);

    status &= Current_Layer -> allocateNewVecs ( cauldron );

    Layers++;
  }

  if ( cauldron->IsCalculationCoupled && cauldron->DoHighResDecompaction && currentTime < FastcauldronSimulator::getInstance ().getAgeOfBasin ()) {
  // if ( cauldron->IsCalculationCoupled && cauldron->DoHighResDecompaction && currentTime < cauldron->Age_Of_Basin ()) {
    SnapshotInterval interval;

    cauldron->projectSnapshots.getCurrentSnapshotInterval ( majorSnapshots, interval );
    cauldron->setSnapshotInterval ( interval );
  }


  status &= cauldron->calcNodeVes( currentTime );
  PETSC_ASSERT( status );

  status &= cauldron->calcNodeMaxVes( currentTime );
  PETSC_ASSERT( status );

  status &= cauldron->calcPorosities(  currentTime  );
  PETSC_ASSERT( status );

  if ( cauldron->IsCalculationCoupled and cauldron->DoHighResDecompaction and not cauldron->isGeometricLoop ()) {
    status &= cauldron->setNodeDepths( currentTime );
  } else {
    status &= cauldron->calcNodeDepths( currentTime );
  }
  PETSC_ASSERT( status );
  
  status &= cauldron->Calculate_Pressure(  currentTime  );
  PETSC_ASSERT( status );



  if ( cauldron->debug1 ) monitorProcessMemorySize();

  return status;

}
