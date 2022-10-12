//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "layer.h"

#include <iostream>

#include <assert.h>

#include "array.h"
#include "utils.h"

#include "FiniteElementTypes.h"
#include "element_contributions.h"
#include "propinterface.h"

#include "PressureSolver.h"
#include "HydraulicFracturingManager.h"


#include "FastcauldronSimulator.h"

#include "auxiliaryfaulttypes.h"
#include "Faulting.h"
#include "FaultCollection.h"
#include "FaultElementCalculator.h"
#include "Snapshot.h"
#include "Formation.h"
#include "Grid.h"
#include "GridMap.h"
#include "Surface.h"
#include "ObjectFactory.h"
#include "RunParameters.h"

#include "Species.h"
#include "ChemicalModel.h"
#include "Simulator.h"


#include "CompoundLithology.h"
#include "Lithology.h"

#include "GeoPhysicalConstants.h"
#include "GeoPhysicsSourceRock.h"
#include "GeoPhysicsFormation.h"

#include "PetscBlockVector.h"

#include "ElementFaceValues.h"
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"

#include "ImmobileSpeciesValues.h"
#include "GeoPhysicalFunctions.h"

using namespace DataAccess;
using namespace FiniteElementMethod;

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;
using Utilities::Numerical::IbsNoDataValue;
#include "ConstantsMathematics.h"
using Utilities::Maths::Zero;
#include "NumericFunctions.h"
using namespace std;

using Interface::X_COORD;
using Interface::Y_COORD;

//------------------------------------------------------------//

LayerProps::LayerProps ( Interface::ProjectHandle& projectHandle,
                         database::Record *              record ) :
   DataAccess::Interface::Formation ( projectHandle, record ),
   GeoPhysics::GeoPhysicsFormation ( projectHandle, record ),
   layerDA(nullptr),
   depthvec(nullptr),
   Depth(nullptr),
   Lithology_ID(nullptr),
   BulkDensXHeatCapacity(nullptr),
   BulkTHCondN(nullptr),
   BulkTHCondP(nullptr),
   BulkHeatProd(nullptr),
   FCTCorrection(nullptr),
   Diffusivity(nullptr),
   Porosity(nullptr),
   Velocity(nullptr),
   Reflectivity(nullptr),
   Sonic(nullptr),
   BulkDensity(nullptr),
   ThCond(nullptr),
   PermeabilityV(nullptr),
   PermeabilityH(nullptr),
   m_averagedSaturation(nullptr),
   m_timeOfElementInvasionVec(nullptr),
   Vre(nullptr),
   layerThickness(nullptr),
   Thickness_Error(nullptr),
   erosionFactor(nullptr),
   Computed_Deposition_Thickness(nullptr),
   faultElements(nullptr),
   allochthonousLithologyMap(nullptr),
   Real_Thickness_Vector(nullptr),
   Solid_Thickness(nullptr),
   OverPressure(nullptr),
   HydroStaticPressure(nullptr),
   Pressure(nullptr),
   Chemical_Compaction(nullptr),
   LithoStaticPressure(nullptr),
   Temperature(nullptr),
   Ves(nullptr),
   Max_VES(nullptr),
   Previous_Real_Thickness_Vector(nullptr),
   Previous_VES(nullptr),
   Previous_Max_VES(nullptr),
   Previous_Depth(nullptr),
   Previous_Solid_Thickness(nullptr),
   Previous_Hydrostatic_Pressure(nullptr),
   Previous_Lithostatic_Pressure(nullptr),
   Previous_Pore_Pressure(nullptr),
   Previous_Overpressure(nullptr),
   Previous_Temperature(nullptr),
   Previous_Chemical_Compaction(nullptr),
   includedNodeVec(nullptr),
   m_flowComponents(nullptr),
   m_previousFlowComponents(nullptr),
   m_immobileComponents(nullptr),
   m_saturations(nullptr),
   m_previousSaturations(nullptr),
   m_transportedMasses(nullptr)
{

  m_nrOfActiveElements = 0;
  depthgridfilename    = "";
  depthgridfileseqnr   = -2;

  destroyCount         = 0;
  createCount          = 0;

  nullify ();
  m_averagedSaturation = nullptr;

  setVectorList();


  Current_Properties.Set_Layer_DA ( &layerDA );
  Current_Properties.Set_Property_Vector ( Basin_Modelling::Depth,                &Depth );
  Current_Properties.Set_Property_Vector ( Basin_Modelling::Real_Thickness,       &Real_Thickness_Vector );
  Current_Properties.Set_Property_Vector ( Basin_Modelling::Solid_Thickness,      &Solid_Thickness );
  Current_Properties.Set_Property_Vector ( Basin_Modelling::Hydrostatic_Pressure, &HydroStaticPressure );
  Current_Properties.Set_Property_Vector ( Basin_Modelling::Lithostatic_Pressure, &LithoStaticPressure );
  Current_Properties.Set_Property_Vector ( Basin_Modelling::Overpressure,         &OverPressure );
  Current_Properties.Set_Property_Vector ( Basin_Modelling::Pore_Pressure,        &Pressure );
  Current_Properties.Set_Property_Vector ( Basin_Modelling::VES_FP,               &Ves );
  Current_Properties.Set_Property_Vector ( Basin_Modelling::Max_VES,              &Max_VES );
  Current_Properties.Set_Property_Vector ( Basin_Modelling::Temperature,          &Temperature );
  Current_Properties.Set_Property_Vector ( Basin_Modelling::Chemical_Compaction,  &Chemical_Compaction );

  Previous_Properties.Set_Layer_DA ( &layerDA );
  Previous_Properties.Set_Property_Vector ( Basin_Modelling::Depth,                &Previous_Depth );
  Previous_Properties.Set_Property_Vector ( Basin_Modelling::Real_Thickness,       &Previous_Real_Thickness_Vector );
  Previous_Properties.Set_Property_Vector ( Basin_Modelling::Solid_Thickness,      &Previous_Solid_Thickness );
  Previous_Properties.Set_Property_Vector ( Basin_Modelling::Hydrostatic_Pressure, &Previous_Hydrostatic_Pressure );
  Previous_Properties.Set_Property_Vector ( Basin_Modelling::Lithostatic_Pressure, &Previous_Lithostatic_Pressure );
  Previous_Properties.Set_Property_Vector ( Basin_Modelling::Overpressure,         &Previous_Overpressure );
  Previous_Properties.Set_Property_Vector ( Basin_Modelling::Pore_Pressure,        &Previous_Pore_Pressure );
  Previous_Properties.Set_Property_Vector ( Basin_Modelling::VES_FP,               &Previous_VES );
  Previous_Properties.Set_Property_Vector ( Basin_Modelling::Max_VES,              &Previous_Max_VES );
  Previous_Properties.Set_Property_Vector ( Basin_Modelling::Temperature,          &Previous_Temperature );
  Previous_Properties.Set_Property_Vector ( Basin_Modelling::Chemical_Compaction,  &Previous_Chemical_Compaction );

  FCTCorrection    = PETSC_IGNORE;
  allochthonousLithologyMap = PETSC_IGNORE;

  vesInterpolator = nullptr;
  maxVesInterpolator = nullptr;

  chemicalCompactionVesValueIsDefined = false;
  chemicalCompactionVesValue = 0.0;

  m_genexData = nullptr;

  m_molarMass.zero ();


}

void LayerProps::setElementInvariants () {

   int i;
   int j;
   int k;

   const MapElementArray& mapElements = FastcauldronSimulator::getInstance ().getMapElementArray ();

   for ( i = mapElements.firstI ( true ); i <= mapElements.lastI ( true ); ++i ) {

      for ( j = mapElements.firstJ ( true ); j <= mapElements.lastJ ( true ); ++j ) {

         const MapElement& e = mapElements ( i, j );

         for ( k = 0; k < getMaximumNumberOfElements (); ++k ) {
            m_elements ( i, j, k ).setPosition ( i, j, k );
            m_elements ( i, j, k ).setFormation ( this );
            m_elements ( i, j, k ).setIsOnProcessor ( mapElements ( i, j ).isOnProcessor ());
            m_elements ( i, j, k ).setIsActive ( false );

            if ( mapElements ( i, j ).isOnProcessor ()) {
               // Notice the 'reflected' local-k node numbering. See volume-element documentation.
               m_elements ( i, j, k ).setNodePosition ( 0, i,     j,     k + 1 );
               m_elements ( i, j, k ).setNodePosition ( 1, i + 1, j,     k + 1 );
               m_elements ( i, j, k ).setNodePosition ( 2, i + 1, j + 1, k + 1 );
               m_elements ( i, j, k ).setNodePosition ( 3, i,     j + 1, k + 1 );
               m_elements ( i, j, k ).setNodePosition ( 4, i,     j,     k );
               m_elements ( i, j, k ).setNodePosition ( 5, i + 1, j,     k );
               m_elements ( i, j, k ).setNodePosition ( 6, i + 1, j + 1, k );
               m_elements ( i, j, k ).setNodePosition ( 7, i,     j + 1, k );

               if ( not mapElements ( i, j ).isOnDomainBoundary ( MapElement::Front )) {
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::Front, &m_elements ( i, j - 1, k ));
               }

               if ( not mapElements ( i, j ).isOnDomainBoundary ( MapElement::Right )) {
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::Right, &m_elements ( i + 1, j, k ));
               }

               if ( not mapElements ( i, j ).isOnDomainBoundary ( MapElement::Back )) {
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::Back, &m_elements ( i, j + 1, k ));
               }

               if ( not mapElements ( i, j ).isOnDomainBoundary ( MapElement::Left )) {
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::Left, &m_elements ( i - 1, j, k ));
               }

               // Other element neighbours (those between formations) will be connected later.
               if ( k > 0 ) {
                  // Add element below to bottom face.
                  // The elments (in the thickness of the layer) are numbered 0 = deepest, n == shallowest.
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::DeepFace, &m_elements ( i, j, k - 1 ));
               }

               if ( k < getMaximumNumberOfElements () - 1 ) {
                  // Add element above to top face.
                  // The elments (in the thickness of the layer) are numbered 0 = deepest, n == shallowest.
                  m_elements ( i, j, k ).setNeighbour ( VolumeData::ShallowFace, &m_elements ( i, j, k + 1 ));
               }

            }

         }

      }

   }

}

void LayerProps::setLayerElements () {

   ElementVolumeGrid&  elementGrid = getVolumeGrid ( 1 );

   m_elements.create ( elementGrid.getDa ());
   setElementInvariants ();
}


void LayerProps::initialise () {

   bool includedInDarcySimulation = FastcauldronSimulator::getInstance ().getMcfHandler ().solveFlowEquations ();

   setLayerElements ();

   if ( m_record != nullptr ) {

      layername      = Interface::Formation::getName ();

      // Crust and mantle layers are not include in the darcy domain.
      includedInDarcySimulation = includedInDarcySimulation and ( not isCrust () and not isMantle ());

      TopSurfaceName = Interface::Formation::getTopSurfaceName ();
      Hydro_Sand     = Interface::Formation::hasConstrainedOverpressure ();
      IsSourceRock   = Interface::Formation::isSourceRock ();
      IsMobile       = Interface::Formation::isMobileLayer ();

      presentDayThickness = Interface::Formation::getInputThicknessMap ();
      depthGridMap = Interface::Formation::getTopSurface ()->getInputDepthMap ();

      if ( Interface::Formation::getTopSurface ()->getSnapshot () != nullptr ) {
         depoage = Interface::Formation::getTopSurface ()->getSnapshot ()->getTime ();
      } else {
         depoage = -2.0;
      }

      TopSurface_DepoSeq = Interface::Formation::getDepositionSequence ();
      Layer_Depo_Seq_Nb = Interface::Formation::getDepositionSequence ();
      Calculate_Chemical_Compaction = Interface::Formation::hasChemicalCompaction ();

      m_lithoMixModel = Interface::Formation::getMixModelStr ();
      m_lithoLayeringIndex = Interface::Formation::getLayeringIndex();

      fluid = (FluidType*)Interface::Formation::getFluidType ();

      if ( includedInDarcySimulation ) {
         m_componentLayerVolumes.construct ( FastcauldronSimulator::getInstance ().getElementGrid (),
                                             getMaximumNumberOfElements (),
                                             NumberOfPVTComponents );

         DMCreateGlobalVector ( m_componentLayerVolumes.getDa (), &m_flowComponents );
         VecZeroEntries ( m_flowComponents );

         DMCreateGlobalVector ( m_componentLayerVolumes.getDa (), &m_previousFlowComponents );
         VecZeroEntries ( m_previousFlowComponents );


         // Set immobile component grid and vector
         m_immobilesLayerGrid.construct ( FastcauldronSimulator::getInstance ().getElementGrid (),
                                          getMaximumNumberOfElements (),
                                          ImmobileSpeciesValues::NumberOfImmobileSpecies );

         DMCreateGlobalVector ( m_immobilesLayerGrid.getDa (), &m_immobileComponents );
         VecZeroEntries ( m_immobileComponents );

         // Set saturation grid and vector
         m_saturationGrid.construct ( FastcauldronSimulator::getInstance ().getElementGrid (),
                                      getMaximumNumberOfElements (),
                                      Saturation::NumberOfPhases );
         DMCreateGlobalVector ( m_saturationGrid.getDa (), &m_saturations );
         DMCreateGlobalVector ( m_saturationGrid.getDa (), &m_previousSaturations );

         VecZeroEntries ( m_saturations );
         VecZeroEntries ( m_previousSaturations );

         PetscBlockVector<Saturation> saturations;
         PetscBlockVector<Saturation> previousSaturations;

         saturations.setVector ( m_saturationGrid, getPhaseSaturationVec (), INSERT_VALUES );
         previousSaturations.setVector ( m_saturationGrid, getPreviousPhaseSaturationVec (), INSERT_VALUES );

         //for time of invasion
         m_timeOfElementInvasionGrid.construct ( FastcauldronSimulator::getInstance ().getElementGrid (),
                                                 getMaximumNumberOfElements (),
                                                 1 );
         DMCreateGlobalVector ( m_timeOfElementInvasionGrid.getDa (), &m_timeOfElementInvasionVec );
         VecSet ( m_timeOfElementInvasionVec, CauldronNoDataValue );
         PetscBlockVector<double> timeOfElementInvasion;
         timeOfElementInvasion.setVector (m_timeOfElementInvasionGrid, getTimeOfElementInvasionVec(), INSERT_VALUES );

         int i;
         int j;
         int k;

         for ( i = m_saturationGrid.firstI (); i <= m_saturationGrid.lastI (); ++i ) {

            for ( j = m_saturationGrid.firstJ (); j <= m_saturationGrid.lastJ (); ++j ) {

               if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

                  for ( k = m_saturationGrid.firstK (); k <= m_saturationGrid.lastK (); ++k ) {
                     saturations ( k, j, i ).initialise ();
                     previousSaturations ( k, j, i ).initialise ();
                  }

               }

            }

         }

         saturations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
         previousSaturations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );

         m_averagedSaturationGrid.construct ( FastcauldronSimulator::getInstance ().getNodalGrid (),
                                              getMaximumNumberOfElements () + 1,
                                              Saturation::NumberOfPhases );

         DMCreateGlobalVector ( m_averagedSaturationGrid.getDa (), &m_averagedSaturation );


         m_elementFluxGrid.construct ( FastcauldronSimulator::getInstance ().getElementGrid (),
                                       getMaximumNumberOfElements (),
                                       ElementFaceValues::NumberOfFaces );

         // Allocate the vector containing the transported masses.
         ElementVolumeGrid& elementGrid = getVolumeGrid ( 1 );

         DMCreateGlobalVector ( elementGrid.getDa (), &m_transportedMasses );
         VecSet ( m_transportedMasses, 0.0 );

      } // if includedInDarcySimulation

   } else {
      layername         = "";
      TopSurfaceName    = "";
      Hydro_Sand        = false;
      IsSourceRock      = false;
      IsMobile          = false;

      presentDayThickness           = nullptr;
      depthGridMap                  = nullptr;
      depoage                       = -2.0;
      TopSurface_DepoSeq            = int (IbsNoDataValue);
      Layer_Depo_Seq_Nb             = int (IbsNoDataValue);
      m_lithoMixModel               = "";
      fluid                         = nullptr;
      Calculate_Chemical_Compaction = false;

   }

   if ( getMaximumNumberOfElements () > 0 )
   {
      DM FCT_DA;

      FastcauldronSimulator::DACreate3D ( getMaximumNumberOfElements() + 1, FCT_DA );
      fracturedPermeabilityScaling.create ( FCT_DA );
      fracturedPermeabilityScaling.fill ( 0.0 );

      pressureExceedsFracturePressure.create ( FCT_DA );
      pressureExceedsFracturePressure.fill ( false );

      nodeIsTemporarilyDirichlet.create ( FCT_DA );
      nodeIsTemporarilyDirichlet.fill ( false );

      preFractureScaling.create ( FCT_DA );
      preFractureScaling.fill ( 0.0 );

      DMDestroy( &FCT_DA );

      // Add scalar volume-grid.
      createVolumeGrid ( 1 );
  }

   int cmp;
   ComponentId species;

   for ( cmp = 0; cmp < NumberOfPVTComponents; ++cmp ) {
      species = ComponentId ( cmp );
      // Which value to use for the gorm?
      //m_molarMass ( species ) = pvtFlash::EosPack::getInstance ().getMolWeight ( cmp, 1.0 );
     m_molarMass ( species ) = pvtFlash::EosPack::getInstance ().getMolWeightLumped( cmp, 1.0 );
   }

   m_molarMass ( ComponentId::ASPHALTENE    ) = 7.979050e+02;
   m_molarMass ( ComponentId::RESIN         ) = 6.105592e+02;
   m_molarMass ( ComponentId::C15_PLUS_ARO    ) = 4.633910e+02;
   m_molarMass ( ComponentId::C15_PLUS_SAT    ) = 2.646560e+02;
   m_molarMass ( ComponentId::C6_MINUS_14ARO  ) = 1.564148e+02;
   m_molarMass ( ComponentId::C6_MINUS_14SAT  ) = 1.025200e+02;
   m_molarMass ( ComponentId::C5            ) = 7.215064e+01;
   m_molarMass ( ComponentId::C4            ) = 5.812370e+01;
   m_molarMass ( ComponentId::C3            ) = 4.409676e+01;
   m_molarMass ( ComponentId::C2            ) = 3.006982e+01;
   m_molarMass ( ComponentId::C1            ) = 1.604288e+01;
   m_molarMass ( ComponentId::COX           ) = 4.400980e+01;
   m_molarMass ( ComponentId::N2            ) = 2.801352e+01;
   m_molarMass ( ComponentId::H2S           ) = 3.407999e+01;
   m_molarMass ( ComponentId::LSC           ) = 2.646560e+02;
   m_molarMass ( ComponentId::C15_PLUS_AT     ) = 2.646560e+02;
   m_molarMass ( ComponentId::C6_MINUS_14BT   ) = 1.564147e+02;
   m_molarMass ( ComponentId::C6_MINUS_14DBT  ) = 1.564147e+02;
   m_molarMass ( ComponentId::C6_MINUS_14BP   ) = 1.564147e+02;
   m_molarMass ( ComponentId::C15_PLUS_ARO_S   ) = 2.646560e+02;
   m_molarMass ( ComponentId::C15_PLUS_SAT_S   ) = 2.646560e+02;
   m_molarMass ( ComponentId::C6_MINUS_14SAT_S ) = 1.564147e+02;
   m_molarMass ( ComponentId::C6_MINUS_14ARO_S ) = 1.564147e+02;

   if ( isSourceRock ()) {
      initialiseSourceRockProperties ( false );
   }

}

void LayerProps::connectElements ( LayerProps* layerAbove ) {

   if ( layerAbove == nullptr ) {
      // Nothing to do here!
      return;
   }

   int i;
   int j;
   int k;

   const int numberOfLowerElements = getMaximumNumberOfElements () - 1;

   const ElementGrid& grid = FastcauldronSimulator::getInstance ().getElementGrid ();

   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {
         m_elements ( i, j, numberOfLowerElements ).setNeighbour ( VolumeData::ShallowFace, &layerAbove->m_elements ( i, j, 0 ));
         layerAbove->m_elements ( i, j, 0 ).setNeighbour ( VolumeData::DeepFace, &m_elements ( i, j, numberOfLowerElements ));
      }

   }

}


LayerProps::~LayerProps(){

   if ( depthvec != nullptr ) {
      Destroy_Petsc_Vector(depthvec);
   }

  Constrained_Property::iterator cp_iter;

  for ( cp_iter = Constrained_Overpressure.begin(); cp_iter != Constrained_Overpressure.end(); cp_iter++ )
  {
    delete ( *cp_iter );
  }

  Constrained_Overpressure.clear();

  if ( m_genexData != nullptr ) {
     delete m_genexData;
     m_genexData = nullptr;
  }

  if ( layerDA != nullptr )  DMDestroy( &layerDA );

  Destroy_Petsc_Vector ( Lithology_ID );
  Destroy_Petsc_Vector ( FCTCorrection );
  Destroy_Petsc_Vector ( Diffusivity );
  Destroy_Petsc_Vector ( Velocity );
  Destroy_Petsc_Vector ( Reflectivity );
  Destroy_Petsc_Vector ( Sonic );
  Destroy_Petsc_Vector ( BulkDensity );
  Destroy_Petsc_Vector ( ThCond );
  Destroy_Petsc_Vector ( Vre );
  Destroy_Petsc_Vector ( layerThickness );
  Destroy_Petsc_Vector ( Thickness_Error );
  Destroy_Petsc_Vector ( erosionFactor );
  Destroy_Petsc_Vector ( faultElements );
  Destroy_Petsc_Vector ( allochthonousLithologyMap );

  Destroy_Petsc_Vector ( Real_Thickness_Vector );
  Destroy_Petsc_Vector ( Solid_Thickness );
  Destroy_Petsc_Vector ( Depth );
  Destroy_Petsc_Vector ( OverPressure );
  Destroy_Petsc_Vector ( HydroStaticPressure );
  Destroy_Petsc_Vector ( Pressure );
  Destroy_Petsc_Vector ( LithoStaticPressure );
  Destroy_Petsc_Vector ( Ves );
  Destroy_Petsc_Vector ( Max_VES );
  Destroy_Petsc_Vector ( Temperature );

  Destroy_Petsc_Vector ( Previous_Real_Thickness_Vector );
  Destroy_Petsc_Vector ( Previous_Solid_Thickness );
  Destroy_Petsc_Vector ( Previous_Depth );
  Destroy_Petsc_Vector ( Previous_Overpressure );
  Destroy_Petsc_Vector ( Previous_Hydrostatic_Pressure );
  Destroy_Petsc_Vector ( Previous_Pore_Pressure );
  Destroy_Petsc_Vector ( Previous_Lithostatic_Pressure );
  Destroy_Petsc_Vector ( Previous_VES );
  Destroy_Petsc_Vector ( Previous_Max_VES );
  Destroy_Petsc_Vector ( Previous_Temperature );

  Destroy_Petsc_Vector ( Chemical_Compaction );
  Destroy_Petsc_Vector ( Previous_Chemical_Compaction );

  Destroy_Petsc_Vector ( Porosity );
  Destroy_Petsc_Vector ( PermeabilityV );
  Destroy_Petsc_Vector ( PermeabilityH );
  Destroy_Petsc_Vector ( BulkDensXHeatCapacity );
  Destroy_Petsc_Vector ( BulkTHCondN );
  Destroy_Petsc_Vector ( BulkTHCondP );
  Destroy_Petsc_Vector ( BulkHeatProd );

  Destroy_Petsc_Vector ( Computed_Deposition_Thickness );

  PetscBool includedInDarcySimulation;
  VecValid ( m_flowComponents, &includedInDarcySimulation);
  if ( includedInDarcySimulation ) {
     Destroy_Petsc_Vector ( m_transportedMasses );
     Destroy_Petsc_Vector ( m_flowComponents );
     Destroy_Petsc_Vector ( m_previousFlowComponents );
     Destroy_Petsc_Vector ( m_immobileComponents );
     Destroy_Petsc_Vector ( m_saturations );
     Destroy_Petsc_Vector ( m_previousSaturations );
     Destroy_Petsc_Vector ( m_timeOfElementInvasionVec );
     Destroy_Petsc_Vector ( m_averagedSaturation );
  }

  if ( vesInterpolator != nullptr ) {
    delete vesInterpolator;
    vesInterpolator = nullptr;
    delete maxVesInterpolator;
    maxVesInterpolator = nullptr;
  }


  // Now delete all allocated element-volume-grids.
   size_t i;

   for ( i = 0; i < m_elementVolumeGrids.size (); ++i )
   {
         delete m_elementVolumeGrids [ i ];
      m_elementVolumeGrids [ i ] = nullptr;
      }

   for ( i = 0; i < m_nodalVolumeGrids.size (); ++i )
   {
      delete m_nodalVolumeGrids[i];
      m_nodalVolumeGrids[i] = nullptr;
   }

}

//------------------------------------------------------------//

#undef __FUNCT__
#define __FUNCT__ "LayerProps::initialiseTemperature"

void LayerProps::initialiseTemperature ( AppCtx* basinModel, const double Current_Time ) {

    int I, J, K;
    int X_Start;
    int Y_Start;
    int Z_Start;
    int X_Count;
    int Y_Count;
    int Z_Count;

    Previous_Properties.Activate_Property ( Basin_Modelling::Temperature );
    Current_Properties.Activate_Property  ( Basin_Modelling::Temperature );

    const Boolean2DArray& Valid_Needle = basinModel->getValidNeedles ();

    DMDAGetCorners ( layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );

    for ( I = X_Start; I < X_Start + X_Count; I++ ) {
       for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {
          if ( Valid_Needle ( I, J )) {
             double Estimated_Temperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, Current_Time );

             for ( K = Z_Start + Z_Count - 1; K >=  Z_Start; K-- ) {

                Current_Properties ( Basin_Modelling::Temperature, K, J, I ) = Estimated_Temperature;
                Previous_Properties ( Basin_Modelling::Temperature, K, J, I ) = Estimated_Temperature;
             }
          }
       }
    }
   Previous_Properties.Restore_Property ( Basin_Modelling::Temperature );
   Current_Properties.Restore_Property  ( Basin_Modelling::Temperature );

}

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "LayerProps::initialisePreviousFluidPressures"

void LayerProps::initialisePreviousFluidPressures( AppCtx* basinModel, const double Current_Time )
{

   int I, J, K;
   int X_Start;
   int Y_Start;
   int Z_Start;
   int X_Count;
   int Y_Count;
   int Z_Count;

   Previous_Properties.Activate_Property( Basin_Modelling::Hydrostatic_Pressure );
   Previous_Properties.Activate_Property( Basin_Modelling::Pore_Pressure );
   const Boolean2DArray& Valid_Needle = basinModel->getValidNeedles();

   DMDAGetCorners( layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );

   for ( I = X_Start; I < X_Start + X_Count; I++ ) {
      for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {
         if ( Valid_Needle( I, J ) && fluid ) {
            double seaTemperature = FastcauldronSimulator::getInstance().getSeaBottomTemperature( I, J, Current_Time );
            double seaBottomDepth = FastcauldronSimulator::getInstance().getSeaBottomDepth( I, J, Current_Time );
            double Estimated_HydrostaticPressure;
            GeoPhysics::computeHydrostaticPressure( fluid, seaTemperature, seaBottomDepth, Estimated_HydrostaticPressure );
            for ( K = Z_Start + Z_Count - 1; K >= Z_Start; K-- ) {
               Previous_Properties( Basin_Modelling::Hydrostatic_Pressure, K, J, I ) = Estimated_HydrostaticPressure;
               Previous_Properties( Basin_Modelling::Pore_Pressure, K, J, I ) = Estimated_HydrostaticPressure;
            }
         }
      }
   }
   Previous_Properties.Restore_Property( Basin_Modelling::Hydrostatic_Pressure );
   Previous_Properties.Restore_Property( Basin_Modelling::Pore_Pressure );

}

//---------------------------------------------------------------//
#undef __FUNCT__
#define __FUNCT__ "LayerProps::allocateNewVecs"

bool LayerProps::allocateNewVecs ( AppCtx* basinModel, const double Current_Time ) {

  int ierr;
  int numberOfZNodes = getNrOfActiveElements() + 1;

  //If layerDA Exists, Check if Needs Reallocation or Deletion (when fully eroded)
  //Skip Non Active Layers

  /// If no layerDA has been allocated yet, then this must be allocated and
  /// the property vectors allocated also. The layer must also be active.
  if (layerDA == nullptr && isActive()) {
    ierr = FastcauldronSimulator::DACreate3D ( numberOfZNodes, layerDA );

    /// Only if we are in loosely calculation mode do we need to allocate these interpolators.
    if ( basinModel->IsCalculationCoupled ) {

      if ( isSediment () && vesInterpolator == nullptr ) {

         if ( FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE ) {
          vesInterpolator    = new ConstantSnapshotInterpolator ( 0.0, 1 );
          maxVesInterpolator = new ConstantSnapshotInterpolator ( 0.0, 1 );
        } else {
          // Number of minor snapshots + 2, to get the total number of snapshots in the snapshot interval (including the end points).
          vesInterpolator    = new LinearSnapshotInterpolator ( 0.0, basinModel->projectSnapshots.maximumNumberOfMinorSnapshots () + 2 );
          maxVesInterpolator = new LinearSnapshotInterpolator ( 0.0, basinModel->projectSnapshots.maximumNumberOfMinorSnapshots () + 2 );
        }

        vesInterpolator->setPropertyNames ( layername, "Ves" );
        vesInterpolator->setDA ( layerDA );

        maxVesInterpolator->setPropertyNames ( layername, "MaxVes" );
        maxVesInterpolator->setDA ( layerDA );
      }

    }

    createVec ( Real_Thickness_Vector );
    createVec ( Solid_Thickness );
    createVec ( Depth );
    setVec ( Depth, CauldronNoDataValue );

    createVec ( OverPressure );
    createVec ( HydroStaticPressure );
    createVec ( Pressure );
    createVec ( LithoStaticPressure );
    createVec ( Ves );
    createVec ( Max_VES );
    createVec ( Temperature );


    setVec(Max_VES,Zero);
    setVec(Ves,Zero);

    createVec ( Previous_Real_Thickness_Vector );
    createVec ( Previous_Solid_Thickness );
    createVec ( Previous_Depth );
    createVec ( Previous_Overpressure );
    createVec ( Previous_Hydrostatic_Pressure );
    createVec ( Previous_Pore_Pressure );
    createVec ( Previous_Lithostatic_Pressure );
    createVec ( Previous_VES );
    createVec ( Previous_Max_VES );
    createVec ( Previous_Temperature );

    initialisePreviousFluidPressures( basinModel, Current_Time );
    initialiseTemperature( basinModel, Current_Time );

    createVec ( Chemical_Compaction );
    createVec ( Previous_Chemical_Compaction );

    setVec ( Chemical_Compaction, Zero );
    setVec ( Previous_Chemical_Compaction, Zero );

    createVec ( Porosity );
    createVec ( PermeabilityV );
    createVec ( PermeabilityH );
    createVec ( BulkDensXHeatCapacity );
    createVec ( BulkTHCondN );
    createVec ( BulkTHCondP );
    createVec ( BulkHeatProd );

    if( basinModel -> isALC() ) {
       allocateBasementVecs( );
    }

  }
  return true;
}

double LayerProps::calcDiffDensity ( const unsigned int i, const unsigned int j ) const {

   double lithodens, fluiddens, densityDifference = 0.0;

  lithodens = getLithology ( i,j ) -> density();

  if ( fluid != nullptr )
  {
    fluiddens = fluid->getConstantDensity();
    if ( lithodens > fluiddens ) densityDifference = lithodens - fluiddens;
  }
  else
  {
     densityDifference = lithodens;
  }

  return densityDifference;
}

void LayerProps::setNrOfActiveElements ( const int a_nrActElem ) {
   m_nrOfActiveElements = a_nrActElem;
}

bool LayerProps::createVec(Vec& propertyVector){

   //cerr<<&propertyVector<<endl;
  assert(nullptr == propertyVector);
  createCount++;
  int ierr = DMCreateGlobalVector(layerDA, &(propertyVector));
  CHKERRQ(ierr);

  // return value is only here because of the CHKERRQ
  return true;
}

bool LayerProps::destroyDA(DM& propertyDA){

  assert(nullptr != propertyDA);
  int ierr = DMDestroy( &propertyDA );
  CHKERRQ(ierr);
  propertyDA = nullptr;

  // return value is only here because of the CHKERRQ
  return true;
}

bool LayerProps::setVec(Vec& propertyVector, const double propertyValue){

  assert(nullptr != propertyVector);
  int ierr = VecSet(propertyVector, propertyValue);
  CHKERRQ(ierr);

  // return value is only here because of the CHKERRQ
  return true;
}

int  LayerProps::Get_TimeIo_DepoSeq ( const SurfacePosition Position,
              const bool            Property_Is_A_Vector ) {

  int Surface_Depo_Seq;

  if ( Position == TOPSURFACE) {

    Surface_Depo_Seq = TopSurface_DepoSeq;

  } else {

    Surface_Depo_Seq = BottSurface_DepoSeq;

  }

  if ( Property_Is_A_Vector ) {

    return ( Layer_Depo_Seq_Nb * 1000 ) + Surface_Depo_Seq;

  } else {

    return Surface_Depo_Seq;

  }

}

void LayerProps::print() {

  cout << layername << " ";
  cout << m_nrOfActiveElements << " ";
  cout << depthgridfilename << " ";
  cout << depthgridfileseqnr << " ";
  cout << depoage << " ";
  cout << Layer_Depo_Seq_Nb << endl;
}

bool LayerProps::propagateVec(DM from_da, DM to_da, Vec from_vec, Vec to_vec)
{
  if ((from_da == nullptr) ||
      (to_da == nullptr) ||
      (from_vec == nullptr) ||
      (to_vec == nullptr)) {
    return false;
  }

  int ierr;
  int from_xs, from_ys, from_zs, from_xm, from_ym, from_zm;
  int to_xs, to_ys, to_zs, to_xm, to_ym, to_zm;
  double ***from_array, ***to_array;

  ierr = DMDAGetCorners(from_da,
                        &from_xs, &from_ys, &from_zs,
                        &from_xm, &from_ym, &from_zm);
  CHKERRQ(ierr);
  ierr = DMDAGetCorners(to_da,
                        &to_xs, &to_ys, &to_zs,
                        &to_xm, &to_ym, &to_zm);
  CHKERRQ(ierr);

  // Check sizes -> PETSC_ASSERT if not ok!! Don't check Z counts as we are
  // chaning that
  PETSC_ASSERT(from_xs == to_xs);  PETSC_ASSERT(from_ys == to_ys);
  PETSC_ASSERT(from_xm == to_xm);  PETSC_ASSERT(from_ym == to_ym);
  PETSC_ASSERT(from_zs == to_zs);

  // Get the arrays
  ierr = DMDAVecGetArray(from_da, from_vec,  &from_array);
  CHKERRQ(ierr);
  ierr = DMDAVecGetArray(to_da, to_vec,  &to_array);
  CHKERRQ(ierr);

  // Boundary for k-index
  int kmax = (from_zm > to_zm) ? to_zm : from_zm;


  for (int i = from_xs; i < from_xs+from_xm; i++)
    for (int j = from_ys; j < from_ys+from_ym; j++)
      //Changed `k = from_zm' to `k = from_zs' OFM
      for (int k = from_zs; k < from_zs+kmax; k++)
  to_array[k][j][i] = from_array[k][j][i];

  ierr = DMDAVecRestoreArray(from_da, from_vec,  &from_array);
  CHKERRQ(ierr);
  ierr = DMDAVecRestoreArray(to_da, to_vec,  &to_array);
  CHKERRQ(ierr);

  return true;
}

#undef __FUNCT__
#define __FUNCT__ "LayerProps::Create_FC_Thickness_Polyfunction"

void LayerProps::Create_FC_Thickness_Polyfunction ( const DM& Map_DA ) {

  unsigned int Number_Of_Segments;

  // Is there any need for this now? Since the function getMaximumNumberOfElements
  // returns a non-negative integer.
  Number_Of_Segments = NumericFunctions::Maximum<unsigned int> ( 0, getMaximumNumberOfElements ());

  if ( Number_Of_Segments > 0 )
  {
    DM FCT_DA;

    // Set here temporarily
    FastcauldronSimulator::DACreate3D ( Number_Of_Segments + 1, FCT_DA );
    fracturedPermeabilityScaling.create ( FCT_DA );
    fracturedPermeabilityScaling.fill ( 0 );

    pressureExceedsFracturePressure.create ( FCT_DA );
    pressureExceedsFracturePressure.fill ( 0 );

    nodeIsTemporarilyDirichlet.create ( FCT_DA );
    nodeIsTemporarilyDirichlet.fill ( false );

    preFractureScaling.create ( FCT_DA );
    preFractureScaling.fill ( 0.0 );

    DMDestroy( &FCT_DA );

  }

}


void LayerProps::setConstrainedOverpressureInterval ( const double startTime,
                                                      const double endTime,
                                                      const double constrainedOverpressureValue ) {

  Boundary_Set* Boundary = new Boundary_Set;

  Boundary -> Start_Time     = startTime;
  Boundary -> End_Time       = endTime;
  Boundary -> Boundary_Value = constrainedOverpressureValue;

  Constrained_Overpressure.push_back( Boundary );

}

void LayerProps::getConstrainedOverpressure ( const double Time, double& Value, bool& Is_Constrained ) const
{
  size_t index;

  for ( index = 0; index < Constrained_Overpressure.size (); index++ )
  {

    if ( ( Time <= Constrained_Overpressure[ index ] -> Start_Time ) &&
         ( Time >= Constrained_Overpressure[ index ] -> End_Time ) )
    {
      Value = Constrained_Overpressure[ index ] -> Boundary_Value;
      Is_Constrained = true;
      return;
    }
  }

  Is_Constrained = false;
}

void LayerProps::nullify (){

   layername         = "";
   TopSurfaceName    = "";
   Hydro_Sand        = false;
   IsSourceRock      = false;
   IsMobile          = false;

   presentDayThickness           = nullptr;
   depthGridMap                  = nullptr;
   depoage                       = -2.0;
   TopSurface_DepoSeq            = int(IbsNoDataValue);
   Layer_Depo_Seq_Nb             = int(IbsNoDataValue);
   m_lithoMixModel               = "";
   fluid                         = nullptr;
   Calculate_Chemical_Compaction = false;


  layerDA               = nullptr;

  depthvec              = PETSC_IGNORE;

  Depth                 = nullptr;
  Porosity              = nullptr;
  Ves                   = nullptr;
  Max_VES               = nullptr;

  HydroStaticPressure   = nullptr;
  LithoStaticPressure   = nullptr;
  Pressure              = nullptr;
  OverPressure          = nullptr;
  Previous_Overpressure = nullptr;
  Previous_Hydrostatic_Pressure = nullptr;
  Previous_Lithostatic_Pressure = nullptr;
  Previous_Pore_Pressure = nullptr;
  Previous_Temperature  = nullptr;
  Previous_Depth        = nullptr;

  Real_Thickness_Vector          = nullptr;
  Previous_Real_Thickness_Vector = nullptr;

  Solid_Thickness               = nullptr;
  Previous_Solid_Thickness      = nullptr;
  Computed_Deposition_Thickness = nullptr;
  Previous_VES                  = nullptr;
  Previous_Max_VES              = nullptr;

  Chemical_Compaction = nullptr;
  Previous_Chemical_Compaction = nullptr;

  PermeabilityV = nullptr;
  PermeabilityH = nullptr;

  Temperature           = nullptr;
  BulkDensXHeatCapacity = nullptr;
  BulkTHCondN           = nullptr;
  BulkTHCondP           = nullptr;
  BulkHeatProd          = nullptr;
  Lithology_ID          = nullptr;

  m_flowComponents = nullptr;
  m_previousFlowComponents = nullptr;
  m_saturations = nullptr;
  m_timeOfElementInvasionVec=nullptr;
}

void LayerProps::initialiseSourceRockProperties ( const bool printInitialisationDetails ) {

   if ( isSourceRock ()) {

      double SomeLargeValue = 1.0e10;

      // The map could be filled with any value since it is over-written anyway with the correct "Boolean" value.
      Interface::GridMap* nodeIsValid = getProjectHandle().getFactory ()->produceGridMap ( 0, 0,
                                                                                         getProjectHandle().getActivityOutputGrid (),
                                                                                         10.0 );
      Interface::GridMap* vre = getProjectHandle().getFactory ()->produceGridMap ( 0, 0,
                                                                                 getProjectHandle().getActivityOutputGrid (),
                                                                                 SomeLargeValue );


      int i;
      int j;

      nodeIsValid->retrieveData ();
      vre->retrieveData ();

      m_genexData = FastcauldronSimulator::getInstance ().getFactory ()->produceGridMap ( 0, 0,
                                                                                          FastcauldronSimulator::getInstance ().getLowResolutionOutputGrid (),
                                                                                          99999.0,
                                                                                          NumberOfPVTComponents);

      for ( i = FastcauldronSimulator::getInstance ().firstI (); i <= FastcauldronSimulator::getInstance ().lastI (); ++i ) {

         for ( j = FastcauldronSimulator::getInstance ().firstJ (); j <= FastcauldronSimulator::getInstance ().lastJ (); ++j ) {
            bool isValid = FastcauldronSimulator::getInstance ().getNodeIsValid ( i, j );
            nodeIsValid->setValue ( i, j, isValid );
         }

      }

      GeoPhysics::GeoPhysicsSourceRock* sourceRock = (GeoPhysicsSourceRock*)(getSourceRock1 ());
      sourceRock->clear ();
      sourceRock->setFormationData ( this );
      sourceRock->initialiseNodes ();
      sourceRock->initialize ( printInitialisationDetails );
      sourceRock->preprocess ( nodeIsValid, vre, printInitialisationDetails );
      sourceRock->addHistoryToNodes ();

      delete nodeIsValid;
      delete vre;

   }
}


void LayerProps::reInitialise (){

   if ( layerDA != nullptr ) {
      DMDestroy( &layerDA );
      layerDA = nullptr;
   }

   initialiseSourceRockProperties ( false );

   Destroy_Petsc_Vector ( Real_Thickness_Vector );
   Destroy_Petsc_Vector ( Solid_Thickness );
   Destroy_Petsc_Vector ( Depth );
   Destroy_Petsc_Vector ( OverPressure );
   Destroy_Petsc_Vector ( HydroStaticPressure );
   Destroy_Petsc_Vector ( Pressure );
   Destroy_Petsc_Vector ( LithoStaticPressure );
   Destroy_Petsc_Vector ( Ves );
   Destroy_Petsc_Vector ( Max_VES );
   Destroy_Petsc_Vector ( Temperature );


   Destroy_Petsc_Vector ( Previous_Real_Thickness_Vector );
   Destroy_Petsc_Vector ( Previous_Solid_Thickness );
   Destroy_Petsc_Vector ( Previous_Depth );
   Destroy_Petsc_Vector ( Previous_Overpressure );
   Destroy_Petsc_Vector ( Previous_Hydrostatic_Pressure );
   Destroy_Petsc_Vector ( Previous_Pore_Pressure );
   Destroy_Petsc_Vector ( Previous_Lithostatic_Pressure );
   Destroy_Petsc_Vector ( Previous_VES );
   Destroy_Petsc_Vector ( Previous_Max_VES );
   Destroy_Petsc_Vector ( Previous_Temperature );

   Destroy_Petsc_Vector ( Chemical_Compaction );
   Destroy_Petsc_Vector ( Previous_Chemical_Compaction );


   Destroy_Petsc_Vector ( Computed_Deposition_Thickness );

   Destroy_Petsc_Vector ( Porosity );
   Destroy_Petsc_Vector ( PermeabilityV );
   Destroy_Petsc_Vector ( PermeabilityH );
   Destroy_Petsc_Vector ( BulkDensXHeatCapacity );
   Destroy_Petsc_Vector ( BulkTHCondN );
   Destroy_Petsc_Vector ( BulkTHCondP );
   Destroy_Petsc_Vector ( BulkHeatProd );

   if( isBasement() ) {
     reInitialiseBasementVecs();
   }

   if ( FastcauldronSimulator::getInstance ().getMcfHandler ().solveFlowEquations ()) {
      PetscBool isValid;

      VecValid ( m_flowComponents, &isValid );

      if ( isValid ) {
         VecZeroEntries ( m_flowComponents );
      }

      VecValid ( m_previousFlowComponents, &isValid );

      if ( isValid ) {
         VecZeroEntries ( m_previousFlowComponents );
      }

      VecValid ( m_saturations, &isValid );

      if ( isValid ) {
         PetscBlockVector<Saturation> saturations;
         PetscBlockVector<Saturation> previousSaturations;

         saturations.setVector ( m_saturationGrid, m_saturations, INSERT_VALUES );
         previousSaturations.setVector ( m_saturationGrid, m_previousSaturations, INSERT_VALUES );

         int i;
         int j;
         int k;

         for ( i = m_saturationGrid.firstI (); i <= m_saturationGrid.lastI (); ++i ) {

            for ( j = m_saturationGrid.firstJ (); j <= m_saturationGrid.lastJ (); ++j ) {

               if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

                  for ( k = m_saturationGrid.firstK (); k <= m_saturationGrid.lastK (); ++k ) {
                     saturations ( k, j, i ).initialise ();
                     previousSaturations ( k, j, i ).initialise ();
                  }

               }

            }

         }

         saturations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
         previousSaturations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
      }

   }

}

void LayerProps::setVectorList() {

  vectorList.VecArray[DEPTH] = &Depth;                  Depth = nullptr;
  vectorList.VecArray[VES] = &Ves;                      Ves = nullptr;
  vectorList.VecArray[MAXVES] = &Max_VES;               Max_VES = nullptr;
  vectorList.VecArray[TEMPERATURE] = &Temperature;      Temperature = nullptr;
  vectorList.VecArray[POROSITYVEC] = &Porosity;         Porosity = nullptr;
  vectorList.VecArray[DIFFUSIVITYVEC] = &Diffusivity;   Diffusivity = nullptr;
  vectorList.VecArray[VR] = &Vre;                       Vre = nullptr;
  vectorList.VecArray[BULKDENSITYVEC] = &BulkDensity;   BulkDensity = nullptr;
  vectorList.VecArray[VELOCITYVEC] = &Velocity;         Velocity = nullptr;
  vectorList.VecArray[SONICVEC] = &Sonic;               Sonic = nullptr;
  vectorList.VecArray[REFLECTIVITYVEC] = &Reflectivity; Reflectivity = nullptr;
  vectorList.VecArray[PERMEABILITYVEC] = &PermeabilityV; PermeabilityV = nullptr;
  vectorList.VecArray[PERMEABILITYHVEC] = &PermeabilityH; PermeabilityH = nullptr;
  vectorList.VecArray[THCONDVEC] = &BulkTHCondN;        BulkTHCondN = nullptr;
  vectorList.VecArray[PRESSURE] = &Pressure;            Pressure = nullptr;
  vectorList.VecArray[OVERPRESSURE] = &OverPressure;    OverPressure = nullptr;
  vectorList.VecArray[HYDROSTATICPRESSURE] = &HydroStaticPressure;
  HydroStaticPressure = nullptr;
  vectorList.VecArray[LITHOSTATICPRESSURE] = &LithoStaticPressure;
  LithoStaticPressure = nullptr;
  vectorList.VecArray[FCTCORRECTION] = &FCTCorrection;  FCTCorrection = nullptr;
  vectorList.VecArray[THICKNESSERROR] = &Thickness_Error;     Thickness_Error = nullptr;
  vectorList.VecArray[FAULTELEMENTS]  = &faultElements;       faultElements = nullptr;
  vectorList.VecArray[THICKNESS]      = &layerThickness;      layerThickness = nullptr;
  vectorList.VecArray[ALLOCHTHONOUS_LITHOLOGY]  = &allochthonousLithologyMap;  allochthonousLithologyMap = nullptr;
  vectorList.VecArray[EROSIONFACTOR] = &erosionFactor;     erosionFactor = nullptr;

  vectorList.VecArray [ CHEMICAL_COMPACTION ] = &Chemical_Compaction;
  Chemical_Compaction = nullptr;
}

//------------------------------------------------------------//

void LayerProps::setLayerElementActivity ( const double age ) {

   const FastcauldronSimulator& fastcauldron = FastcauldronSimulator::getInstance ();
   const MapElementArray&       mapElements  = fastcauldron.getMapElementArray ();

   unsigned int i;
   unsigned int j;
   unsigned int k;

   bool activeSegment1;
   bool activeSegment2;
   bool activeSegment3;
   bool activeSegment4;

   for ( i = mapElements.firstI ( true ); i <= mapElements.lastI ( true ); ++i ) {

      for ( j = mapElements.firstJ ( true ); j <= mapElements.lastJ ( true ); ++j ) {
         const MapElement& mapElement = mapElements ( i, j );

         if ( mapElement.isValid () and isActive ()) {

            for ( k = 0; k < getMaximumNumberOfElements (); ++k ) {

               LayerElement& volumeElement = getLayerElement ( i, j, k );

               if ( mapElement.isOnProcessor ()) {

                  // Can mapElement.getNodeIPosition be replaced with i, or i +1? same for J.
                  activeSegment1 = getDepositingThickness ( mapElement.getNodeIPosition ( 0 ), mapElement.getNodeJPosition ( 0 ), k, age ) > DepositingThicknessTolerance;
                  activeSegment2 = getDepositingThickness ( mapElement.getNodeIPosition ( 1 ), mapElement.getNodeJPosition ( 1 ), k, age ) > DepositingThicknessTolerance;
                  activeSegment3 = getDepositingThickness ( mapElement.getNodeIPosition ( 2 ), mapElement.getNodeJPosition ( 2 ), k, age ) > DepositingThicknessTolerance;
                  activeSegment4 = getDepositingThickness ( mapElement.getNodeIPosition ( 3 ), mapElement.getNodeJPosition ( 3 ), k, age ) > DepositingThicknessTolerance;

                  // if any segment is active then the element is active.
                  if ( activeSegment1 or activeSegment2 or activeSegment3 or activeSegment4 ) {
                     volumeElement.setIsActive ( true );

                     // Set face activity.
                     volumeElement.setIsActiveBoundary ( VolumeData::ShallowFace, true );
                     volumeElement.setIsActiveBoundary ( VolumeData::DeepFace, true );
                     volumeElement.setIsActiveBoundary ( VolumeData::Front, activeSegment1 or activeSegment2 );
                     volumeElement.setIsActiveBoundary ( VolumeData::Right, activeSegment2 or activeSegment3 );
                     volumeElement.setIsActiveBoundary ( VolumeData::Back,  activeSegment3 or activeSegment4 );
                     volumeElement.setIsActiveBoundary ( VolumeData::Left,  activeSegment4 or activeSegment1 );

                  } else {
                     volumeElement.setIsActive ( false );
                  }

               } else {

                  // Here we need only check to see if the face is active.
                  // If the face is active then the element must be active.
                  // If the face is not active then there can be no transport between
                  // the two elements.

                  if ( i > mapElements.lastI ( false )) {

                     if ( NumericFunctions::inRange<unsigned int> ( j, mapElements.firstJ ( false ), mapElements.lastJ ( false ))) {

                        activeSegment1 = getDepositingThickness ( i, j,     k, age ) > DepositingThicknessTolerance;
                        activeSegment2 = getDepositingThickness ( i, j + 1, k, age ) > DepositingThicknessTolerance;

                        volumeElement.setIsActive ( activeSegment1 or activeSegment2 );
                        volumeElement.setIsActiveBoundary ( VolumeData::Left, activeSegment1 or activeSegment2 );
                     }

                  } else if ( mapElements.firstI ( false ) != 0 and i < mapElements.firstI ( false )) {

                     if ( NumericFunctions::inRange<unsigned int> ( j, mapElements.firstJ ( false ), mapElements.lastJ ( false ))) {

                        activeSegment1 = getDepositingThickness ( i, j,     k, age ) > DepositingThicknessTolerance;
                        activeSegment2 = getDepositingThickness ( i, j + 1, k, age ) > DepositingThicknessTolerance;

                        volumeElement.setIsActive ( activeSegment1 or activeSegment2 );
                        volumeElement.setIsActiveBoundary ( VolumeData::GAMMA_3, activeSegment1 or activeSegment2 );
                     }

                  }

                  if ( j > mapElements.lastJ ( false )) {

                     if ( NumericFunctions::inRange<unsigned int> ( i, mapElements.firstI ( false ), mapElements.lastI ( false ))) {

                        activeSegment1 = getDepositingThickness ( i,     j, k, age ) > DepositingThicknessTolerance;
                        activeSegment2 = getDepositingThickness ( i + 1, j, k, age ) > DepositingThicknessTolerance;

                        volumeElement.setIsActive ( activeSegment1 or activeSegment2 );
                        volumeElement.setIsActiveBoundary ( VolumeData::GAMMA_4, activeSegment1 or activeSegment2 );
                     }


                  } else if ( mapElements.firstJ ( false ) != 0 and j < mapElements.firstJ ( false )) {

                     if ( NumericFunctions::inRange<unsigned int> ( i, mapElements.firstI ( false ), mapElements.lastI ( false ))) {

                        activeSegment1 = getDepositingThickness ( i,     j, k, age ) > DepositingThicknessTolerance;
                        activeSegment2 = getDepositingThickness ( i + 1, j, k, age ) > DepositingThicknessTolerance;

                        volumeElement.setIsActive ( activeSegment1 or activeSegment2 );
                        volumeElement.setIsActiveBoundary ( VolumeData::GAMMA_4, activeSegment1 or activeSegment2 );
                     }

                  }

               }


            }

         } else {

            // The whole column of elements is inactive.
            for ( k = 0; k < getMaximumNumberOfElements (); ++k ) {
               LayerElement& volumeElement = getLayerElement ( i, j, k );

               volumeElement.setIsActive ( false );
            }

         }

      }

   }

}

//------------------------------------------------------------//

double LayerProps::getDepositingThickness ( const unsigned int i,
                                            const unsigned int j,
                                            const unsigned int k,
                                            const double       age ) const {

   if (( FastcauldronSimulator::getInstance ().getCalculationMode () == PRESSURE_AND_TEMPERATURE_MODE or
         FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURE_MODE or
         FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURED_TEMPERATURE_MODE or
         FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_DARCY_MODE ) and
       not FastcauldronSimulator::getInstance ().getCauldron ()->isGeometricLoop ()) {
      return getRealThickness ( i, j, k, age );
   } else {
      return getSolidThickness ( i, j, k, age );
   }

}

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "LayerProps::Fill_Topmost_Segment_Array"

void LayerProps::Fill_Topmost_Segment_Array ( const double                  Required_Age,
                                              const bool                    Use_Solid_Thickness,
                                              const Boolean2DArray&         Valid_Needle,
                                                    PETSc_2D_Integer_Array& Topmost_Segments,
                                                    int&                    Topmost_Segment ) const {


  int X_Start;
  int X_Count;
  int Y_Start;
  int Y_Count;
  int Z_Start;
  int Z_Count;

  int I, J, K;

  double Thickness;

  Topmost_Segment = -1;

  DMDAGetGhostCorners ( layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );

  for ( I = X_Start; I < X_Start + X_Count; I++ ) {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

      if ( Valid_Needle ( I, J )) {

        for ( K = Z_Start + Z_Count - 2; K >= Z_Start; K-- ) {

          if ( Use_Solid_Thickness ) {
            Thickness = m_solidThickness ( I, J, K ).F ( Required_Age );
          } else {
            Thickness = m_realThickness ( I, J, K ).F ( Required_Age );
          }

          if ( Thickness > 0.0 ) {
            Topmost_Segment = Integer_Max ( Topmost_Segment, K );
            Topmost_Segments ( I, J ) = K;
            break;
          }

        }

      }

    }

  }

}
//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "LayerProps::Fill_Topmost_Segment_Arrays"

void LayerProps::Fill_Topmost_Segment_Arrays ( const double          Previous_Time,
                                               const double          Current_Time,
                                               const bool            Use_Solid_Thickness,
                                               const Boolean2DArray& Valid_Needle ) {

  int Current_Topmost_Segment = 0;
  int Previous_Topmost_Segment = 0;

  if ( layerDA == nullptr ) {
    Previous_Topmost_Segments.fill ( -3 );
    Current_Topmost_Segments.fill ( -3 );
  } else {
    Fill_Topmost_Segment_Array ( Previous_Time, Use_Solid_Thickness, Valid_Needle, Previous_Topmost_Segments, Previous_Topmost_Segment );
    Fill_Topmost_Segment_Array ( Current_Time,  Use_Solid_Thickness, Valid_Needle, Current_Topmost_Segments,  Current_Topmost_Segment );
    //
    //
    // Is it possible here, to find the maximum of Current_Topmost_Segment on
    // ALL the processes? If so, could this be used as the number of elements
    // (in the z-direction) for the layer?
    //
  }

}


//------------------------------------------------------------//

void LayerProps::setFaultElementsMap ( AppCtx*         basinModel,
                                       const Boolean2DArray& validNeedle ) {

  int I;
  int J;
  int K;

  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;

  PETSC_3D_Array faultElementsMap;

  DMDAGetCorners ( layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

  DMCreateGlobalVector ( layerDA, &faultElements );
  VecSet ( faultElements, CauldronNoDataValue );

  faultElementsMap.Set_Global_Array( layerDA, faultElements, INSERT_VALUES );

  for ( I = xStart; I < xStart + xCount; I++ ) {

    for ( J = yStart; J < yStart + yCount; J++ ) {

      if ( validNeedle ( I, J )) {

        if ( getLithology ( I, J )->isFault ()) {

          for ( K = zStart; K < zStart + zCount; K++ ) {
            faultElementsMap ( K, J, I ) = 1.0;
          }

        } else {

          for ( K = zStart; K < zStart + zCount; K++ ) {
            faultElementsMap ( K, J, I ) = 0.0;
          }

        }

      }

    }

  }

//    faultElementsMap.Restore_Global_Array ( Update_Excluding_Ghosts );
}

//------------------------------------------------------------//

void LayerProps::setErosionFactorMap ( AppCtx*         basinModel,
                                       const Boolean2DArray& validNeedle,
                                       const double    currentAge ) {

  int i;
  int j;
  int k;

  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;

  // Time-wise it would be better to save a deposition-thickness
  // map immediately after the layer has been deposited rather
  // than compute each time this function is called.
  double depositionThickness;
  double currentThickness;

  // Should be 2d but due to limitations in the output mechanism, this must be a 3d array.
  PETSC_3D_Array erosionFactorMap;

  DMDAGetCorners ( layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

  DMCreateGlobalVector ( layerDA, &erosionFactor );
  VecSet ( erosionFactor, CauldronNoDataValue );

  erosionFactorMap.Set_Global_Array ( layerDA, erosionFactor, INSERT_VALUES, false );

  for ( i = xStart; i < xStart + xCount; ++i ) {

    for ( j = yStart; j < yStart + yCount; ++j ) {

      if ( validNeedle ( i, j )) {
        depositionThickness = 0.0;
        currentThickness = 0.0;

        // If the currentAge > depoage then the layer is, either, in the process
        // of being deposited, or has not started to be deposited yet.
        if ( currentAge < depoage ) {

          if ( basinModel->isGeometricLoop ()) {

            for ( k = zStart; k < zStart + zCount - 1; ++k ) {
              depositionThickness += getSolidThickness ( i, j, k, depoage );
              currentThickness += getSolidThickness ( i, j, k, currentAge );
            }

          } else {

            for ( k = zStart; k < zStart + zCount - 1; ++k ) {
              depositionThickness += getRealThickness ( i, j, k, depoage );
              currentThickness += getRealThickness ( i, j, k, currentAge );
            }

          }

        } else {
          currentThickness = 1.0;
          depositionThickness = 1.0;
        }

        // Since the erosionFactor vector is a 3d array and the output requires a map only
        // (and the output function takes only the top-most layer of the array), set the
        // top-most layer of the array to the erosion-factor value.
        if ( depositionThickness != 0.0 ) {
          erosionFactorMap ( zCount - 1, j, i ) = currentThickness / depositionThickness;
        } else {
          erosionFactorMap ( zCount - 1, j, i ) = 1.0;
        }

      }

    }

  }

}

//------------------------------------------------------------//

void LayerProps::switchLithologies ( const double age ) {

   // First perform any lithology switching.
   GeoPhysics::GeoPhysicsFormation::switchLithologies ( age );

   // Now ready to set volume-elements with new lithologies
   int i;
   int j;
   int k;

   const ElementGrid&     grid = FastcauldronSimulator::getInstance ().getElementGrid ();
   const MapElementArray& mapElements = FastcauldronSimulator::getInstance ().getMapElementArray ();

   for ( i = grid.firstI ( true ); i <= grid.lastI ( true ); ++i ) {

      for ( j = grid.firstJ ( true ); j <= grid.lastJ ( true ); ++j ) {

         for ( k = 0; k < getMaximumNumberOfElements (); ++k ) {

            if ( mapElements ( i, j ).isValid ()) {
               m_elements ( i, j, k ).setLithology ( dynamic_cast<const Lithology*>(getCompoundLithology ( i, j )));
            }

         }

      }

   }



}

//------------------------------------------------------------//

void LayerProps::setAllochthonousLithologyMap ( AppCtx*         basinModel,
                                                const Boolean2DArray& validNeedle ) {

  int I;
  int J;
  int K;

  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;

  PETSC_3D_Array allochthonousLithologies;

  DMDAGetGhostCorners ( layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

  DMCreateGlobalVector ( layerDA, &allochthonousLithologyMap );
  VecSet ( allochthonousLithologyMap, CauldronNoDataValue );

  allochthonousLithologies.Set_Global_Array( layerDA, allochthonousLithologyMap, INSERT_VALUES );

  for ( I = xStart; I < xStart + xCount; I++ ) {

    for ( J = yStart; J < yStart + yCount; J++ ) {

      if ( validNeedle ( I, J )) {

        if ( m_compoundLithologies.isAllochthonousLithology ( I, J )) {

          for ( K = zStart; K < zStart + zCount; K++ ) {
            allochthonousLithologies ( K, J, I ) = 1.0;
          }

        } else {

          for ( K = zStart; K < zStart + zCount; K++ ) {
            allochthonousLithologies ( K, J, I ) = 0.0;
          }

        }

      }

    }

  }

}

//------------------------------------------------------------//

void LayerProps::deleteAllochthonousLithologyMap () {

  PetscBool validVector;

  VecValid ( allochthonousLithologyMap, &validVector );

  if ( validVector ) {
    VecDestroy ( &allochthonousLithologyMap );
    allochthonousLithologyMap = Vec ( 0 );
  }

}

//------------------------------------------------------------//

void LayerProps::deleteFaultElementsMap () {

   PetscBool validVector ( faultElements != nullptr ? PETSC_TRUE : PETSC_FALSE );

  if ( validVector ) {
    VecDestroy ( &faultElements );
    faultElements = Vec ( 0 );
  }

}

//------------------------------------------------------------//

void LayerProps::deleteErosionFactorMap () {

  PetscBool validVector ( erosionFactor != nullptr ? PETSC_TRUE : PETSC_FALSE );

  if ( validVector ) {
    VecDestroy ( &erosionFactor );
    erosionFactor = Vec ( 0 );
  }

}

//------------------------------------------------------------//

void LayerProps::setSnapshotInterval ( const SnapshotInterval& interval,
                                             AppCtx*          basinModel ) {


   // If the vesInterpolator != 0 then the maxVesInterpolator will also != 0.
   if ( isSediment () && vesInterpolator != nullptr ) {
      vesInterpolator->setInterval ( interval, basinModel->getOutputDirectory (), layername, "Ves" );
      maxVesInterpolator->setInterval ( interval, basinModel->getOutputDirectory (), layername, "MaxVes" );
   }

}

//------------------------------------------------------------//

void LayerProps::interpolateProperty ( AppCtx*               basinModel,
                                       const double          currentTime,
                                       DM                    propertyDA,
                                       SnapshotInterpolator& interpolator,
                                       Vec                   propertyVector ) {

  int I;
  int J;
  int K;

  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;


  PETSC_3D_Array propertyArray ( propertyDA, propertyVector );

  interpolator.setCurrentTime ( currentTime );

  DMDAGetCorners ( propertyDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

  for ( K = zStart; K < zStart + zCount; K++ ) {

     for ( I = xStart; I < xStart + xCount; I++ ) {

        for ( J = yStart; J < yStart + yCount; J++ ) {

           if ( basinModel->nodeIsDefined ( I, J )) {
              propertyArray ( K, J, I ) = interpolator ( I, J, K );
           } else {
              propertyArray ( K, J, I ) = CauldronNoDataValue;
           }

        }

     }

  }

}

//------------------------------------------------------------//

void LayerProps::interpolateProperty (       AppCtx*                  basinModel,
                                       const double                   currentTime,
                                       const bool                     doingHighResDecompaction,
                                       const PropertyIdentifier&             property ) {


  if ( ! isSediment () || vesInterpolator == nullptr ) {
    return;
  }

  switch ( property ) {

    case VES :

       interpolateProperty ( basinModel, currentTime, layerDA,
                             *vesInterpolator, Ves );
      break;

    case MAXVES :

       interpolateProperty ( basinModel, currentTime, layerDA,
                             *maxVesInterpolator, Max_VES );

      break;

    default:

      assert ( false );

      // ERROR

  }

}

//------------------------------------------------------------//

double LayerProps::estimateStandardPermeability () const {

   double standardTemperature = 10.0 + StandardDepth / 1000 * StandardTemperatureGradient;
   double standardStress = StandardDepth / 1000.0 * StandardStressGradient * 1.0e6;

   unsigned int i;
   unsigned int j;
   CompoundProperty porosity;
   double permeabilityN;
   double permeabilityH;
   double maximumPermeability = 0.0;
   double globalMaximumPermeability;
   bool valueSet = false;

   for ( i = FastcauldronSimulator::getInstance ().firstI (); i <= FastcauldronSimulator::getInstance ().lastI (); ++i ) {

      for ( j = FastcauldronSimulator::getInstance ().firstJ (); j <= FastcauldronSimulator::getInstance ().lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            getCompoundLithology ( i, j )->getPorosity ( standardStress, standardStress, false, 0.0, porosity );
            getCompoundLithology ( i, j )->calcBulkPermeabilityNP ( standardStress, standardStress, porosity, permeabilityN, permeabilityH );
            maximumPermeability = NumericFunctions::Maximum ( maximumPermeability, permeabilityN );
            valueSet = true;
         }

      }

   }

   if ( not valueSet ) {
      maximumPermeability = -9999.0;
   }

   MPI_Allreduce ( &maximumPermeability, &globalMaximumPermeability, 1, MPI_DOUBLE, MPI_MAX, PETSC_COMM_WORLD );

   return globalMaximumPermeability / Utilities::Maths::MilliDarcyToM2;
}

//------------------------------------------------------------//

void LayerProps::setChemicalCompactionVesValue ( const double newVesValue ) {

  chemicalCompactionVesValueIsDefined = true;
  chemicalCompactionVesValue = newVesValue;

}

//------------------------------------------------------------//

void LayerProps::copyProperties () {
   Previous_Properties.Copy ( Current_Properties,
                              FastcauldronSimulator::getInstance ().getCauldron ()->getValidNeedles ());
}

//------------------------------------------------------------//

void LayerProps::copySaturations () {
   VecCopy ( m_saturations, m_previousSaturations );
}

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "LayerProps::Set_Chemical_Compaction_Mode"

void LayerProps::Set_Chemical_Compaction_Mode ( const bool Mode ) {

  Calculate_Chemical_Compaction = Mode;

}

//------------------------------------------------------------//

void LayerProps::computeThicknessScaling ( const double startTime,
                                           const double endTime,
                                           LinearPropertyInterpolator& thicknessScaling ) {

   double depositionSolidThickness;
   double previousSolidThickness;
   double currentSolidThickness;
   int i;
   int j;
   int k;

   for ( i = FastcauldronSimulator::getInstance ().firstI (); i <= FastcauldronSimulator::getInstance ().lastI (); ++i ) {

      for ( j = FastcauldronSimulator::getInstance ().firstJ (); j <= FastcauldronSimulator::getInstance ().lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

            if ( isMobileLayer ()) {
               thicknessScaling.compute ( i, j, startTime, 1.0, endTime, 1.0 );
            } else {
               depositionSolidThickness = 0.0;
               currentSolidThickness = 0.0;
               previousSolidThickness = 0.0;

               for ( k = 0; k < getMaximumNumberOfElements (); ++k ) {
                  depositionSolidThickness += getDepositingThickness ( i, j, k, 0.0 );
                  currentSolidThickness += getDepositingThickness ( i, j, k, endTime );
                  previousSolidThickness += getDepositingThickness ( i, j, k, startTime );
               }

               if ( depositionSolidThickness != 0.0 ) {
                  thicknessScaling.compute ( i, j,
                                             startTime, previousSolidThickness / depositionSolidThickness,
                                             endTime,   currentSolidThickness  / depositionSolidThickness );
               } else {
                  thicknessScaling.compute ( i, j, startTime, 1.0, endTime, 1.0 );
               }

            }

         }

      }

   }

}

//------------------------------------------------------------//

void LayerProps::computeThicknessScaling ( const double time,
                                           LinearPropertyInterpolator& thicknessScaling ) {

   double depositionSolidThickness;
   double currentSolidThickness;
   int i;
   int j;
   int k;

   for ( i = FastcauldronSimulator::getInstance ().firstI (); i <= FastcauldronSimulator::getInstance ().lastI (); ++i ) {

      for ( j = FastcauldronSimulator::getInstance ().firstJ (); j <= FastcauldronSimulator::getInstance ().lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

            if ( isMobileLayer ()) {
               thicknessScaling.compute ( i, j, time, 1.0 );
            } else {
               depositionSolidThickness = 0.0;
               currentSolidThickness = 0.0;

               for ( k = 0; k < getMaximumNumberOfElements (); ++k ) {
                  depositionSolidThickness += getDepositingThickness ( i, j, k, 0.0 );
                  currentSolidThickness += getDepositingThickness ( i, j, k, time );
               }

               if ( depositionSolidThickness != 0.0 ) {
                  thicknessScaling.compute ( i, j,
                                             time, currentSolidThickness  / depositionSolidThickness );
               } else {
                  thicknessScaling.compute ( i, j, time, 1.0 );
               }

            }

         }

      }

   }

}

//------------------------------------------------------------//

void LayerProps::extractGenexDataInterval ( const double startTime,
                                            const double endTime,
                                            LinearPropertyInterpolator& ves,
                                            LinearPropertyInterpolator& temperature,
                                            LinearPropertyInterpolator& hydrostaticPressure,
                                            LinearPropertyInterpolator& lithostaticPressure,
                                            LinearPropertyInterpolator& porePressure,
                                            LinearPropertyInterpolator& porosity,
                                            LinearPropertyInterpolator& permeability,
                                            LinearPropertyInterpolator& vre ) {

   Previous_Properties.Activate_Property ( Basin_Modelling::VES_FP );
   Current_Properties.Activate_Property  ( Basin_Modelling::VES_FP );

   Previous_Properties.Activate_Property ( Basin_Modelling::Max_VES );
   Current_Properties.Activate_Property  ( Basin_Modelling::Max_VES );

   Previous_Properties.Activate_Property ( Basin_Modelling::Chemical_Compaction );
   Current_Properties.Activate_Property  ( Basin_Modelling::Chemical_Compaction );

   Previous_Properties.Activate_Property ( Basin_Modelling::Temperature );
   Current_Properties.Activate_Property  ( Basin_Modelling::Temperature );

   Previous_Properties.Activate_Property ( Basin_Modelling::Hydrostatic_Pressure );
   Current_Properties.Activate_Property  ( Basin_Modelling::Hydrostatic_Pressure );

   Previous_Properties.Activate_Property ( Basin_Modelling::Lithostatic_Pressure );
   Current_Properties.Activate_Property  ( Basin_Modelling::Lithostatic_Pressure );

   Previous_Properties.Activate_Property ( Basin_Modelling::Pore_Pressure );
   Current_Properties.Activate_Property  ( Basin_Modelling::Pore_Pressure );

   ves.compute ( layerDA, Basin_Modelling::VES_FP, startTime, Previous_Properties, endTime, Current_Properties );
   temperature.compute ( layerDA, Basin_Modelling::Temperature, startTime, Previous_Properties, endTime, Current_Properties );
   hydrostaticPressure.compute ( layerDA, Basin_Modelling::Hydrostatic_Pressure, startTime, Previous_Properties, endTime, Current_Properties );
   lithostaticPressure.compute ( layerDA, Basin_Modelling::Lithostatic_Pressure, startTime, Previous_Properties, endTime, Current_Properties );
   porePressure.compute ( layerDA, Basin_Modelling::Pore_Pressure, startTime, Previous_Properties, endTime, Current_Properties );


   PETSc_Local_2D_Array<double> porosityArrayStart;
   PETSc_Local_2D_Array<double> porosityArrayEnd;
   PETSc_Local_2D_Array<double> permeabilityArrayStart;
   PETSc_Local_2D_Array<double> permeabilityArrayEnd;
   PETSc_Local_2D_Array<double> vreArrayStart;
   PETSc_Local_2D_Array<double> vreArrayEnd;

   double vesStart;
   double maxVesStart;
   double ccStart;
   double porosityStart;
   double permeabilityStart;
   double vesEnd;
   double maxVesEnd;
   double ccEnd;
   double porosityEnd;
   double permeabilityEnd;
   double dummy; // The un-needed lateral permeability.

   int i;
   int j;
   int kMax = getMaximumNumberOfElements ();
   bool ccRequired = hasChemicalCompaction () and FastcauldronSimulator::getInstance ().getRunParameters ()->getChemicalCompaction ();


   porosityArrayStart.create ( layerDA  );
   porosityArrayEnd.create ( layerDA  );
   permeabilityArrayStart.create ( layerDA  );
   permeabilityArrayEnd.create ( layerDA  );

   vreArrayStart.create ( layerDA  );
   vreArrayEnd.create ( layerDA  );


   for ( i = FastcauldronSimulator::getInstance ().firstI (); i <= FastcauldronSimulator::getInstance ().lastI (); ++i ) {

      for ( j = FastcauldronSimulator::getInstance ().firstJ (); j <= FastcauldronSimulator::getInstance ().lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            vesStart = Previous_Properties ( Basin_Modelling::VES_FP, kMax, j, i );
            maxVesStart = Previous_Properties ( Basin_Modelling::Max_VES, kMax, j, i );
            ccStart = Previous_Properties ( Basin_Modelling::Chemical_Compaction, kMax, j, i );

            porosityStart = getLithology ( i, j )->porosity ( vesStart, maxVesStart, ccRequired, ccStart );
            getLithology ( i, j )->calcBulkPermeabilityNP ( vesStart, maxVesStart, porosityStart, permeabilityStart, dummy );

            vesEnd = Current_Properties ( Basin_Modelling::VES_FP, kMax, j, i );
            maxVesEnd = Current_Properties ( Basin_Modelling::Max_VES, kMax, j, i );
            ccEnd = Current_Properties ( Basin_Modelling::Chemical_Compaction, kMax, j, i );

            porosityEnd = getLithology ( i, j )->porosity ( vesEnd, maxVesEnd, ccRequired, ccEnd );
            getLithology ( i, j )->calcBulkPermeabilityNP ( vesEnd, maxVesEnd, porosityEnd, permeabilityEnd, dummy );

            porosityArrayStart ( i, j ) = porosityStart;
            porosityArrayEnd ( i, j ) = porosityEnd;

            permeabilityArrayStart ( i, j ) = permeabilityStart;
            permeabilityArrayEnd ( i, j ) = permeabilityEnd;

            vreArrayStart ( i, j ) = 0.0;
            vreArrayEnd ( i, j ) = 0.0;
         }

      }

   }

   porosity.compute ( layerDA, startTime, porosityArrayStart, endTime, porosityArrayEnd );
   permeability.compute ( layerDA, startTime, permeabilityArrayStart, endTime, permeabilityArrayEnd );
   vre.compute ( layerDA, startTime, vreArrayStart, endTime, vreArrayEnd );

   Previous_Properties.Restore_Property ( Basin_Modelling::VES_FP );
   Current_Properties.Restore_Property  ( Basin_Modelling::VES_FP );

   Previous_Properties.Restore_Property ( Basin_Modelling::Max_VES );
   Current_Properties.Restore_Property  ( Basin_Modelling::Max_VES );

   Previous_Properties.Restore_Property ( Basin_Modelling::Chemical_Compaction );
   Current_Properties.Restore_Property  ( Basin_Modelling::Chemical_Compaction );

   Previous_Properties.Restore_Property ( Basin_Modelling::Temperature );
   Current_Properties.Restore_Property  ( Basin_Modelling::Temperature );

   Previous_Properties.Restore_Property ( Basin_Modelling::Hydrostatic_Pressure );
   Current_Properties.Restore_Property  ( Basin_Modelling::Hydrostatic_Pressure );

   Previous_Properties.Restore_Property ( Basin_Modelling::Lithostatic_Pressure );
   Current_Properties.Restore_Property  ( Basin_Modelling::Lithostatic_Pressure );

   Previous_Properties.Restore_Property ( Basin_Modelling::Pore_Pressure );
   Current_Properties.Restore_Property  ( Basin_Modelling::Pore_Pressure );

}

//------------------------------------------------------------//

void LayerProps::extractGenexDataInstance ( const double time,
                                            LinearPropertyInterpolator& ves,
                                            LinearPropertyInterpolator& temperature ) {

   Current_Properties.Activate_Property  ( Basin_Modelling::VES_FP );
   Current_Properties.Activate_Property  ( Basin_Modelling::Temperature );

   ves.compute ( layerDA, Basin_Modelling::VES_FP, time, Current_Properties );
   temperature.compute ( layerDA, Basin_Modelling::Temperature, time, Current_Properties );

   Current_Properties.Restore_Property  ( Basin_Modelling::VES_FP );
   Current_Properties.Restore_Property  ( Basin_Modelling::Temperature );
}

//------------------------------------------------------------//

void LayerProps::integrateGenexEquations ( const double previousTime,
                                           const double currentTime ) {

   if ( not isSourceRock () or currentTime > depoage ) {
      return;
   }

   LinearPropertyInterpolator vesInterp;
   LinearPropertyInterpolator temperatureInterp;
   LinearPropertyInterpolator thicknessInterp;

   LinearPropertyInterpolator hydrostaticPressureInterp;
   LinearPropertyInterpolator lithostaticPressureInterp;
   LinearPropertyInterpolator porePressureInterp;
   LinearPropertyInterpolator porosityInterp;
   LinearPropertyInterpolator permeabilityInterp;
   LinearPropertyInterpolator vreInterp;


   GeoPhysics::GeoPhysicsSourceRock* sourceRock = (GeoPhysicsSourceRock*)(getSourceRock1 ());

   thicknessInterp.initialise ( layerDA );

   computeThicknessScaling ( previousTime, currentTime, thicknessInterp );

   extractGenexDataInterval ( previousTime, currentTime,
                              vesInterp,
                              temperatureInterp,
                              hydrostaticPressureInterp,
                              lithostaticPressureInterp,
                              porePressureInterp,
                              porosityInterp,
                              permeabilityInterp,
                              vreInterp );

   sourceRock->computeTimeInterval ( previousTime, currentTime,
                                     &vesInterp,
                                     &temperatureInterp,
                                     &thicknessInterp,
                                     &hydrostaticPressureInterp,
                                     &lithostaticPressureInterp,
                                     &porePressureInterp,
                                     &porosityInterp,
                                     &permeabilityInterp,
                                     &vreInterp,
                                     m_genexData );

}

//------------------------------------------------------------//

void LayerProps::getGenexGenerated ( const int i,
                                     const int j,
                                     PVTComponents& generated ) const {


   if ( isSourceRock ()) {
      unsigned int cmp;
      unsigned int id;
      ComponentId species;
      double gen;

      const GeoPhysics::GeoPhysicsSourceRock* sourceRock = (GeoPhysics::GeoPhysicsSourceRock*)(getSourceRock1 ());
      const Genex6::SpeciesManager& speciesManager = sourceRock->getSpeciesManager ();

      for ( cmp = 0; cmp < NumberOfPVTComponents; ++cmp ) {
         species = ComponentId ( cmp );
         id = speciesManager.mapPvtComponentsToId ( species );

         gen = m_genexData->getValue ( (unsigned int)i, (unsigned int)j, cmp );

         if ( gen == CauldronNoDataValue or gen == IbsNoDataValue ) {
            generated ( species ) = 0.0;
         } else {
            generated ( species ) = gen;
         }


      }

   } else {
      generated.zero ();
   }

}

//------------------------------------------------------------//


void LayerProps::retrieveGenexData () {
   m_genexData->retrieveData ( true );
}

bool LayerProps::genexDataIsRetrieved () const {
   return m_genexData->retrieved ();
}


void LayerProps::restoreGenexData () {
   m_genexData->retrieveData ( false );
}


//------------------------------------------------------------//

void LayerProps::clearGenexOutput () {

   if ( not isSourceRock ()) {
      return;
   }

   GeoPhysics::GeoPhysicsSourceRock* sourceRock = (GeoPhysicsSourceRock*)(getSourceRock1 ());
//    const GeoPhysics::GeoPhysicsSourceRock* sourceRock = dynamic_cast<const GeoPhysicsSourceRock*>(getSourceRock ());

   sourceRock->clearOutputHistory ();
}

//------------------------------------------------------------//

void LayerProps::zeroTransportedMass () {
   VecSet ( m_transportedMasses, 0.0 );
}

//------------------------------------------------------------//

void LayerProps::createVolumeGrid ( const int numberOfDofs ) {
   createVolumeGridSlave ( numberOfDofs );
}

//------------------------------------------------------------//

ElementVolumeGrid& LayerProps::getVolumeGrid ( const int numberOfDofs ) {
   createVolumeGridSlave ( numberOfDofs );
   return *m_elementVolumeGrids [ numberOfDofs - 1 ];
}

//------------------------------------------------------------//

const ElementVolumeGrid& LayerProps::getVolumeGrid ( const int numberOfDofs ) const {
   createVolumeGridSlave ( numberOfDofs );
   return *m_elementVolumeGrids [ numberOfDofs - 1 ];
}

//------------------------------------------------------------//

void LayerProps::createVolumeGridSlave ( const int numberOfDofs ) const {

   // Resize the array if array is too small, filling extra values with the null value.
   if ( m_elementVolumeGrids.size () < numberOfDofs ) {
      m_elementVolumeGrids.resize ( numberOfDofs, 0 );
   }

   // If the element-grid does not exist then create one.
   if ( m_elementVolumeGrids [ numberOfDofs - 1 ] == nullptr ) {
      m_elementVolumeGrids [ numberOfDofs - 1 ] = new ElementVolumeGrid;
      m_elementVolumeGrids [ numberOfDofs - 1 ]->construct ( FastcauldronSimulator::getInstance ().getElementGrid (),
                                                             getMaximumNumberOfElements (),
                                                             numberOfDofs );
   }

}

//------------------------------------------------------------//

void LayerProps::createNodalVolumeGrid ( const int numberOfDofs ) {
   createNodalVolumeGridSlave ( numberOfDofs );
}

//------------------------------------------------------------//

NodalVolumeGrid& LayerProps::getNodalVolumeGrid ( const int numberOfDofs ) {
   createNodalVolumeGridSlave ( numberOfDofs );
   return *m_nodalVolumeGrids [ numberOfDofs - 1 ];
}

//------------------------------------------------------------//

const NodalVolumeGrid& LayerProps::getNodalVolumeGrid ( const int numberOfDofs ) const {
   createNodalVolumeGridSlave ( numberOfDofs );
   return *m_nodalVolumeGrids [ numberOfDofs - 1 ];
}

//------------------------------------------------------------//

void LayerProps::createNodalVolumeGridSlave ( const int numberOfDofs ) const {

   // Resize the array if array is too small, filling extra values with the null value.
   if ( m_nodalVolumeGrids.size () < numberOfDofs ) {
      m_nodalVolumeGrids.resize ( numberOfDofs, 0 );
   }

   // If the nodal-grid does not exist then create one.
   if ( m_nodalVolumeGrids [ numberOfDofs - 1 ] == nullptr ) {
      m_nodalVolumeGrids [ numberOfDofs - 1 ] = new NodalVolumeGrid;
      m_nodalVolumeGrids [ numberOfDofs - 1 ]->construct ( FastcauldronSimulator::getInstance ().getNodalGrid (),
                                                           getMaximumNumberOfElements () + 1,
                                                           numberOfDofs );
   }

}

//------------------------------------------------------------//
