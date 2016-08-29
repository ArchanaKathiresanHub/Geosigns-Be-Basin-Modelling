//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _FASTCAULDRON__LAYER_H_
#define _FASTCAULDRON__LAYER_H_

#include <set>
#include <string>
#include <vector>

#include "petscvec.h"
#include "petscdmda.h"
#include "Polyfunction.h"//from CBM Generics
using CBMGenerics::Polyfunction;

#include "CompoundLithology.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicsFormation.h"
#include "filterwizard.h"
#include "globaldefs.h"
#include "ghost_array.h"
#include "PetscVectors.h"
#include "CompoundLithologyArray.h"

#include "snapshotdata.h"
#include "snapshotinterpolator.h"

#include "CauldronGridDescription.h"

#include "property_manager.h"

#include "BiomarkersSimulatorState.h"
#include "SmectiteIlliteSimulatorState.h"

#include "LinearPropertyInterpolator.h"

#include "ElementVolumeGrid.h"
#include "NodalVolumeGrid.h"
#include "LayerElement.h"


#include "PVTCalculator.h"


using namespace std;

#include "Interface/GridMap.h"
#include "Interface/Formation.h"
using namespace DataAccess;
using namespace GeoPhysics;

//------------------------------------------------------------//


struct listVec {
  Vec* VecArray[PropertyListSize];
};


typedef vector<double> STLVecOfDouble;


struct Boundary_Set {
  double Start_Time;
  double End_Time;
  double Boundary_Value;
} ;

typedef vector< Boundary_Set* > Constrained_Property;

typedef GeoPhysics::Local2DArray <int> PETSc_2D_Integer_Array;
// typedef PETSc_Local_2D_Array<int> PETSc_2D_Integer_Array;

typedef PETSc_Local_3D_Array<bool> PETSc_3D_Boolean_Array;

typedef PETSc_Local_3D_Array<int> PETSc_3D_Integer_Array;

typedef PETSc_Local_3D_Array<double> PETSc3DDoubleArray;

typedef PETSc_Local_3D_Array<PVTComponents> PVTComponents3DArray;


class LayerProps : virtual public GeoPhysics::Formation
{
public:


   LayerProps ( Interface::ProjectHandle * projectHandle,
                database::Record *              record);

   virtual ~LayerProps();

   virtual void initialise ();

   virtual void connectElements ( LayerProps* layerAbove );

   void switchLithologies ( const double age );

   bool allocateNewVecs ( AppCtx* basinModel, const double Current_Time );

   void initialiseTemperature( AppCtx* basinModel, const double Current_Time );

   void initialisePreviousFluidPressures( AppCtx* basinModel, const double Current_Time );

   double calcDiffDensity ( const unsigned int i, const unsigned int j ) const;

   const GeoPhysics::FluidType* getFluid () const;

   int  getNrOfActiveElements () const;

   void setNrOfActiveElements ( const int a_nrActElem );

   int  Get_TimeIo_DepoSeq ( const SurfacePosition Position,
                             const bool Property_Is_A_Vector );

   bool isActive () const;

   bool isSediment () const;

   bool isBasement () const;

   bool isSourceRock () const;

   bool isMobile () const;

   bool Is_Overpressure_Constrained () const;

   virtual bool isBasalt() const { return false; };

   /// \brief Set the activity of the layer elements.
   ///
   /// Both element and face activity is set.
   virtual void setLayerElementActivity ( const double age );

   /// \brief Return a constant reference to the array containing the layer elements.
   const LayerElementArray& getLayerElements () const;

   /// \brief Return a constant reference to the layer element at the position.
   ///
   /// The k index is relative to the locak layer based k counting scheme.
   const LayerElement& getLayerElement ( const int i, 
                                         const int j,
                                         const int k ) const;

   /// \brief Return a reference to the layer element at the position.
   ///
   /// The k index is relative to the locak layer based k counting scheme.
   LayerElement& getLayerElement ( const int i, 
                                   const int j,
                                   const int k );


   /// Return the thickness of the segment at the age that is being deposited.
   ///
   /// I.e. If the simulation is a pressure or temperature simulation and is non-geometric 
   /// loop the return the solid thickness otherwise return the real-thickness.
   double getDepositingThickness ( const unsigned int i,
                                   const unsigned int j,
                                   const unsigned int k,
                                   const double       age ) const;

  void Determine_CFL_Value ( AppCtx* Basin_Model,
                             double& Layer_CFL_Value );


   double estimateStandardPermeability () const;

  void print();
  void nullify ();
  void Initialise();
  void reInitialise();
  void setVectorList();
  void Create_FC_Thickness_Polyfunction ( const DM& Map_DA );

  void setConstrainedOverpressureInterval ( const double startTime,
                                            const double endTime,
                                            const double constrainedOverpressureValue );

  void getConstrainedOverpressure ( const double Time, double& Value, bool& Is_Constrained ) const;

  void Set_Chemical_Compaction_Mode ( const bool Mode );


  bool Get_Chemical_Compaction_Mode () const { return Calculate_Chemical_Compaction; }

   void integrateGenexEquations ( const double previousTime,
                                  const double currentTime );


   void clearGenexOutput ();

   /// \brief Set the transported mass vector to zero.
   void zeroTransportedMass ();

   void getGenexGenerated ( const int i,
                            const int j,
                            PVTComponents& generated ) const;

   void retrieveGenexData ();

   bool genexDataIsRetrieved () const;

   void restoreGenexData ();

   const PVTComponents& getMolarMass () const;

  void Fill_Topmost_Segment_Arrays ( const double          Previous_Time, 
                                     const double          Current_Time,
                                     const bool            Use_Solid_Thickness,
                                     const Boolean2DArray& Valid_Needle );

  void setFaultElementsMap ( AppCtx*         Basin_Model,
			     const Boolean2DArray& validNeedle );

  void deleteFaultElementsMap ();

  void setAllochthonousLithologyMap ( AppCtx*         Basin_Model,
                                      const Boolean2DArray& validNeedle );

  void deleteAllochthonousLithologyMap ();


  /// \brief Compute the map of ratio of current thickness to deposition thickness.
  ///
  /// The thicknesses used will be:
  ///   - Solid thickness for geometric loop; and
  ///   - Real thickness for non-geometric loop.
  void setErosionFactorMap ( AppCtx*         basinModel,
			     const Boolean2DArray& validNeedle,
			     const double    currentAge );

  /// Delete the erosion-factor maps.
  void deleteErosionFactorMap ();

  void setSnapshotInterval ( const SnapshotInterval& interval,
			            AppCtx*          basinModel );

  void interpolateProperty (       AppCtx*                  basinModel,
                             const double                   currentTime, 
                             const bool                     doingHighResDecompaction,
                             const PropertyList             property );


  // Set the ves value that is to be used in chemical compaction instead of the computed ves value.
  void setChemicalCompactionVesValue ( const double newVesValue );

  virtual void allocateBasementVecs() {};
  virtual void reInitialiseBasementVecs() {};

  string           layername;
  string           TopSurfaceName;
  int              TopSurface_DepoSeq;
  string           BottSurfaceName;
  int              BottSurface_DepoSeq;

  Vec              depthvec;          
  const Interface::GridMap * depthGridMap;
  const Interface::GridMap * presentDayThickness;


   // Replace ALL calls to this function with those to getCompoundLithology.
   const CompoundLithology* getLithology ( const int i, const int j ) const {
      return getCompoundLithology ( i, j );
   }

   virtual const CompoundLithology* getLithology ( const int i, const int j, const int k ) const {
      // Added to prevent a compiler warning about unused parameter.
      (void) k;
      return getCompoundLithology ( i, j );
   }
   virtual const CompoundLithology* getLithology ( const double time, const int i, const int j, const double k ) {
      // Added to prevent a compiler warning about unused parameter.
      (void) k;
      (void) time;
      return getCompoundLithology ( i, j );
   }
   virtual void setBasaltLitho (const int iPosition, const int jPosition, const int kPosition ) {
      // Added to prevent a compiler warning about unused parameter.
      (void) iPosition;
      (void) jPosition;
      (void) kPosition;
   }

   virtual bool getPreviousBasaltLitho(const int iPosition, const int jPosition, const int kPosition ){
      // Added to prevent a compiler warning about unused parameter.
      (void) iPosition;
      (void) jPosition;
      (void) kPosition;
      return false;
   } 
      
   // Vec getGeneratedMasses ();

   const Vec& getComponentVec () const;

   Vec& getComponentVec ();

   const Vec& getPreviousComponentVec () const;

   Vec& getPreviousComponentVec ();

   /// \brief Return a const reference to the vector of the immobile species.
   const Vec& getImmobileComponentsVec () const;

   /// \brief Return a reference to the vector of the immobile species.
   Vec& getImmobileComponentsVec ();

   const Vec& getPhaseSaturationVec () const;

   Vec& getPhaseSaturationVec ();

   const Vec& getPreviousPhaseSaturationVec () const;

   Vec& getPreviousPhaseSaturationVec ();

   /// \brief Return the vector containing the transported masses.
   ///
   /// The masses for the hc transported over a snapshot-interval.
   Vec& getTransportedMassesVec ();

   /// \brief Return the vector containing the transported masses.
   ///
   /// The masses for the hc transported over a snapshot-interval.
   const Vec& getTransportedMassesVec () const;


   /// \brief Get the vector containing the averaged saturations.
   Vec& getAveragedSaturations ();

   /// \brief Get the vector containing the averaged saturations.
   const Vec& getAveragedSaturations () const;

   /// \brief Get the vector containing time of element invasion
   Vec& getTimeOfElementInvasionVec ();
   /// \brief Get the vector containing time of element invasion
   const Vec& getTimeOfElementInvasionVec () const;

   const ElementVolumeGrid& getComponentLayerVolumes () const;

   const ElementVolumeGrid& getSaturationGrid () const;

   const ElementVolumeGrid& getElementFluxGrid () const;
   
   const ElementVolumeGrid& getTimeOfElementInvasionGrid () const;


   /// \brief Copy all current computed properties to previous.
   void copyProperties ();

   /// \brief Copy current computed saturations to previous.
   void copySaturations ();

  string           m_lithoMixModel;
  float       m_lithoLayeringIndex;

  string           depthgridfilename;
  int              depthgridfileseqnr;

  // Age at end of deposition.
  double           depoage;

  // Age at start of deposition.
  double           depositionStartAge;

  bool             IsSourceRock;
  bool             IsMobile;
  int              Layer_Depo_Seq_Nb;
   GeoPhysics::FluidType const* fluid;

  int              destroyCount;
  int              createCount;


  PETSc_2D_Integer_Array Previous_Topmost_Segments;
  PETSc_2D_Integer_Array Current_Topmost_Segments;



  DM               layerDA;

  Vec Depth;

  Vec              Lithology_ID;
  Vec              BulkDensXHeatCapacity;
  Vec              BulkTHCondN;
  Vec              BulkTHCondP;
  Vec              BulkHeatProd;

  PETSc_Local_3D_Array<Calibration::SmectiteIlliteSimulatorState> m_SmectiteIlliteState;
  PETSc_Local_3D_Array<Calibration::BiomarkersSimulatorState> m_BiomarkersState;

  void resetSmectiteIlliteStateVectors();
  void resetBiomarkerStateVectors();

  //~~~~~~~~~ pressure - Temperature Coupling ~~~~~~~
  Vec              FCTCorrection;

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  //~~~~~~~~~~~~~~~~Vector Properties~~~~~~~~~~~~~~~~
  Vec  Diffusivity, Porosity, Velocity, Reflectivity;
  Vec  Sonic, BulkDensity, ThCond, PermeabilityV, PermeabilityH;
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Vec m_averagedSaturation;
  Vec m_timeOfElementInvasionVec;

  //~~~~~~~~~~~~~~~~Scalar Properties~~~~~~~~~~~~~~~~
  Vec Vre;

  Vec m_IlliteFraction;
  Vec m_HopaneIsomerisation, m_SteraneIsomerisation, m_SteraneAromatisation;
  
  //
  //
  // These properties should be removed when the property manager
  // has been implemented. Perhaps ALL properties should be put there
  //

  Vec layerThickness;
  Vec Thickness_Error;
  Vec erosionFactor;
  Vec Computed_Deposition_Thickness;

  Vec faultElements;
  Vec allochthonousLithologyMap;

  PETSc3DDoubleArray     fracturedPermeabilityScaling;
  PETSc_3D_Boolean_Array pressureExceedsFracturePressure;
  PETSc_3D_Boolean_Array nodeIsTemporarilyDirichlet;
  PETSc3DDoubleArray preFractureScaling;

  Basin_Modelling::Fundamental_Property_Manager Current_Properties;
  Basin_Modelling::Fundamental_Property_Manager Previous_Properties;

   double getCurrentLayerThickness ( const int i,
                                     const int j ) const;

  listVec          vectorList;   



   /// Create a volume-grid with the number of dofs indicated and add it to the array of volume-grids.
   ///
   /// If volume-grid exists already then no action will be taken.
   void createVolumeGrid ( const int numberOfDofs );

   /// \brief Return reference to ElementVolumeGrid with corresponding number of dofs.
   ///
   /// If the corresponding volume-grid does not exist then one will be created.
   ElementVolumeGrid& getVolumeGrid ( const int numberOfDofs = 1 );

   /// \brief Return const reference to ElementVolumeGrid with corresponding number of dofs.
   ///
   /// If the corresponding volume-grid does not exist then one will be created.
   const ElementVolumeGrid& getVolumeGrid ( const int numberOfDofs = 1 ) const;



   /// Create a nodal-volume-grid with the number of dofs indicated and add it to the array of volume-grids.
   ///
   /// If nodal-volume-grid exists already then no action will be taken.
   void createNodalVolumeGrid ( const int numberOfDofs );

   /// \brief Return reference to NodalVolumeGrid with corresponding number of dofs.
   ///
   /// If the corresponding nodal-volume-grid does not exist then one will be created.
   NodalVolumeGrid& getNodalVolumeGrid ( const int numberOfDofs = 1 );

   /// \brief Return const reference to NodalVolumeGrid with corresponding number of dofs.
   ///
   /// If the corresponding nodal-volume-grid does not exist then one will be created.
   const NodalVolumeGrid& getNodalVolumeGrid ( const int numberOfDofs = 1 ) const;


   bool isChemicalCompactionVesValueIsDefined() const;

   double getChemicalCompactionVesValue() const;

 protected :

   /// \brief Allocate array containing all the layer elements and set members that will not change, e.g. (i,j,k) position.
   void setLayerElements ();

   bool Calculate_Chemical_Compaction;
   void extractGenexDataInterval ( const double startTime,
                                   const double endTime,
                                   LinearPropertyInterpolator& ves,
                                   LinearPropertyInterpolator& temperatureves,
                                   LinearPropertyInterpolator& hydrostaticPressure,
                                   LinearPropertyInterpolator& lithostaticPressure,
                                   LinearPropertyInterpolator& porePressure,
                                   LinearPropertyInterpolator& porosity,
                                   LinearPropertyInterpolator& permeability,
                                   LinearPropertyInterpolator& vre );

   void extractGenexDataInstance ( const double time,
                                   LinearPropertyInterpolator& ves,
                                   LinearPropertyInterpolator& temperature );

   void computeThicknessScaling ( const double startTime,
                                  const double endTime,
                                  LinearPropertyInterpolator& thicknessScaling );

   void computeThicknessScaling ( const double time,
                                  LinearPropertyInterpolator& thicknessScaling );


  bool createVec(Vec& propertyVector);
  bool destroyDA(DM& propertyDA);
  bool setVec(Vec& propertyVector, const double propertyValue);
  bool propagateVec(DM from_da, DM to_da, Vec from_vec, Vec to_vec);


  void Fill_Topmost_Segment_Array ( const double                  Required_Age,
                                    const bool                    Use_Solid_Thickness,
                                    const Boolean2DArray&         Valid_Needle,
                                          PETSc_2D_Integer_Array& Topmost_Segments,
                                          int&                    Topmost_Segment ) const;

   void setElementInvariants ();


   void initialiseSourceRockProperties ( const bool printInitialisationDetails );

  int  m_nrOfActiveElements;
  bool Hydro_Sand;
  Constrained_Property Constrained_Overpressure;

  ///
  /// Fundamental properties from current time step.
  ///
  Vec Real_Thickness_Vector; // This name will change when it is moved to property manager.
  Vec Solid_Thickness;
  Vec OverPressure;
  Vec HydroStaticPressure;
  Vec Pressure;
  Vec Chemical_Compaction;
  Vec LithoStaticPressure;
  Vec Temperature;
  Vec Ves;
  Vec Max_VES;

  ///
  /// Fundamental properties from previous time step.
  ///
  Vec Previous_Real_Thickness_Vector; // This name will change when it is moved to property manager.
  Vec Previous_VES;
  Vec Previous_Max_VES;
  Vec Previous_Depth;
  Vec Previous_Solid_Thickness;
  Vec Previous_Hydrostatic_Pressure;
  Vec Previous_Lithostatic_Pressure;
  Vec Previous_Pore_Pressure;
  Vec Previous_Overpressure;
  Vec Previous_Temperature;
  Vec Previous_Chemical_Compaction;



  Vec  includedNodeVec;

  void interpolateProperty (       AppCtx*         basinModel,
                             const double          currentTime,
                             DM                    propertyDA,
                             SnapshotInterpolator& interpolator,
                             Vec                   propertyVector );


  SnapshotInterpolator*  vesInterpolator;
  SnapshotInterpolator*  maxVesInterpolator;

  bool   chemicalCompactionVesValueIsDefined;
  double chemicalCompactionVesValue;


   Interface::GridMap* m_genexData;
   // PVTComponents3DArray m_generatedMasses;

   PVTComponents m_molarMass;


   ElementVolumeGrid  m_componentLayerVolumes;
   Vec                m_flowComponents;
   Vec                m_previousFlowComponents;

   /// \brief Vector containing the immobile species for each element.
   ///
   /// Units will be kg/m^3.
   ElementVolumeGrid  m_immobilesLayerGrid;
   Vec                m_immobileComponents;


   /// \brief Contains the three saturations.
   ///
   /// S_w, S_o and S_g.
   ElementVolumeGrid  m_saturationGrid;
   Vec                m_saturations;
   Vec                m_previousSaturations;

   NodalVolumeGrid    m_averagedSaturationGrid;

   Vec                m_transportedMasses;

   LayerElementArray m_elements;


   ElementVolumeGrid m_elementFluxGrid;

    ElementVolumeGrid  m_timeOfElementInvasionGrid;

   /// Create a volume-grid with the number of dofs indicated and add it to the array of volume-grids.
   ///
   /// If volume-grid exists already then no action will be taken.
   /// This function exists so as to have a single function that creates the volume-grids.
   /// It can be const because the volume-grid container object is mutable.
   void createVolumeGridSlave ( const int numberOfDofs ) const;

   /// Create a nodal-volume-grid with the number of dofs indicated and add it to the array of volume-grids.
   ///
   /// If nodal-volume-grid exists already then no action will be taken.
   /// This function exists so as to have a single function that creates the nodal-volume-grids.
   /// It can be const because the nodal-volume-grid container object is mutable.
   void createNodalVolumeGridSlave ( const int numberOfDofs ) const;

   /// \brief Mapping from number-of-dofs->element-volume-grid.
   mutable ElementVolumeGridArray m_elementVolumeGrids;

   /// \brief Mapping from number-of-dofs->nodal-volume-grid.
   mutable NodalVolumeGridArray m_nodalVolumeGrids;

   
};

inline int LayerProps::getNrOfActiveElements () const {
   return m_nrOfActiveElements;
}


inline bool LayerProps::isActive () const {
   return m_nrOfActiveElements > 0;
}

inline bool LayerProps::isSediment () const {
   return kind () == Interface::SEDIMENT_FORMATION;
}

inline bool LayerProps::isBasement () const {
   return kind () == Interface::BASEMENT_FORMATION;
}

inline bool LayerProps::isSourceRock () const {
   return IsSourceRock;
}

inline bool LayerProps::isMobile () const {
   return IsMobile;
}

inline const GeoPhysics::FluidType* LayerProps::getFluid () const {
   return fluid;
}

inline bool LayerProps::Is_Overpressure_Constrained () const {
   return Hydro_Sand;
}

inline const LayerElementArray& LayerProps::getLayerElements () const {
   return m_elements;
}

inline const LayerElement& LayerProps::getLayerElement ( const int i, 
                                                         const int j,
                                                         const int k ) const {
   // Should check indices.
   return m_elements ( i, j, k );
}


inline LayerElement& LayerProps::getLayerElement ( const int i, 
                                                   const int j,
                                                   const int k ) {
   // Should check indices.
   return m_elements ( i, j, k );
}

inline const ElementVolumeGrid& LayerProps::getComponentLayerVolumes () const {
   return m_componentLayerVolumes;
}

inline const ElementVolumeGrid& LayerProps::getSaturationGrid () const {
   return m_saturationGrid;
}

inline const ElementVolumeGrid& LayerProps::getTimeOfElementInvasionGrid () const {
   return m_timeOfElementInvasionGrid;
}

inline const ElementVolumeGrid& LayerProps::getElementFluxGrid () const {
   return m_elementFluxGrid;
}

// inline Vec LayerProps::getGeneratedMasses () {
//    return m_generatedMasses;
// }


inline const Vec& LayerProps::getComponentVec () const {
   return m_flowComponents;
}

inline Vec& LayerProps::getComponentVec () {
   return m_flowComponents;
}

inline const Vec& LayerProps::getPreviousComponentVec () const {
   return m_previousFlowComponents;
}

inline Vec& LayerProps::getPreviousComponentVec () {
   return m_previousFlowComponents;
}

inline const Vec& LayerProps::getImmobileComponentsVec () const {
   return m_immobileComponents;
}

inline Vec& LayerProps::getImmobileComponentsVec () {
   return m_immobileComponents;
}

inline const Vec& LayerProps::getPhaseSaturationVec () const {
   return m_saturations;
}

inline Vec& LayerProps::getPhaseSaturationVec () {
   return m_saturations;
}

inline const Vec& LayerProps::getPreviousPhaseSaturationVec () const {
   return m_previousSaturations;
}

inline Vec& LayerProps::getPreviousPhaseSaturationVec () {
   return m_previousSaturations;
}


inline Vec& LayerProps::getAveragedSaturations () {
   return m_averagedSaturation;
}

inline const Vec& LayerProps::getAveragedSaturations () const {
   return m_averagedSaturation;
}

inline Vec& LayerProps::getTimeOfElementInvasionVec () {
   return m_timeOfElementInvasionVec;
}

inline const Vec& LayerProps::getTimeOfElementInvasionVec () const {
   return m_timeOfElementInvasionVec;
}

inline const Vec& LayerProps::getTransportedMassesVec () const {
   return m_transportedMasses;
}

inline Vec& LayerProps::getTransportedMassesVec () {
   return m_transportedMasses;
}


inline double LayerProps::getCurrentLayerThickness ( const int i,
                                                     const int j ) const {
   return Current_Properties ( Basin_Modelling::Depth, 0, j, i ) - Current_Properties ( Basin_Modelling::Depth, m_maximumNumberOfElements, j, i );
}

inline const PVTComponents& LayerProps::getMolarMass () const {
   return m_molarMass;
}

inline  bool LayerProps::isChemicalCompactionVesValueIsDefined() const{
	return chemicalCompactionVesValueIsDefined;
}

inline  double LayerProps::getChemicalCompactionVesValue() const{
	return chemicalCompactionVesValue;
}

typedef LayerProps* LayerProps_Ptr;


typedef vector<LayerProps*> LayerList;

#endif /* _FASTCAULDRON__LAYER_H_ */
