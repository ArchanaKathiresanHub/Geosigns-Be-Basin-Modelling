#include "SmectiteIlliteCalculator.h"
#include "SmectiteIlliteSimulator.h"
#include "SmectiteIlliteOutput.h"

#include "globaldefs.h"
#include "petscvec.h"
#include "PetscVectors.h"
#include "layer_iterators.h"
#include "property_manager.h"
#include "propinterface.h"
#include "CalibrationNode.h"
#include "NodeInput.h"

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

using namespace Basin_Modelling;

SmectiteIlliteCalculator::SmectiteIlliteCalculator( AppCtx* Application_Context ):
m_Basin_Model(Application_Context)
{
   database::Table * SmectiteIlliteKinIoTbl;
   SmectiteIlliteKinIoTbl = m_Basin_Model->database->getTable ("SmectiteIlliteKinIoTbl");
   assert(SmectiteIlliteKinIoTbl);
   database::Record * SmectiteIlliteKinIoTblRecord = * (SmectiteIlliteKinIoTbl->begin ());
 
   const double & ActEnergy1 = database::getActEnergy1(SmectiteIlliteKinIoTblRecord);
   const double & FreqFactor1 = database::getFreqFactor1(SmectiteIlliteKinIoTblRecord);
   const double & ActEnergy2 = database::getActEnergy2(SmectiteIlliteKinIoTblRecord);
   const double & FreqFactor2 = database::getFreqFactor2(SmectiteIlliteKinIoTblRecord);
   const double & InitIlliteFraction = database::getInitIlliteFraction(SmectiteIlliteKinIoTblRecord);

   m_SmectiteIlliteSimulator = new Calibration::SmectiteIlliteSimulator(ActEnergy1,FreqFactor1,
                                                                        ActEnergy2, FreqFactor2,InitIlliteFraction);
};


SmectiteIlliteCalculator::~SmectiteIlliteCalculator()
{
   delete m_SmectiteIlliteSimulator;
};

#undef __FUNCT__  
#define __FUNCT__ "SmectiteIlliteCalculator::computeSmectiteIlliteIncrement"

bool SmectiteIlliteCalculator::computeSmectiteIlliteIncrement( double time, double timeStep )
{
   int xs, ys, zs, xm, ym, zm;
   int i, j, k;

   Layer_Iterator Layers;
   Layers.Initialise_Iterator ( m_Basin_Model -> layers, Descending, Sediments_Only, Active_Layers_Only );

   while ( ! Layers.Iteration_Is_Done () ) 
   {
      LayerProps_Ptr Current_Layer = Layers.Current_Layer ();

      DMDAGetCorners(Current_Layer ->layerDA,&xs,&ys,&zs,&xm,&ym,&zm);

      PETSC_3D_Array layerTemperature ( Current_Layer -> layerDA, Current_Layer -> Current_Properties ( Basin_Modelling::Temperature ));

      for (i = xs; i < xs+xm; i++) 
      {
         for (j = ys; j < ys+ym; j++) 
         {
            for (k = zs; k < zs+zm; k++) 
            {
               Calibration::NodeInput currentInput( time,  layerTemperature( k, j, i ) );
               m_SmectiteIlliteSimulator->advanceState(currentInput, Current_Layer->m_SmectiteIlliteState(i,j,k));      
            }
         }
      }
      Layers++;
   }
   return true;

}


#undef __FUNCT__  
#define __FUNCT__ "SmectiteIlliteCalculator::computeSnapShotSmectiteIllite"

bool SmectiteIlliteCalculator::computeSnapShotSmectiteIllite ( const double time, const Boolean2DArray& validNeedleSet )
{
   int xs, ys, zs, xm, ym, zm;
   int i, j, k;

   Layer_Iterator Layers;
   Layers.Initialise_Iterator ( m_Basin_Model -> layers, Descending, Sediments_Only, Active_Layers_Only );

   while ( ! Layers.Iteration_Is_Done () ) 
   {
      LayerProps_Ptr Current_Layer = Layers.Current_Layer ();

      DMDAGetCorners(Current_Layer ->layerDA,&xs,&ys,&zs,&xm,&ym,&zm);

      PETSC_3D_Array layerTemperature ( Current_Layer -> layerDA, Current_Layer -> Current_Properties ( Basin_Modelling::Temperature ));

      DMCreateGlobalVector(Current_Layer ->layerDA, &Current_Layer ->m_IlliteFraction);
      VecSet(Current_Layer ->m_IlliteFraction, CAULDRONIBSNULLVALUE);

      double ***snapShotSmectiteIllite;
      DMDAVecGetArray(Current_Layer ->layerDA, Current_Layer ->m_IlliteFraction, &snapShotSmectiteIllite);
    
      for (i = xs; i < xs+xm; i++) 
      {
         for (j = ys; j < ys+ym; j++) 
         {
            if ( !validNeedleSet(i,j) ) continue;

            for (k = zs; k < zs+zm; k++) 
            {
               Calibration::NodeInput currentInput( time,  layerTemperature( k, j, i ) );
               m_SmectiteIlliteSimulator->advanceState(currentInput, Current_Layer->m_SmectiteIlliteState(i,j,k));//need to opt for ret: const T & instead of T 
          
               Calibration::SmectiteIlliteOutput currentOutput(time);
               m_SmectiteIlliteSimulator->computeOutput(Current_Layer->m_SmectiteIlliteState(i,j,k), currentOutput);
               snapShotSmectiteIllite[k][j][i] = currentOutput.getIlliteTransfRatio();
            }
         }
      }

      DMDAVecRestoreArray(Current_Layer ->layerDA, Current_Layer ->m_IlliteFraction,  &snapShotSmectiteIllite);

      Layers++;
   }
 
   return true;
}

