//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "BiomarkersCalculator.h"

#include "BiomarkersSimulator.h"
#include "BiomarkersOutput.h"

#include "ConstantsFastcauldron.h"
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

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;

using namespace Basin_Modelling;

BiomarkersCalculator::BiomarkersCalculator( AppCtx* Application_Context ):
m_Basin_Model(Application_Context)
{
  database::Table * BiomarkerKinIoTbl;
  BiomarkerKinIoTbl = m_Basin_Model->database->getTable ("BiomarkerKinIoTbl");
  assert(BiomarkerKinIoTbl);
  database::Record * BiomarkerKinIoTblRecord = * (BiomarkerKinIoTbl->begin ());
 
  const double & HopIsoActEnergy 	= database::getHopIsoActEnergy(BiomarkerKinIoTblRecord);
  const double & SteIsoActEnergy 	= database::getSteIsoActEnergy(BiomarkerKinIoTblRecord);
  const double & SteAroActEnergy 	= database::getSteAroActEnergy(BiomarkerKinIoTblRecord);
  const double & HopIsoFreqFactor 	= database::getHopIsoFreqFactor(BiomarkerKinIoTblRecord);
  const double & SteIsoFreqFactor	= database::getSteIsoFreqFactor(BiomarkerKinIoTblRecord);
  const double & SteAroFreqFactor 	= database::getSteAroFreqFactor(BiomarkerKinIoTblRecord);
  const double & HopIsoGamma 		= database::getHopIsoGamma(BiomarkerKinIoTblRecord);
  const double & SteIsoGamma 		= database::getSteIsoGamma(BiomarkerKinIoTblRecord);

  m_BiomarkersSimulator = new Calibration::BiomarkersSimulator(	HopIsoActEnergy , 
                                                                SteIsoActEnergy , 
                                                                SteAroActEnergy , 
                                                                HopIsoFreqFactor, 
                                                                SteIsoFreqFactor, 
                                                                SteAroFreqFactor, 
                                                                HopIsoGamma, 
                                                                SteIsoGamma );
};


BiomarkersCalculator::~BiomarkersCalculator()
{
  delete m_BiomarkersSimulator;
};

#undef __FUNCT__  
#define __FUNCT__ "BiomarkersCalculator::computeBiomarkersIncrement"

bool BiomarkersCalculator::computeBiomarkersIncrement( double time, double timeStep )
{
  int xs, ys, zs, xm, ym, zm;
  int i, j, k;

  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( m_Basin_Model -> layers, Descending, Sediments_Only, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done () ) 
  {
    LayerProps_Ptr Current_Layer = Layers.Current_Layer ();

    DMDAGetCorners( Current_Layer ->layerDA, &xs, &ys, &zs, &xm, &ym, &zm );

    PETSC_3D_Array layerTemperature ( Current_Layer -> layerDA, Current_Layer -> Current_Properties ( Basin_Modelling::Temperature ));

    for (i = xs; i < xs+xm; i++) 
    {
      for (j = ys; j < ys+ym; j++) 
      {
        for (k = zs; k < zs+zm; k++) 
        {
          Calibration::NodeInput currentInput( time,  layerTemperature( k, j, i ) );
          m_BiomarkersSimulator->advanceState(currentInput, Current_Layer->m_BiomarkersState(i,j,k));
        }
      }
    }
    Layers++;
  }
  return true;

}


#undef __FUNCT__  
#define __FUNCT__ "BiomarkersCalculator::computeSnapShotBiomarkers"

bool BiomarkersCalculator::computeSnapShotBiomarkers ( const double time, const Boolean2DArray& validNeedleSet )
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

		DMCreateGlobalVector(Current_Layer ->layerDA, &Current_Layer ->m_SteraneAromatisation);
		VecSet(Current_Layer ->m_SteraneAromatisation,CauldronNoDataValue);
		DMCreateGlobalVector(Current_Layer ->layerDA, &Current_Layer ->m_SteraneIsomerisation);
		VecSet(Current_Layer ->m_SteraneIsomerisation,CauldronNoDataValue);
		DMCreateGlobalVector(Current_Layer ->layerDA, &Current_Layer ->m_HopaneIsomerisation);
		VecSet(Current_Layer ->m_HopaneIsomerisation,CauldronNoDataValue);

		double ***SteraneAromatisation;
                double ***SteraneIsomerisation;
		double ***HopaneIsomerisation;

		DMDAVecGetArray(Current_Layer ->layerDA, Current_Layer ->m_SteraneAromatisation, &SteraneAromatisation);
		DMDAVecGetArray(Current_Layer ->layerDA, Current_Layer ->m_SteraneIsomerisation, &SteraneIsomerisation);
		DMDAVecGetArray(Current_Layer ->layerDA, Current_Layer ->m_HopaneIsomerisation, &HopaneIsomerisation);
		
		for (i = xs; i < xs+xm; i++) 
		{
			for (j = ys; j < ys+ym; j++) 
			{
				if ( !validNeedleSet(i,j) ) continue;

				for (k = zs; k < zs+zm; k++) 
				{
                                   //Calibration::NodeInput currentInput( time,  layerTemperature( k, j, i ) );
                                   //m_BiomarkersSimulator->advanceState(currentInput, Current_Layer->m_BiomarkersState(i,j,k));//need to opt for ret: const T & instead of T 
					
					Calibration::BiomarkersOutput currentOutput(time);
					m_BiomarkersSimulator->computeOutput(Current_Layer->m_BiomarkersState(i,j,k), currentOutput);

					SteraneAromatisation[k][j][i]= currentOutput.getSteraneAromatisation();
					SteraneIsomerisation[k][j][i]= currentOutput.getSteraneIsomerisation();
					HopaneIsomerisation [k][j][i]= currentOutput.getHopaneIsomerisation();
				}
			}
		}

		DMDAVecRestoreArray(Current_Layer ->layerDA, Current_Layer ->m_SteraneAromatisation, &SteraneAromatisation);
		DMDAVecRestoreArray(Current_Layer ->layerDA, Current_Layer ->m_SteraneIsomerisation, &SteraneIsomerisation);
		DMDAVecRestoreArray(Current_Layer ->layerDA, Current_Layer ->m_HopaneIsomerisation, &HopaneIsomerisation);

		Layers++;
	}
 
  return true;
}

