#ifndef _GEOPHYSICS__PROJECT_HANDLE_H_
#define _GEOPHYSICS__PROJECT_HANDLE_H_

#include <string>
#include <list>
#include <vector>

#include "Polyfunction.h"

#include "CompoundLithology.h"
#include "CompoundLithologyComposition.h"
#include "CauldronGridDescription.h"
#include "GeoPhysicsFormation.h"
#include "FracturePressureCalculator.h"

#include "Local2DArray.h"

#include "Interface/ProjectHandle.h"


namespace DataAccess {

   namespace Interface {
      class GridMap;
      class Formation;
      class CrustFormation;
      class MantleFormation;
   }

}

namespace GeoPhysics {
   class LithologyManager;
   class AllochthonousLithologyManager;
   class BasementLithologyProps;
}

namespace GeoPhysics {

   class ProjectHandle : public DataAccess::Interface::ProjectHandle {

      typedef GeoPhysics::Local2DArray <CBMGenerics::Polyfunction> PolyFunction2DArray;

      typedef std::list<double> FloatStack;

      typedef std::vector<int> IntegerArray;

      static const int MaximumNumberOfErrorsPerLayer = 5;

   public :

      typedef GeoPhysics::Local2DArray <bool>   BooleanLocal2DArray;
      typedef GeoPhysics::Local2DArray <double> DoubleLocal2DArray;


      ProjectHandle ( database::Database * database, const std::string & name, const std::string & accessMode );

      ~ProjectHandle ();

      /// start a new activity
      bool startActivity ( const std::string& name, const DataAccess::Interface::Grid* grid );

      /// Assign the litholgies to the formations.
      ///
      /// Returns a true unless this fails for any reason then a false will be returned.
      /// This must be done after the activity grid has been assigned,
      /// so that the dimension of the array structures are known.
      virtual bool setFormationLithologies ( const bool canRunGeomorph,
                                             const bool includeFaults );

      /// Initialise the geo-physics project-handle.
      ///
      /// Can be executed only after the startActivity function has been called.
      /// The valid-nodes array is filled, it is constructed according to the input maps.
      /// Further updates may be necessary depending on the application.
      /// \param readSizeFromVolumeData Indicate whether the number of elements in the depth 
      /// should be read from the volume results files or not.
      virtual bool initialise ( const bool readSizeFromVolumeData = false,
                                const bool printTable = true );

      /// Initialises both the solid- and real-thickness arrays.
      bool initialiseLayerThicknessHistory ( const bool overpressureCalculation );

      /// Scale the solid-thicknesses by the correction factor.
      bool applyFctCorrections ();


      /// Return reference to the lithology-manager.
      LithologyManager& getLithologyManager () const;

      /// Return a const reference to the fracture-pressure-calculator.
      const FracturePressureCalculator& getFracturePressureCalculator () const;

      /// Switches the time-dependant lithologies to the current time.
      ///
      /// Must be called whenever the age changes, time-stepping, etc.
      void switchLithologies ( const double age );

      /// Return reference to the allochthonous-lithology-manager.
      AllochthonousLithologyManager& getAllochthonousLithologyManager () const;

      /// Return whether or not the basin has any active faults.
      bool getBasinHasActiveFaults () const;

      // Temporary function, I will change any function that 
      // depends on the grid-description to use a Grid object.
      const CauldronGridDescription& getCauldronGridDescription () const;

      /// Return the age of the basin.
      ///
      /// Will return the oldest snapshot age.
      double getAgeOfBasin () const;

      /// Return the depth of the sea-bottom at the location i, j and at time age.
      double getSeaBottomDepth       ( const unsigned int i,
                                       const unsigned int j,
                                       const double       age ) const;

      /// Return the temperature at the sea-bottom at the location i, j and at time age.
      double getSeaBottomTemperature ( const unsigned int i,
                                       const unsigned int j,
                                       const double       age ) const;

      /// Return the mantle heat-flow at the location i, j and at time age.
      double getMantleHeatFlow       ( const unsigned int i,
                                       const unsigned int j,
                                       const double       age ) const;

      /// in ALC - return the thickness of the continental crust at the location i, j and at time age.
      double getContCrustThickness       ( const unsigned int i,
                                           const unsigned int j,
                                           const double       age ) const;

      /// Return the thickness of the basalt at the location i, j and at time age.
      double getBasaltThickness       ( const unsigned int i,
                                        const unsigned int j,
                                        const double       age ) const;

      /// Return the thickness of the basalt in the Mantle layer at the location i, j and at time age.
      double getBasaltInMantleThickness       ( const unsigned int i,
                                                const unsigned int j,
                                                const double       age ) const;

      /// Return the depth thickness of the crust at the location i, j and at time age.
      /// In ALC - return effective crustal thickness
      double getCrustThickness       ( const unsigned int i,
                                       const unsigned int j,
                                       const double       age ) const;
      /// In ALC - return the max model lithosphere thickness. Below it the temperature is fixed as the bottom of mantle temperature
      double getLithosphereThicknessMod ( const unsigned int i,
                                          const unsigned int j,
                                          const double       age ) const;
      
      /// In ALC - return the calculated age of the rift event
      double getEndOfRiftEvent ( const unsigned int i,
                                 const unsigned int j ) const;
      
      /// Return the maximum thickness attained by the basement.
      double getMaximumBasementThickness () const;
      
      /// Return whether or not the node is defined.
      bool getNodeIsValid ( const unsigned int i, const unsigned int j ) const;

      /// Return the first index in the 'I' direction.
      ///
      /// \param includeGhosts Whether or not to include the ghost nodes in the boundary.
      unsigned int firstI ( const bool includeGhosts = false ) const;

      /// Return the first index in the 'J' direction.
      ///
      /// \param includeGhosts Whether or not to include the ghost nodes in the boundary.
      unsigned int firstJ ( const bool includeGhosts = false ) const;

      /// Return the last index in the 'I' direction.
      ///
      /// \param includeGhosts Whether or not to include the ghost nodes in the boundary.
      unsigned int lastI  ( const bool includeGhosts = false ) const;

      /// Return the last index in the 'J' direction.
      ///
      /// \param includeGhosts Whether or not to include the ghost nodes in the boundary.
      unsigned int lastJ  ( const bool includeGhosts = false ) const;

      void printValidNeedles ( std::ostream& o = std::cout );

      bool isALC() const;

      double getMinimumLithosphereThickness() const;
      int    getMaximumNumberOfMantleElements() const;
      double getConstrainedBasaltTemperature() const;

      BasementLithologyProps * getBasementLithologyProps() const;
   protected :

      /// Loads the fluid property tables into the fluids.
      // Should this be performed during construction?
      void loadFluidPropertyTables ();

      /// 
      void loadFracturePressureCalculator ();

      void deleteFracturePressureCalculator ();


      /// Correct the simple (constant) fluid densities to standard conditions.
      void correctSimpleFluidDensities ();

      /// Correct the simple-lithology thermal conductivity tables.
      ///
      /// I do not know the reason for requireing the correction.
      void correctThermalConductivityTables ();

      /// Add the simple-lithologies to the lithology-manager.
      void addSimpleLithologiesToLithologyManager ();

      /// Add undefined areas of a grid-map to the undefined-node array.
      void addUndefinedAreas ( const DataAccess::Interface::GridMap* theMap );

      /// Add undefined areas of formation input maps to the undefined-node array.
      ///
      /// Input maps include:
      ///   o Lithology maps;
      ///   o Mobile layer thickness maps;
      ///   o Allochthonous lithology distribution maps;
      ///   o All reservoir maps.
      void addFormationUndefinedAreas ( const DataAccess::Interface::Formation* formation );

      /// Add undefined areas of the mantle-formation input maps to the undefined-node array.
      ///
      /// Input maps include:
      ///   o Paleo-thickness history;
      ///   o Heat-flow history maps.
      void addMantleUndefinedAreas ( const DataAccess::Interface::MantleFormation* mantle );

      /// Add undefined areas of the crust-formation input maps to the undefined-node array.
      ///
      /// Input maps are paleo-thickness history.
      void addCrustUndefinedAreas ( const DataAccess::Interface::CrustFormation* crust );

      /// Add undefined areas from the property-value map.
      ///
      /// The undefined areas may depend on 
      void addUndefinedAreas ( const DataAccess::Interface::PropertyValue* theProperty );


      /// Initialise the valid-node array.
      ///
      /// This function uses only the input data maps and is sufficient to construct
      /// all of the necessary items in the cauldron model, e.g. sea-surface temperature,
      /// crust-thickness history, etc. It may be necessary to restrict further the
      /// valid nodes with other input data, e.g. fct-correction maps, or ves property.
      virtual bool initialiseValidNodes ( const bool readSizeFromVolumeData );

      /// Create the paleo-sea-bottom depth.
      bool createPaleoBathymetry ();

      /// Create the sea-bottom temperature.
      bool createSeaBottomTemperature ();

      /// Create the mantle heat-flow.
      bool createMantleHeatFlow ();

      /// Create the crust thickness history.
      bool createCrustThickness ();

      /// Create the crust thickness history (= effective crust thickness) and basalt thickness history.
      bool createBasaltThicknessAndECT ();

      /// Find the global maximum and minimum in the thickness of each layer.
      bool determineLayerMinMaxThickness ();

      /// Determine the crust thinning ratio.
      // The main reason for this function is to provide easier error handling.
      bool determineCrustThinningRatio ();

      /// \brief If necessary add a crust thickness property to the sequence of maps at the age at which the simulation starts.
      bool correctCrustThicknessHistory ();

      /// Compute the maximum number of elements that will be used in each layer.
      bool determineMaximumNumberOfSegmentsPerLayer ( const bool readSizeFromVolumeData,
                                                      const bool printTable );

      void setBasinAge ();

      /// Compute the thickness history of the layer.
      ///
      /// Part of the solid-thickness initialisation.
      bool computeThicknessHistories ( const unsigned int i,
                                       const unsigned int j,
                                             GeoPhysics::Formation* formation,
                                             IntegerArray& numberOfErrorsPerLayer );

      /// Set the deposition history of the layer.
      ///
      /// Part of the solid-thickness initialisation.
      bool setDepositionHistory ( const unsigned int i,
                                  const unsigned int j,
                                  const double       thickness,
                                        GeoPhysics::Formation* formation );

      /// Part of the solid-thickness initialisation.
      bool setHistoriesForUnconformity ( const unsigned int i,
                                         const unsigned int j,
                                         const double       thickness,
                                               GeoPhysics::Formation* formation );

      /// Part of the solid-thickness initialisation.
      bool setErosionHistory ( const unsigned int i,
                               const unsigned int j,
                                     GeoPhysics::Formation* formation,
                               const double startErosionAge,
                               const double endErosionAge,
                               const double erodedThickness );

      /// Part of the solid-thickness initialisation.
      bool setMobileLayerThicknessHistory ( const unsigned int i,
                                            const unsigned int j,
                                                  GeoPhysics::Formation* formation,
                                                  IntegerArray&          numberOfErrorsPerLayer );

      /// \brief Part of the solid-thickness initialisation when layer is igneous intrusion.
      bool setIgneousIntrusionThicknessHistory ( const unsigned int i,
                                                 const unsigned int j,
                                                       GeoPhysics::Formation* formation,
                                                       IntegerArray&          numberOfErrorsPerLayer );

      /// Part of the solid-thickness initialisation.
      void storePresentDayThickness ( const unsigned int i,
                                      const unsigned int j,
                                            GeoPhysics::Formation* formation );

      /// Part of the solid-thickness initialisation.
      bool updateMobileLayerOrIgneousIntrusionMaxVes ( const unsigned int i,
                                                       const unsigned int j,
                                                       GeoPhysics::Formation* formation,
                                                       double &maxVes );

      /// Part of the solid-thickness initialisation.
      bool compFCThicknessHistories ( const unsigned int i,
                                      const unsigned int j,
                                      const bool     overpressureCalculation,
                                            GeoPhysics::Formation* formation,
                                            int& nrActUnc, 
                                            FloatStack &uncMaxVes, 
                                            FloatStack &uncThickness );

      /// Part of the solid-thickness initialisation.
      bool compactLayerThicknessHistory ( const unsigned int i,
                                          const unsigned int j,
                                          const bool     overpressureCalculation,
                                                GeoPhysics::Formation* formation,
                                                FloatStack &uncMaxVes,
                                                FloatStack &uncThickness,
                                          const int nrActUnc );

      /// Part of the solid-thickness initialisation.
      bool calcFullCompactedThickness ( const unsigned int i,
                                        const unsigned int j,
                                        const bool     overpressureCalculation,
                                              GeoPhysics::Formation* formation,
                                        const double compThickness,
                                              double &uncMaxVes,
                                              double &fullCompThickness );

      /// Load constants from configuration file ( ALC mode )
      bool loadALCConfigurationFile( const string & cfgFileName );

      /// Manages both the simple- and compound-lithologies.
      LithologyManager* m_lithologyManager;

      /// Provides functionality for calculating the fracture-pressure.
      FracturePressureCalculator* m_fracturePressureCalculator;

      /// Manages the allochthonous-lithologies and the interpolator.
      AllochthonousLithologyManager* m_allochthonousLithologyManager;

      /// Indicates whether a node is valid or not.
      ///
      /// I.e. whether it is included in the calculation.
      BooleanLocal2DArray m_validNodes;

      CauldronGridDescription m_cauldronGridDescription;

      // Only made mutable because the evaluate function is not const.
      mutable PolyFunction2DArray m_seaBottomDepth;
      mutable PolyFunction2DArray m_mantleHeatFlow;
      mutable PolyFunction2DArray m_seaBottomTemperature;
      mutable PolyFunction2DArray m_crustThicknessHistory;
      mutable PolyFunction2DArray m_contCrustThicknessHistory;
      mutable PolyFunction2DArray m_basaltThicknessHistory;
      mutable DoubleLocal2DArray  m_endOfRiftEvent;

            
      /// The first indexes in the 'I' direction.
      unsigned int m_firstI [ 2 ];

      /// The first indexes in the 'J' direction.
      unsigned int m_firstJ [ 2 ];

      /// The last indexes in the 'I' direction.
      unsigned int m_lastI  [ 2 ];

      /// The last indexes in the 'J' direction.
      unsigned int m_lastJ  [ 2 ];

      /// Indicates whether or not the basin has any active faults.
      bool m_basinHasActiveFaults;

      /// The age of the basin, i.e. the oldest snapshot time in the snapshot-list.
      double m_basinAge;

      /// ALC bottom boundary conditions mode
      bool m_isALCMode;   
      BasementLithologyProps *m_basementLithoProps;
      double m_minimumLithosphereThickness; // defined in configuration file
      int    m_maximumNumberOfMantleElements; // defined in configuration file
      double m_constrainedBasaltTemperature; // defined in configuration file

   }; 

}

//------------------------------------------------------------//
//   Inline functions.
//------------------------------------------------------------//

inline const GeoPhysics::CauldronGridDescription& GeoPhysics::ProjectHandle::getCauldronGridDescription () const {
   return m_cauldronGridDescription;
}

//------------------------------------------------------------//

inline unsigned int GeoPhysics::ProjectHandle::firstI ( const bool includeGhosts ) const {
   return m_firstI [ includeGhosts ? 1 : 0 ];
}

//------------------------------------------------------------//

inline unsigned int GeoPhysics::ProjectHandle::firstJ ( const bool includeGhosts ) const {
   return m_firstJ [ includeGhosts ? 1 : 0 ];
}

//------------------------------------------------------------//

inline unsigned int GeoPhysics::ProjectHandle::lastI  ( const bool includeGhosts ) const {
   return m_lastI [ includeGhosts ? 1 : 0 ];
}

//------------------------------------------------------------//

inline unsigned int GeoPhysics::ProjectHandle::lastJ  ( const bool includeGhosts ) const {
   return m_lastJ [ includeGhosts ? 1 : 0 ];
}

//------------------------------------------------------------//

inline bool GeoPhysics::ProjectHandle::getNodeIsValid ( const unsigned int i, const unsigned int j ) const {
   return m_validNodes ( i, j );
}

//------------------------------------------------------------//

inline bool GeoPhysics::ProjectHandle::getBasinHasActiveFaults () const {
   return m_basinHasActiveFaults;
}

//------------------------------------------------------------//

inline double GeoPhysics::ProjectHandle::getAgeOfBasin () const {
   return m_basinAge;
}

inline bool GeoPhysics::ProjectHandle::isALC() const {
   return m_isALCMode;
}

inline double GeoPhysics::ProjectHandle::getMinimumLithosphereThickness() const {
   return m_minimumLithosphereThickness;
}

inline int GeoPhysics::ProjectHandle::getMaximumNumberOfMantleElements() const {
   return m_maximumNumberOfMantleElements;
}

inline double GeoPhysics::ProjectHandle::getConstrainedBasaltTemperature() const {
   return m_constrainedBasaltTemperature;
}

//------------------------------------------------------------//

#endif // _GEOPHYSICS__PROJECT_HANDLE_H_
