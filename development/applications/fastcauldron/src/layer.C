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

#include "Interface/auxiliaryfaulttypes.h"
#include "Interface/Faulting.h"
#include "Interface/FaultCollection.h"
#include "Interface/FaultElementCalculator.h"
#include "Interface/Snapshot.h"
#include "Interface/Formation.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Surface.h"
#include "Interface/ObjectFactory.h"
#include "Interface/RunParameters.h"

#include "Species.h"
#include "ChemicalModel.h"
#include "Simulator.h"


#include "CompoundLithology.h"
#include "Lithology.h"

#include "ComponentManager.h"

#include "GeoPhysicalConstants.h"
#include "GeoPhysicsSourceRock.h"
#include "GeoPhysicsFormation.h"

#include "PetscBlockVector.h"

#include "ElementFaceValues.h"
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"

#include "ImmobileSpeciesValues.h"

using namespace DataAccess;
using namespace FiniteElementMethod;


#include "NumericFunctions.h"


using Interface::X_COORD;
using Interface::Y_COORD;

//------------------------------------------------------------//

LayerProps::LayerProps ( Interface::ProjectHandle * projectHandle,
                         database::Record *              record ) :
   DataAccess::Interface::Formation ( projectHandle, record ),
   GeoPhysics::Formation ( projectHandle, record )
{

  m_nrOfActiveElements = 0;
  depthgridfilename    = "";
  depthgridfileseqnr   = -2;

  destroyCount         = 0;
  createCount          = 0;

  nullify ();
  m_averagedSaturation = NULL;

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

  FCTCorrection    = PETSC_NULL;
  allochthonousLithologyMap = PETSC_NULL;

  vesInterpolator = 0;
  maxVesInterpolator = 0;

  chemicalCompactionVesValueIsDefined = false;
  chemicalCompactionVesValue = 0.0;

  m_genexData = 0;

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

void LayerProps::initialise () {

   bool includedInDarcySimulation = FastcauldronSimulator::getInstance ().getMcfHandler ().solveFlowEquations ();

   if ( m_record != 0 ) {

      layername      = Interface::Formation::getName ();

      // Crust and mantle layers are not include in the darcy domain.
      includedInDarcySimulation = includedInDarcySimulation and ( not isCrust () and not isMantle ());

      TopSurfaceName = Interface::Formation::getTopSurfaceName ();
      Hydro_Sand     = Interface::Formation::hasConstrainedOverpressure ();
      IsSourceRock   = Interface::Formation::isSourceRock ();
      IsMobile       = Interface::Formation::isMobileLayer ();

      presentDayThickness = Interface::Formation::getInputThicknessMap ();
      depthGridMap = Interface::Formation::getTopSurface ()->getInputDepthMap ();

      if ( Interface::Formation::getTopSurface ()->getSnapshot () != 0 ) {
         depoage = Interface::Formation::getTopSurface ()->getSnapshot ()->getTime ();
      } else {
         depoage = -2.0;
      }

      TopSurface_DepoSeq = Interface::Formation::getDepositionSequence ();
      Layer_Depo_Seq_Nb = Interface::Formation::getDepositionSequence ();
      Calculate_Chemical_Compaction = Interface::Formation::hasChemicalCompaction ();

      lithoMixModel = Interface::Formation::getMixModelStr ();

      fluid = (FluidType*)Interface::Formation::getFluidType ();

      ElementVolumeGrid&  elementGrid = getVolumeGrid ( 1 );
      m_elements.create ( elementGrid.getDa ());
      setElementInvariants ();


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
         VecSet ( m_timeOfElementInvasionVec, CAULDRONIBSNULLVALUE );
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

      // if ( darcySimulation ) {
         // Allocate the vector containing the transported masses.
         ElementVolumeGrid& elementGrid = getVolumeGrid ( 1 );

         DMCreateGlobalVector ( elementGrid.getDa (), &m_transportedMasses );
         VecSet ( m_transportedMasses, 0.0 );
      // }

      } // if includedInDarcySimulation

   } else {
      layername         = "";
      TopSurfaceName    = "";
      Hydro_Sand        = false;
      IsSourceRock      = false;
      IsMobile          = false;

      presentDayThickness = 0;
      depthGridMap = 0;
      depoage = -2.0;
      TopSurface_DepoSeq = int (IBSNULLVALUE);
      Layer_Depo_Seq_Nb = int (IBSNULLVALUE);
      lithoMixModel = "";
      fluid = 0;
      Calculate_Chemical_Compaction = false;

   }

   if ( getMaximumNumberOfElements () > 0 )
   {
      DM FCT_DA;

      FastcauldronSimulator::DACreate3D ( getMaximumNumberOfElements() + 1, FCT_DA );
      fracturedPermeabilityScaling.create ( FCT_DA );
      fracturedPermeabilityScaling.fill ( 0 );

      pressureExceedsFracturePressure.create ( FCT_DA );
      pressureExceedsFracturePressure.fill ( 0 );
      
      nodeIsTemporarilyDirichlet.create ( FCT_DA );
      nodeIsTemporarilyDirichlet.fill ( false );

      preFractureScaling.create ( FCT_DA );
      preFractureScaling.fill ( 0.0 );

      DMDestroy( &FCT_DA );

      // Add scalar volume-grid.
      createVolumeGrid ( 1 );
  }

   int cmp;
   pvtFlash::ComponentId species;
   
   //for ( cmp = 0; cmp < CBMGenerics::ComponentManager::NumberOfSpeciesToFlash; ++cmp ) {
   for ( cmp = 0; cmp < NumberOfPVTComponents; ++cmp ) {
      species = pvtFlash::ComponentId ( cmp );
      // Which value to use for the gorm?
      //m_molarMass ( species ) = pvtFlash::EosPack::getInstance ().getMolWeight ( cmp, 1.0 );
	  m_molarMass ( species ) = pvtFlash::EosPack::getInstance ().getMolWeightLumped( cmp, 1.0 );
   }

   m_molarMass ( pvtFlash::ASPHALTENES ) = 7.979050e+02;
   m_molarMass (      pvtFlash::RESINS ) = 6.105592e+02;
   m_molarMass (     pvtFlash::C15_ARO ) = 4.633910e+02;
   m_molarMass (     pvtFlash::C15_SAT ) = 2.646560e+02;
   m_molarMass (    pvtFlash::C6_14ARO ) = 1.564148e+02;
   m_molarMass (    pvtFlash::C6_14SAT ) = 1.025200e+02;
   m_molarMass (          pvtFlash::C5 ) = 7.215064e+01;
   m_molarMass (          pvtFlash::C4 ) = 5.812370e+01;
   m_molarMass (          pvtFlash::C3 ) = 4.409676e+01;
   m_molarMass (          pvtFlash::C2 ) = 3.006982e+01;
   m_molarMass (          pvtFlash::C1 ) = 1.604288e+01;
   m_molarMass (         pvtFlash::COX ) = 4.400980e+01;
   m_molarMass (          pvtFlash::N2 ) = 2.801352e+01;
   m_molarMass (         pvtFlash::H2S ) = 3.407999e+01;
   m_molarMass (         pvtFlash::LSC ) = 2.646560e+02;
   m_molarMass (      pvtFlash::C15_AT ) = 2.646560e+02;
   m_molarMass (     pvtFlash::C6_14BT ) = 1.564147e+02;
   m_molarMass (    pvtFlash::C6_14DBT ) = 1.564147e+02;
   m_molarMass (     pvtFlash::C6_14BP ) = 1.564147e+02;
   m_molarMass (    pvtFlash::C15_AROS ) = 2.646560e+02;
   m_molarMass (    pvtFlash::C15_SATS ) = 2.646560e+02;
   m_molarMass (   pvtFlash::C6_14SATS ) = 1.564147e+02;
   m_molarMass (   pvtFlash::C6_14AROS ) = 1.564147e+02;
 
   if ( isSourceRock ()) {

      initialiseSourceRockProperties ( false );

#if 0
      double SomeLargeValue = 1.0e10;

      // The map could be filled with any value since it is over-written anyway with the correct "Boolean" value.
      Interface::GridMap* nodeIsValid = m_projectHandle->getFactory ()->produceGridMap ( 0, 0, 
                                                                                         m_projectHandle->getActivityOutputGrid (),
                                                                                         10.0 );
      Interface::GridMap* vre = m_projectHandle->getFactory ()->produceGridMap ( 0, 0, 
                                                                                 m_projectHandle->getActivityOutputGrid (),
                                                                                 SomeLargeValue );

      GeoPhysics::GeoPhysicsSourceRock* sourceRock = (GeoPhysicsSourceRock*)(getSourceRock1 ());

      int i;
      int j;
      // int cmp;
      // pvtFlash::ComponentId species;

      nodeIsValid->retrieveData ();
      vre->retrieveData ();

      m_genexData = FastcauldronSimulator::getInstance ().getFactory ()->produceGridMap ( 0, 0,
                                                                                          FastcauldronSimulator::getInstance ().getLowResolutionOutputGrid (),
                                                                                          99999.0,
                                                                                          NumberOfPVTComponents);
                                                                                          // CBMGenerics::ComponentManager::NumberOfOutputSpecies );

      for ( i = FastcauldronSimulator::getInstance ().firstI (); i <= FastcauldronSimulator::getInstance ().lastI (); ++i ) {

         for ( j = FastcauldronSimulator::getInstance ().firstJ (); j <= FastcauldronSimulator::getInstance ().lastJ (); ++j ) {
            bool isValid = FastcauldronSimulator::getInstance ().getNodeIsValid ( i, j );
            nodeIsValid->setValue ( i, j, isValid );
         }

      }

      sourceRock->setFormationData ( this );
      sourceRock->initialize ( true );
      sourceRock->preprocess ( nodeIsValid, vre, true );

      // for ( cmp = 0; cmp < CBMGenerics::ComponentManager::NumberOfSpeciesToFlash; ++cmp ) {
      //    species = pvtFlash::ComponentId ( cmp );
      //    // Which value to use for the gorm?
      //    m_molarMass ( species ) = pvtFlash::EosPack::getInstance ().getMolWeight ( cmp, 1.0 );

      //    //    const Genex5::Species* gen5Species = sourceRock->getSimulator ().getChemicalModel ().GetSpeciesByName ( CBMGenerics::ComponentManager::getInstance().GetSpeciesName ( cmp ));

      //    //    if ( gen5Species != 0 ) {
      //    //       // m_molarMass ( species ) = gen5Species->ComputeMolWeight (); //GetMolWeight ();

      //    //       // Which value to use for the gorm?
      //    //       m_molarMass ( species ) = pvtFlash::EosPack::getInstance ().getMolWeight ( cmp, 1.0 );
      //    //    } else {
      //    //       // Assign some big number. Should this be an error?
      //    //       m_molarMass ( species ) = 1.0e20;
      //    //    }

      //    // if ( sourceRock->getSimulator ().getChemicalModel ().GetSpeciesById ( cmp ) != 0 ) {
      //    //    m_molarMass ( species ) = sourceRock->getSimulator ().getChemicalModel ().GetSpeciesById ( cmp )->ComputeMolWeight (); //GetMolWeight ();
      //    // } else {
      //    //    m_molarMass ( species ) = 1.0e20;
      //    // }

      // }

      delete nodeIsValid;
      delete vre;
#endif
   }

}

void LayerProps::connectElements ( LayerProps* layerAbove ) {

   if ( layerAbove == 0 or isCrust () or isMantle ()) {
      // Nothing to do here!
      return;
   }

   int i;
   int j;
   int k;

   const int numberOfLowerElements = getMaximumNumberOfElements () - 1;

   const ElementGrid&     grid = FastcauldronSimulator::getInstance ().getElementGrid ();
   // const MapElementArray& mapElements = FastcauldronSimulator::getInstance ().getMapElementArray ();

   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {
         m_elements ( i, j, numberOfLowerElements ).setNeighbour ( VolumeData::ShallowFace, &layerAbove->m_elements ( i, j, 0 ));
         layerAbove->m_elements ( i, j, 0 ).setNeighbour ( VolumeData::DeepFace, &m_elements ( i, j, numberOfLowerElements ));
      }

   }

}


LayerProps::~LayerProps(){

   bool includedInDarcySimulation = FastcauldronSimulator::getInstance ().getMcfHandler ().solveFlowEquations ()
                                    and ( not isCrust () and not isMantle ());

   if ( depthvec != 0 ) {
      Destroy_Petsc_Vector(depthvec);
   }

  Constrained_Property::iterator cp_iter;

  for ( cp_iter = Constrained_Overpressure.begin(); cp_iter != Constrained_Overpressure.end(); cp_iter++ )
  {
    delete ( *cp_iter );
  }

  Constrained_Overpressure.clear();

  if ( m_genexData != 0 ) {
     delete m_genexData;
  }

  if ( layerDA != NULL )  DMDestroy(&layerDA);

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
  Destroy_Petsc_Vector ( BulkDensXHeatCapacity );
  Destroy_Petsc_Vector ( BulkTHCondN );
  Destroy_Petsc_Vector ( BulkTHCondP );
  Destroy_Petsc_Vector ( BulkHeatProd );


  Destroy_Petsc_Vector ( m_SteraneAromatisation );
  Destroy_Petsc_Vector ( m_SteraneIsomerisation );
  Destroy_Petsc_Vector ( m_HopaneIsomerisation ); 

  Destroy_Petsc_Vector ( m_IlliteFraction );

  Destroy_Petsc_Vector ( Computed_Deposition_Thickness );

  Destroy_Petsc_Vector ( m_averagedSaturation );

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

  if ( vesInterpolator != 0 ) {
    delete vesInterpolator;
    delete maxVesInterpolator;
  }


  // Now delete all allocated element-volume-grids.
   size_t i;

   for ( i = 0; i < m_elementVolumeGrids.size (); ++i ) {

      if ( m_elementVolumeGrids [ i ] != 0 ) {
         delete m_elementVolumeGrids [ i ];
      }

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

#undef __FUNCT__  
#define __FUNCT__ "LayerProps::allocateNewVecs"

bool LayerProps::allocateNewVecs ( AppCtx* basinModel, const double Current_Time ) {

  int ierr;
  int numberOfZNodes = getNrOfActiveElements() + 1;

  //If layerDA Exists, Check if Needs Reallocation or Deletion (when fully eroded)
  //Skip Non Active Layers

  /// If no layerDA has been allocated yet, then this must be allocated and 
  /// the property vectors allocated also. The layer must also be active.
  if (layerDA == NULL && isActive()) {
    ierr = FastcauldronSimulator::DACreate3D ( numberOfZNodes, layerDA );

    /// Only if we are in loosely calculation mode do we need to allocate these interpolators.
    if ( basinModel->IsCalculationCoupled ) {

      if ( isSediment () && vesInterpolator == 0 ) {

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
    setVec ( Depth, CAULDRONIBSNULLVALUE );

    createVec ( OverPressure );
    createVec ( HydroStaticPressure );
    createVec ( Pressure );
    createVec ( LithoStaticPressure );
    createVec ( Ves );
    createVec ( Max_VES );
    createVec ( Temperature );
    //setVec(Temperature,CAULDRONIBSNULLVALUE);
   

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

    initialiseTemperature( basinModel, Current_Time );

//     setVec ( Previous_Real_Thickness_Vector, Zero );
//     setVec ( Previous_Solid_Thickness, Zero );
//     setVec ( Previous_Depth, Zero );
//     setVec ( Previous_Overpressure, Zero );
//     setVec ( Previous_Hydrostatic_Pressure, Zero );
//     setVec ( Previous_Pore_Pressure, Zero );
//     setVec ( Previous_Lithostatic_Pressure, Zero );
//     setVec ( Previous_VES, Zero );
//     setVec ( Previous_Max_VES, Zero );
//     setVec ( Previous_Temperature, Zero );


//     setVec ( Previous_Temperature, Zero );
//     setVec ( Previous_VES, Zero );
//     setVec ( Previous_Max_VES, Zero );
    //    setVec( Previous_Temperature,CAULDRONIBSNULLVALUE);

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

    if(  basinModel->isModellingMode1D()  )
    {
       m_BiomarkersState.create(layerDA);
    }
    
    createVec ( m_SteraneAromatisation);
    createVec ( m_SteraneIsomerisation);
    createVec ( m_HopaneIsomerisation );

    if(  basinModel->isModellingMode1D() )
    {
       m_SmectiteIlliteState.create(layerDA);
    }

   createVec ( m_IlliteFraction );

    if ( includedNodes.isNull ()) {
      includedNodes.create ( layerDA );
    }

    if( basinModel -> isALC() ) {
       allocateBasementVecs( );
    }

  }
  return true;
}

double LayerProps::calcDiffDensity ( const unsigned int i, const unsigned int j ) const {

  double lithodens, fluiddens;
  
  lithodens = getLithology ( i,j ) -> density();

  if ( fluid != 0 && getLithology ( i,j ) -> surfacePorosity() != 0.0 ) {
    fluiddens = fluid->getConstantDensity();
  } else {
    fluiddens = 0.0;
  }

  return lithodens - fluiddens;


}

void LayerProps::setNrOfActiveElements ( const int a_nrActElem ) {
   m_nrOfActiveElements = a_nrActElem;
}

bool LayerProps::createVec(Vec& propertyVector){

   //cerr<<&propertyVector<<endl;
  IBSASSERT(NULL == propertyVector);
  createCount++;
  int ierr = DMCreateGlobalVector(layerDA, &(propertyVector));
  CHKERRQ(ierr);

  // return value is only here because of the CHKERRQ
  return true;
}

bool LayerProps::destroyDA(DM& propertyDA){

  IBSASSERT(NULL != propertyDA);
  int ierr = DMDestroy( &propertyDA );
  CHKERRQ(ierr);
  propertyDA = NULL;

  // return value is only here because of the CHKERRQ
  return true;
}

bool LayerProps::setVec(Vec& propertyVector, const double propertyValue){

  IBSASSERT(NULL != propertyVector);
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
  if ((from_da == NULL) || 
      (to_da == NULL) || 
      (from_vec == NULL) || 
      (to_vec == NULL)) {
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
//   Number_Of_Segments  = ( getMaximumNumberOfElements() > 0 ) ? getMaximumNumberOfElements() : 0;

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

void LayerProps::getConstrainedOverpressure ( const double Time, double& Value, bool& Is_Constrained )
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

   presentDayThickness = 0;
   depthGridMap = 0;
   depoage = -2.0;
   TopSurface_DepoSeq = int(IBSNULLVALUE);
   Layer_Depo_Seq_Nb = int(IBSNULLVALUE);
   lithoMixModel = "";
   fluid = 0;
   Calculate_Chemical_Compaction = false;


  layerDA               = NULL;

  depthvec              = PETSC_NULL;

  Depth                 = NULL;
  Porosity              = NULL;
  Ves                   = NULL;
  Max_VES               = NULL;

  HydroStaticPressure   = NULL;
  LithoStaticPressure   = NULL;
  Pressure              = NULL;
  OverPressure          = NULL;
  Previous_Overpressure = NULL;
  Previous_Hydrostatic_Pressure = NULL;
  Previous_Lithostatic_Pressure = NULL;
  Previous_Pore_Pressure = NULL;
  Previous_Temperature  = NULL;
  Previous_Depth        = NULL;

  Real_Thickness_Vector          = 0;
  Previous_Real_Thickness_Vector = 0;

  Solid_Thickness               = 0;
  Previous_Solid_Thickness      = 0;
  Computed_Deposition_Thickness = 0;
  Previous_VES                  = 0;
  Previous_Max_VES              = 0;
//  //    Intermediate_Max_VES          = 0;

  Chemical_Compaction = 0;
  Previous_Chemical_Compaction = 0;

  PermeabilityV = 0;
  PermeabilityH = 0;

  Temperature           = NULL;
  BulkDensXHeatCapacity = NULL;
  BulkTHCondN           = NULL;
  BulkTHCondP           = NULL;
  BulkHeatProd          = NULL;
  Lithology_ID          = NULL;

  m_IlliteFraction = NULL;
  m_SteraneAromatisation = NULL;
  m_SteraneIsomerisation = NULL;
  m_HopaneIsomerisation   = NULL;
  m_flowComponents = NULL;
  m_previousFlowComponents = NULL;
  m_saturations = NULL;
  m_timeOfElementInvasionVec=NULL;

   if( isBasement() ) {
     initialiseBasementVecs();
   }

}

void LayerProps::initialiseSourceRockProperties ( const bool printInitialisationDetails ) {

   if ( isSourceRock ()) {

      double SomeLargeValue = 1.0e10;

      // The map could be filled with any value since it is over-written anyway with the correct "Boolean" value.
      Interface::GridMap* nodeIsValid = m_projectHandle->getFactory ()->produceGridMap ( 0, 0, 
                                                                                         m_projectHandle->getActivityOutputGrid (),
                                                                                         10.0 );
      Interface::GridMap* vre = m_projectHandle->getFactory ()->produceGridMap ( 0, 0, 
                                                                                 m_projectHandle->getActivityOutputGrid (),
                                                                                 SomeLargeValue );


      int i;
      int j;
      // int cmp;
      // pvtFlash::ComponentId species;

      nodeIsValid->retrieveData ();
      vre->retrieveData ();

      m_genexData = FastcauldronSimulator::getInstance ().getFactory ()->produceGridMap ( 0, 0,
                                                                                          FastcauldronSimulator::getInstance ().getLowResolutionOutputGrid (),
                                                                                          99999.0,
                                                                                          NumberOfPVTComponents);
                                                                                          // CBMGenerics::ComponentManager::NumberOfOutputSpecies );

      for ( i = FastcauldronSimulator::getInstance ().firstI (); i <= FastcauldronSimulator::getInstance ().lastI (); ++i ) {

         for ( j = FastcauldronSimulator::getInstance ().firstJ (); j <= FastcauldronSimulator::getInstance ().lastJ (); ++j ) {
            bool isValid = FastcauldronSimulator::getInstance ().getNodeIsValid ( i, j );
            nodeIsValid->setValue ( i, j, isValid );
         }

      }

      GeoPhysics::GeoPhysicsSourceRock* sourceRock = (GeoPhysicsSourceRock*)(getSourceRock1 ());
      // sourceRock->clearSimulator ();
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

   if ( layerDA != NULL ) {
      DMDestroy( &layerDA );
      layerDA = NULL;
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

   Destroy_Petsc_Vector ( m_SteraneAromatisation );
   Destroy_Petsc_Vector ( m_SteraneIsomerisation );
   Destroy_Petsc_Vector ( m_HopaneIsomerisation ); 

   Destroy_Petsc_Vector ( m_IlliteFraction );

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

  vectorList.VecArray[DEPTH] = &Depth;                  Depth = NULL;
  vectorList.VecArray[VES] = &Ves;                      Ves = NULL;
  vectorList.VecArray[MAXVES] = &Max_VES;               Max_VES = NULL;
  vectorList.VecArray[TEMPERATURE] = &Temperature;      Temperature = NULL;
  vectorList.VecArray[POROSITYVEC] = &Porosity;         Porosity = NULL;
  vectorList.VecArray[DIFFUSIVITYVEC] = &Diffusivity;   Diffusivity = NULL;
  vectorList.VecArray[VR] = &Vre;                       Vre = NULL;
  vectorList.VecArray[BULKDENSITYVEC] = &BulkDensity;   BulkDensity = NULL;
  vectorList.VecArray[VELOCITYVEC] = &Velocity;         Velocity = NULL;
  vectorList.VecArray[SONICVEC] = &Sonic;               Sonic = NULL;
  vectorList.VecArray[REFLECTIVITYVEC] = &Reflectivity; Reflectivity = NULL;
  vectorList.VecArray[PERMEABILITYVEC] = &PermeabilityV; PermeabilityV = NULL;
  vectorList.VecArray[PERMEABILITYHVEC] = &PermeabilityH; PermeabilityH = NULL;
  vectorList.VecArray[THCONDVEC] = &BulkTHCondN;        BulkTHCondN = NULL;
  vectorList.VecArray[PRESSURE] = &Pressure;            Pressure = NULL;
  vectorList.VecArray[OVERPRESSURE] = &OverPressure;    OverPressure = NULL;
  vectorList.VecArray[HYDROSTATICPRESSURE] = &HydroStaticPressure; 
  HydroStaticPressure = NULL;
  vectorList.VecArray[LITHOSTATICPRESSURE] = &LithoStaticPressure;
  LithoStaticPressure = NULL;
  vectorList.VecArray[FCTCORRECTION] = &FCTCorrection;  FCTCorrection = NULL;
  vectorList.VecArray[THICKNESSERROR] = &Thickness_Error;     Thickness_Error = NULL;
  vectorList.VecArray[FAULTELEMENTS]  = &faultElements;       faultElements = NULL;
  vectorList.VecArray[THICKNESS]      = &layerThickness;      layerThickness = NULL;
  vectorList.VecArray[ALLOCHTHONOUS_LITHOLOGY]  = &allochthonousLithologyMap;  allochthonousLithologyMap = NULL;
  vectorList.VecArray[EROSIONFACTOR] = &erosionFactor;     erosionFactor = NULL;

  vectorList.VecArray [ CHEMICAL_COMPACTION ] = &Chemical_Compaction;
  Chemical_Compaction = NULL;

  vectorList.VecArray[ILLITEFRACTION]  = &m_IlliteFraction;  m_IlliteFraction = NULL;
  vectorList.VecArray[STERANEAROMATISATION]  = &m_SteraneAromatisation;  m_SteraneAromatisation = NULL;
  vectorList.VecArray[STERANEISOMERISATION]  = &m_SteraneIsomerisation;  m_SteraneIsomerisation = NULL;
  vectorList.VecArray[HOPANEISOMERISATION ]  = &m_HopaneIsomerisation ;  m_HopaneIsomerisation = NULL;

}
void LayerProps::resetSmectiteIlliteStateVectors()
{

   int xs, ys, zs, xm, ym, zm;
   int i, j, k;
   DMDAGetCorners(layerDA,&xs,&ys,&zs,&xm,&ym,&zm);
   for (i = xs; i < xs+xm; i++) 
   {
      for (j = ys; j < ys+ym; j++) 
      {
         for (k = zs; k < zs+zm; k++) 
         {	
            m_SmectiteIlliteState(i,j,k).setAsNotInitialized();
         }
      }
   }
}
void LayerProps::resetBiomarkerStateVectors()
{

   int xs, ys, zs, xm, ym, zm;
   int i, j, k;
   DMDAGetCorners(layerDA,&xs,&ys,&zs,&xm,&ym,&zm);
   for (i = xs; i < xs+xm; i++) 
   {
      for (j = ys; j < ys+ym; j++) 
      {
         for (k = zs; k < zs+zm; k++) 
         {	
            m_BiomarkersState(i,j,k).setAsNotInitialized();
         }
      }
   }
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

   bool geometricLoop = fastcauldron.getCauldron ()->isGeometricLoop ();

   for ( i = mapElements.firstI ( true ); i <= mapElements.lastI ( true ); ++i ) {

      for ( j = mapElements.firstJ ( true ); j <= mapElements.lastJ ( true ); ++j ) {
         const MapElement& mapElement = mapElements ( i, j );

         if ( mapElement.isValid () and isActive ()) {

            for ( k = 0; k < getMaximumNumberOfElements (); ++k ) {

               LayerElement& volumeElement = getLayerElement ( i, j, k );

               if ( mapElement.isOnProcessor ()) {

                  if ( geometricLoop ) {
                     activeSegment1 = getSolidThickness ( mapElement.getNodeIPosition ( 0 ), mapElement.getNodeJPosition ( 0 ), k, age ) > DepositingThicknessTolerance;
                     activeSegment2 = getSolidThickness ( mapElement.getNodeIPosition ( 1 ), mapElement.getNodeJPosition ( 1 ), k, age ) > DepositingThicknessTolerance;
                     activeSegment3 = getSolidThickness ( mapElement.getNodeIPosition ( 2 ), mapElement.getNodeJPosition ( 2 ), k, age ) > DepositingThicknessTolerance;
                     activeSegment4 = getSolidThickness ( mapElement.getNodeIPosition ( 3 ), mapElement.getNodeJPosition ( 3 ), k, age ) > DepositingThicknessTolerance;
                  } else {
                     activeSegment1 = getRealThickness ( mapElement.getNodeIPosition ( 0 ), mapElement.getNodeJPosition ( 0 ), k, age ) > DepositingThicknessTolerance;
                     activeSegment2 = getRealThickness ( mapElement.getNodeIPosition ( 1 ), mapElement.getNodeJPosition ( 1 ), k, age ) > DepositingThicknessTolerance;
                     activeSegment3 = getRealThickness ( mapElement.getNodeIPosition ( 2 ), mapElement.getNodeJPosition ( 2 ), k, age ) > DepositingThicknessTolerance;
                     activeSegment4 = getRealThickness ( mapElement.getNodeIPosition ( 3 ), mapElement.getNodeJPosition ( 3 ), k, age ) > DepositingThicknessTolerance;
                  }

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

                        if ( geometricLoop ) {
                           activeSegment1 = getSolidThickness ( i, j,     k, age ) > DepositingThicknessTolerance;
                           activeSegment2 = getSolidThickness ( i, j + 1, k, age ) > DepositingThicknessTolerance;
                        } else {
                           activeSegment1 = getRealThickness ( i, j,     k, age ) > DepositingThicknessTolerance;
                           activeSegment2 = getRealThickness ( i, j + 1, k, age ) > DepositingThicknessTolerance;
                        }

                        volumeElement.setIsActive ( activeSegment1 or activeSegment1 );
                        volumeElement.setIsActiveBoundary ( VolumeData::GAMMA_5, activeSegment1 or activeSegment2 );
                     } 

                  } else if ( mapElements.firstI ( false ) != 0 and i < mapElements.firstI ( false )) {

                     if ( NumericFunctions::inRange<unsigned int> ( j, mapElements.firstJ ( false ), mapElements.lastJ ( false ))) {

                        if ( geometricLoop ) {
                           activeSegment1 = getSolidThickness ( i, j,     k, age ) > DepositingThicknessTolerance;
                           activeSegment2 = getSolidThickness ( i, j + 1, k, age ) > DepositingThicknessTolerance;
                        } else {
                           activeSegment1 = getRealThickness ( i, j,     k, age ) > DepositingThicknessTolerance;
                           activeSegment2 = getRealThickness ( i, j + 1, k, age ) > DepositingThicknessTolerance;
                        }

                        volumeElement.setIsActive ( activeSegment1 or activeSegment1 );
                        volumeElement.setIsActiveBoundary ( VolumeData::GAMMA_3, activeSegment1 or activeSegment2 );
                     } 

                  }

                  if ( j > mapElements.lastJ ( false )) {

                     if ( NumericFunctions::inRange<unsigned int> ( i, mapElements.firstI ( false ), mapElements.lastI ( false ))) {

                        if ( geometricLoop ) {
                           activeSegment1 = getSolidThickness ( i,     j, k, age ) > DepositingThicknessTolerance;
                           activeSegment2 = getSolidThickness ( i + 1, j, k, age ) > DepositingThicknessTolerance;
                        } else {
                           activeSegment1 = getRealThickness ( i,     j, k, age ) > DepositingThicknessTolerance;
                           activeSegment2 = getRealThickness ( i + 1, j, k, age ) > DepositingThicknessTolerance;
                        }

                        volumeElement.setIsActive ( activeSegment1 or activeSegment1 );
                        volumeElement.setIsActiveBoundary ( VolumeData::GAMMA_4, activeSegment1 or activeSegment2 );
                     } 


                  } else if ( mapElements.firstJ ( false ) != 0 and j < mapElements.firstJ ( false )) {

                     if ( NumericFunctions::inRange<unsigned int> ( i, mapElements.firstI ( false ), mapElements.lastI ( false ))) {

                        if ( geometricLoop ) {
                           activeSegment1 = getSolidThickness ( i,     j, k, age ) > DepositingThicknessTolerance;
                           activeSegment2 = getSolidThickness ( i + 1, j, k, age ) > DepositingThicknessTolerance;
                        } else {
                           activeSegment1 = getRealThickness ( i,     j, k, age ) > DepositingThicknessTolerance;
                           activeSegment2 = getRealThickness ( i + 1, j, k, age ) > DepositingThicknessTolerance;
                        }

                        volumeElement.setIsActive ( activeSegment1 or activeSegment1 );
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

   if ( FastcauldronSimulator::getInstance ().getCauldron ()->isGeometricLoop ()) {
      return getSolidThickness ( i, j, k, age );
   } else {
      return getRealThickness ( i, j, k, age );
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

  if ( layerDA == 0 ) {
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

#undef  __FUNCT__
#define __FUNCT__ "LayerProps::LayerProps::SetIncludedNodeArray"

void LayerProps::SetIncludedNodeArray ( const Boolean2DArray& Valid_Needle ) {

   if ( getMaximumNumberOfElements () <= 0 ) {
    ///
    /// If there are no elements in the layer then there will be no nodes that need to be included/not-included
    ///
    return;
  }

  int I;
  int J;
  int K;

  int xStart;
  int xCount;
  int xEnd;

  int yStart;
  int yCount;
  int yEnd;

  int zStart;
  int zCount;
  int zEnd;
  bool IncludeGhosts = true;

  DMCreateGlobalVector ( layerDA, &includedNodeVec );

  VecSet ( includedNodeVec, Zero );

  PETSC_3D_Array includedNodeArray ( layerDA, includedNodeVec, INSERT_VALUES, IncludeGhosts );

  DMDAGetGhostCorners ( layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

  xEnd = xStart + xCount;
  yEnd = yStart + yCount;
  zEnd = zStart + zCount;

  for ( I = xStart; I < xEnd; I++ ) {

    for ( J = yStart; J < yEnd; J++ ) {

      if ( Valid_Needle ( I, J )) {

        if ( getLithology ( I, J ) -> surfacePorosity () == 0.0 || // salt layer
             ( fluid->SwitchPermafrost() && fluid->density( 0, 0.1 ) > getLithology( I, J )->density() ) ) { // NLSAY3: Ice sheet modeling

          for ( K = zStart; K < zEnd; K++ ) {
            includedNodeArray ( K, J, I ) = 0.0;
          }

        } else {

          for ( K = zStart; K < zEnd; K++ ) {
            ///
            /// Dont care what the value is here as long as its greater than 0.0
            ///
            includedNodeArray ( K, J, I ) = 1.0; 
          }
        }
      }
    }
  }

  includedNodeArray.Restore_Global_Array( Update_Including_Ghosts );

  /// Re-initialise the includedNodeArray PETSC_Array object for copy of its values
  includedNodeArray.Set_Global_Array ( layerDA, includedNodeVec, INSERT_VALUES, true );

  for ( I = xStart; I < xEnd; I++ ) {

    for ( J = yStart; J < yEnd; J++ ) {

      if ( Valid_Needle ( I, J )) {

        for ( K = zStart; K < zEnd; K++ ) {
          includedNodes ( I, J, K ) = includedNodeArray ( K, J, I ) > 0.0;
        }

      }

    }

  }

  VecDestroy ( &includedNodeVec );

}

//------------------------------------------------------------//


void LayerProps::SetTopIncludedNodes ( const Boolean2DArray& Valid_Needle,
                                             Boolean2DArray& topIncludedNodes ) {

   if ( getMaximumNumberOfElements () <= 0 ) {
    ///
    /// If there are no elements in the layer then there will be no nodes that need to be included/not-included
    ///
    return;
  }

  int I;
  int J;

  int xStart;
  int xCount;
  int xEnd;

  int yStart;
  int yCount;
  int yEnd;

  int zStart;
  int zCount;
  int zTop;

  DMDAGetGhostCorners ( layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

  xEnd = xStart + xCount;
  yEnd = yStart + yCount;
  zTop = zStart + zCount - 1;

  for ( I = xStart; I < xEnd; I++ ) {

    for ( J = yStart; J < yEnd; J++ ) {

      if ( Valid_Needle ( I, J )) {
        includedNodes ( I, J, zTop ) = includedNodes ( I, J, zTop ) || topIncludedNodes ( I, J );

        ///
        /// Now that the layer array has been updated, we can overwrite the topIncludedNodes
        /// array with the bottom of the layer ready for the top of the next layer.
        ///
        topIncludedNodes ( I, J ) = includedNodes ( I, J, 0 );
      }

    }

  }

}

//------------------------------------------------------------//


void LayerProps::SetBottomIncludedNodes ( const Boolean2DArray& Valid_Needle,
                                                Boolean2DArray& bottomIncludedNodes ) {

   if ( getMaximumNumberOfElements () <= 0 ) {
    ///
    /// If there are no elements in the layer then there will be no nodes that need to be included/not-included
    ///
    return;
  }

  int I;
  int J;

  int xStart;
  int xCount;
  int xEnd;

  int yStart;
  int yCount;
  int yEnd;

  int zStart;
  int zCount;
  int zTop;

  DMDAGetGhostCorners ( layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

  xEnd = xStart + xCount;
  yEnd = yStart + yCount;
  zTop = zStart + zCount - 1;

  for ( I = xStart; I < xEnd; I++ ) {

    for ( J = yStart; J < yEnd; J++ ) {

      if ( Valid_Needle ( I, J )) {
        includedNodes ( I, J, 0 ) = includedNodes ( I, J, 0 ) || bottomIncludedNodes ( I, J );

        ///
        /// Now that the layer array has been updated, we can overwrite the bottomIncludedNodes
        /// array with the bottom of the layer ready for the bottom of the next layer.
        ///
        bottomIncludedNodes ( I, J ) = includedNodes ( I, J, zTop );
      }

    }

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
  VecSet ( faultElements, CAULDRONIBSNULLVALUE );

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
  VecSet ( erosionFactor, CAULDRONIBSNULLVALUE );

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
   GeoPhysics::Formation::switchLithologies ( age );

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

  DMDAGetCorners ( layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

  DMCreateGlobalVector ( layerDA, &allochthonousLithologyMap );
  VecSet ( allochthonousLithologyMap, CAULDRONIBSNULLVALUE );

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

   PetscBool validVector ( faultElements != 0 ? PETSC_TRUE : PETSC_FALSE );

  // VecValid ( faultElements, &validVector );
  
  if ( validVector ) {
    VecDestroy ( &faultElements );
    faultElements = Vec ( 0 );
  }

}

//------------------------------------------------------------//

void LayerProps::deleteErosionFactorMap () {

  PetscBool validVector ( erosionFactor != 0 ? PETSC_TRUE : PETSC_FALSE );

  // VecValid ( erosionFactor, &validVector );

  if ( validVector ) {
    VecDestroy ( &erosionFactor );
    erosionFactor = Vec ( 0 );
  }

}

//------------------------------------------------------------//


void LayerProps::PutIncludedNodes () const {

   if ( getMaximumNumberOfElements () <= 0 ) {
    ///
    /// If there are no elements in the layer then there will be no nodes that need to be included/not-included
    ///
    return;
  }

  int I;
  int J;
  int K;

  int xStart;
  int xCount;
  int xEnd;

  int yStart;
  int yCount;
  int yEnd;

  int zStart;
  int zCount;
  int zEnd;

  DMDAGetGhostCorners ( layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

  xEnd = xStart + xCount;
  yEnd = yStart + yCount;
  zEnd = zStart + zCount;

  cout << " Nodes for layer " << layername << endl;

  for ( I = xStart; I < xEnd; I++ ) {

    for ( J = yStart; J < yEnd; J++ ) {

      for ( K = zStart; K < zEnd; K++ ) {
        cout << " Include Node " << I << "  " << J << "  " << K << "  " << int ( includedNodes ( I, J, K )) << endl;
      }

    }

  }

}

//------------------------------------------------------------//

void LayerProps::setSnapshotInterval ( const SnapshotInterval& interval,
                                             AppCtx*          basinModel ) {


   // If the vesInterpolator != 0 then the maxVesInterpolator will also != 0.
   if ( isSediment () && vesInterpolator != 0 ) {
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
              propertyArray ( K, J, I ) = CAULDRONIBSNULLVALUE;
           }

        }

     }

  }

}

//------------------------------------------------------------//

void LayerProps::interpolateProperty (       AppCtx*                  basinModel,
                                       const double                   currentTime, 
                                       const bool                     doingHighResDecompaction,
                                       const PropertyList             property ) {


  if ( ! isSediment () || vesInterpolator == 0 ) {
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


void LayerProps::Determine_CFL_Value ( AppCtx* Basin_Model,
                                       double& Layer_CFL_Value ) {

  using namespace Basin_Modelling;

  const bool Include_Chemical_Compaction = (( Basin_Model -> Do_Chemical_Compaction ) && ( Get_Chemical_Compaction_Mode ()));

  double Element_CFL_Value;

  int K;
  int I_Position;
  int J_Position;

  int X_Start, X_Count;
  int Y_Start, Y_Count;
  int Z_Start, Z_Count;
  size_t Element_Index;
  int layerMx, layerMy, layerMz;
  int Inode;

  const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  double Delta_X  = grid.deltaI;
  double Delta_Y  = grid.deltaJ;
  double Origin_X = grid.originI;
  double Origin_Y = grid.originJ;

//   double Delta_X  = Basin_Model -> getDeltaX  ( Basin_Model -> Grid_Resolution );
//   double Delta_Y  = Basin_Model -> getDeltaY  ( Basin_Model -> Grid_Resolution );
//   double Origin_X = Basin_Model -> getXorigin ( Basin_Model -> Grid_Resolution );
//   double Origin_Y = Basin_Model -> getYorigin ( Basin_Model -> Grid_Resolution );

  const CompoundLithology*  Element_Lithology;


  Layer_CFL_Value = 100.0 * Secs_IN_MA;

  ElementVector Current_Ph;
  ElementVector Current_Po;
  ElementVector Current_Element_VES;
  ElementVector Current_Element_Max_VES;
  ElementVector Current_Element_Temperature;
  ElementVector Current_Element_Chemical_Compaction;
  ElementGeometryMatrix Geometry_Matrix;

  ElementVector Exceeded_Fracture_Pressure;
//    Boolean_Vector Exceeded_Fracture_Pressure;

  bool Include_Ghost_Values = true;

  Element_Positions Positions;

  DMDAGetInfo( layerDA,0,&layerMx,&layerMy,&layerMz,0,0,0,0,0,0,0,0,0);
  DMDAGetCorners ( layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );

  Current_Properties.Activate_Properties  ( INSERT_VALUES, Include_Ghost_Values );

  for ( K = Z_Start; K < Z_Start + Z_Count; K++ ) {

    if ( K != layerMz - 1 ) {

      for ( Element_Index = 0; Element_Index < Basin_Model -> mapElementList.size (); Element_Index++ ) {

        if ( Basin_Model -> mapElementList [ Element_Index ].exists ) {
          //
          //
          // Copy segment lithologies
          //
          I_Position = Basin_Model->mapElementList [ Element_Index ].i [ 0 ];
          J_Position = Basin_Model->mapElementList [ Element_Index ].j [ 0 ];

          Element_Lithology = getLithology ( I_Position, J_Position );

          for (Inode = 0; Inode<8; Inode++) {
            int LidxZ = K + (Inode<4 ? 1 : 0);
            int GidxY = Basin_Model->mapElementList [ Element_Index ].j[Inode%4];
            int GidxX = Basin_Model->mapElementList [ Element_Index ].i[Inode%4];

            Positions.Set_Node_Position ( Inode, LidxZ, GidxY, GidxX );
            Exceeded_Fracture_Pressure ( Inode + 1 ) = fracturedPermeabilityScaling ( GidxX, GidxY, LidxZ );

            Geometry_Matrix ( 1, Inode + 1 ) = Origin_X + (Delta_X * GidxX);
            Geometry_Matrix ( 2, Inode + 1 ) = Origin_Y + (Delta_Y * GidxY);
            Geometry_Matrix ( 3, Inode + 1 ) = Current_Properties ( Basin_Modelling::Depth, LidxZ, GidxY, GidxX );
          }

          Current_Properties.Extract_Property ( Basin_Modelling::Hydrostatic_Pressure, Positions, Current_Ph );
          Current_Properties.Extract_Property ( Basin_Modelling::Overpressure,         Positions, Current_Po );
          Current_Properties.Extract_Property ( Basin_Modelling::VES_FP,               Positions, Current_Element_VES );
          Current_Properties.Extract_Property ( Basin_Modelling::Max_VES,              Positions, Current_Element_Max_VES );
          Current_Properties.Extract_Property ( Basin_Modelling::Temperature,          Positions, Current_Element_Temperature );
          Current_Properties.Extract_Property ( Basin_Modelling::Chemical_Compaction,  Positions, Current_Element_Chemical_Compaction );

          Element_CFL_Value = Basin_Modelling::CFL_Value ( Element_Lithology, this->fluid, 
                                                           Include_Chemical_Compaction,
                                                           Geometry_Matrix,
                                                           Current_Ph, 
                                                           Current_Po, 
                                                           Current_Element_VES, 
                                                           Current_Element_Max_VES, 
                                                           Current_Element_Temperature,
                                                           Current_Element_Chemical_Compaction,
                                                           Exceeded_Fracture_Pressure );

          Layer_CFL_Value = NumericFunctions::Minimum ( Layer_CFL_Value, Element_CFL_Value );

        }

      }

    }

  }


  Layer_CFL_Value = Layer_CFL_Value / Secs_IN_MA;

  Current_Properties.Restore_Properties ();

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

   return globalMaximumPermeability / GeoPhysics::MILLIDARCYTOM2;
}

//------------------------------------------------------------//

bool LayerProps::isAllochthonous () const {
  return m_compoundLithologies.hasAllochthonousLithologyInterpolator ();
}

//------------------------------------------------------------//

void LayerProps::setChemicalCompactionVesValue ( const double newVesValue ) {

  chemicalCompactionVesValueIsDefined = true;
  chemicalCompactionVesValue = newVesValue;

}

//------------------------------------------------------------//

bool LayerProps::outputValid ( const PropertyList currentProperty ) const {

  switch ( currentProperty ) {

    // The layer should output the allochthonous lithology distribution iff
    // the layer is an allochthonous layer. Otherwise a maps consisting of
    // only zeros would be output.
    case ALLOCHTHONOUS_LITHOLOGY : return isAllochthonous ();

//     // The layer should output the faulted element map iff
//     // the layer has a fault defined. Otherwise a maps consisting of
//     // only zeros would be output.
//     case FAULTELEMENTS : return isFaulted ();

    // The pressure output is only valid in the sediments.
    case PRESSURE : return isSediment ();

    // The hydrostatic pressure output is only valid in the sediments.
    case HYDROSTATICPRESSURE : return isSediment ();

    // The overpressure output is only valid in the sediments.
    case OVERPRESSURE : return isSediment ();

    // The Vr output is only valid in the sediments.
    case VR : return isSediment ();

    // Always assume that a property can be output
    default : return true;
  }

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

                  // depositionSolidThickness += getSolidThickness ( i, j, k, 0.0 );
                  // currentSolidThickness += getSolidThickness ( i, j, k, endTime );
                  // previousSolidThickness += getSolidThickness ( i, j, k, startTime );
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
                  // depositionSolidThickness += getSolidThickness ( i, j, k, 0.0 );
                  // currentSolidThickness += getSolidThickness ( i, j, k, time );
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


//    if ( previousTime - currentTime > sourceRock->getMaximumTimeStepSize ( depoage ) and currentTime < depoage ) {
//       extractGenexDataInterval ( previousTime, currentTime, vesInterp, temperatureInterp );
//       sourceRock->computeTimeInterval ( previousTime, currentTime, &vesInterp, &temperatureInterp, &thicknessInterp );
//    } else {
//       extractGenexDataInstance ( currentTime, vesInterp, temperatureInterp );
//       sourceRock->computeTimeInstance ( currentTime, &vesInterp, &temperatureInterp, &thicknessInterp );
//    }

}

//------------------------------------------------------------//

void LayerProps::getGenexGenerated ( const int i,
                                     const int j,
                                     PVTComponents& generated ) const {


   if ( isSourceRock ()) {
      unsigned int cmp;
      unsigned int id;
      pvtFlash::ComponentId species;
      double gen;

      const GeoPhysics::GeoPhysicsSourceRock* sourceRock = (GeoPhysics::GeoPhysicsSourceRock*)(getSourceRock1 ());
      const Genex6::SpeciesManager& speciesManager = sourceRock->getSpeciesManager ();

      for ( cmp = 0; cmp < NumberOfPVTComponents; ++cmp ) {
         species = pvtFlash::ComponentId ( cmp );
         id = speciesManager.mapPvtComponentsToId ( species );

         gen = m_genexData->getValue ( (unsigned int)i, (unsigned int)j, cmp );

         if ( gen == CAULDRONIBSNULLVALUE or gen == IBSNULLVALUE ) {
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

#undef  __FUNCT__  
#define __FUNCT__ "LayerProps::Integrate_Chemical_Compaction"

void LayerProps::Integrate_Chemical_Compaction ( const double          Previous_Time,
                                                 const double          Current_Time,
                                                 const Boolean2DArray& validNeedle ) {

  // Here we dont need to know about the global chemical compaction switch, because
  // if the global switch is off then we would not be in this function!
  if ( ! Calculate_Chemical_Compaction ) {
    return;
  }

  int K;
  int I;
  int J;

  int X_Start, X_Count;
  int Y_Start, Y_Count;
  int Z_Start, Z_Count;

  const double Time_Step = Previous_Time - Current_Time;

  double Current_VES;
  double Current_Nodal_Porosity;
  double Current_Temperature;
  double previousChemicalCompactionValue;


  const CompoundLithology*  Current_Lithology;
  PETSC_3D_Array Layer_Porosity ( layerDA, Porosity );

  DMDAGetCorners ( layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );

  Current_Properties.Activate_Property  ( Basin_Modelling::VES_FP );
  Current_Properties.Activate_Property  ( Basin_Modelling::Temperature );
  Current_Properties.Activate_Property  ( Basin_Modelling::Chemical_Compaction );

  Previous_Properties.Activate_Property ( Basin_Modelling::VES_FP );
  Previous_Properties.Activate_Property ( Basin_Modelling::Max_VES );
  Previous_Properties.Activate_Property ( Basin_Modelling::Temperature );
  Previous_Properties.Activate_Property ( Basin_Modelling::Chemical_Compaction );

  for ( I = X_Start; I < X_Start + X_Count; I++ ) {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

      if ( validNeedle ( I, J )) {
        Current_Lithology = getLithology ( I, J );

        for ( K = Z_Start + Z_Count - 1; K >= Z_Start; K-- ) {

          // If the chemical compaction env-var is defined then use this value for the ves, otherwise use the computed value.
          if ( chemicalCompactionVesValueIsDefined ) {
            Current_VES = chemicalCompactionVesValue;
          } else {
            Current_VES = Current_Properties ( Basin_Modelling::VES_FP, K, J, I );
          }

          Current_Nodal_Porosity = Layer_Porosity ( K, J, I );
          Current_Temperature = Current_Properties ( Basin_Modelling::Temperature, K, J, I );

          previousChemicalCompactionValue = Previous_Properties ( Basin_Modelling::Chemical_Compaction, K, J, I );

          Current_Properties ( Basin_Modelling::Chemical_Compaction, K, J, I ) = previousChemicalCompactionValue + 
                                                            Current_Lithology->integrateChemicalCompaction ( Time_Step,
                                                                                                             Current_VES, 
                                                                                                             Current_Nodal_Porosity, 
                                                                                                             Current_Temperature );

        }

      }

    }

  }

  Current_Properties.Restore_Property  ( Basin_Modelling::VES_FP );
  Current_Properties.Restore_Property  ( Basin_Modelling::Temperature );
  Current_Properties.Restore_Property  ( Basin_Modelling::Chemical_Compaction );

  Previous_Properties.Restore_Property ( Basin_Modelling::VES_FP );
  Previous_Properties.Restore_Property ( Basin_Modelling::Max_VES );
  Previous_Properties.Restore_Property ( Basin_Modelling::Temperature );
  Previous_Properties.Restore_Property ( Basin_Modelling::Chemical_Compaction );

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
   if ( m_elementVolumeGrids [ numberOfDofs - 1 ] == 0 ) {
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
   if ( m_nodalVolumeGrids [ numberOfDofs - 1 ] == 0 ) {
      m_nodalVolumeGrids [ numberOfDofs - 1 ] = new NodalVolumeGrid;
      m_nodalVolumeGrids [ numberOfDofs - 1 ]->construct ( FastcauldronSimulator::getInstance ().getNodalGrid (),
                                                           getMaximumNumberOfElements () + 1,
                                                           numberOfDofs );
   }

}

//------------------------------------------------------------//
