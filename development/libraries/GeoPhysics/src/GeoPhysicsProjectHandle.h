//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef GEOPHYSICS__PROJECT_HANDLE_H
#define GEOPHYSICS__PROJECT_HANDLE_H

//std
#include <string>
#include <list>
#include <vector>

//CbmGenerics
#include "Polyfunction.h"

#include "CauldronGridDescription.h"

//DataAccess
#include "Local2DArray.h"
#include "ProjectHandle.h"

namespace GeoPhysics {
   class AllochthonousLithologyManager;
   class ConfigFileParameterAlc;
   class GeoPhysicsFormation;
   class FracturePressureCalculator;
   class LithologyManager;
   class ObjectFactory;
}

namespace GeoPhysics {

   class ProjectHandle : public DataAccess::Interface::ProjectHandle
   {
      typedef DataAccess::Interface::Local2DArray <CBMGenerics::Polyfunction> PolyFunction2DArray;

      typedef std::list<double> FloatStack;

      typedef std::vector<int> IntegerArray;

      static const int MaximumNumberOfErrorsPerLayer = 5;

   public :

      typedef DataAccess::Interface::Local2DArray <double> DoubleLocal2DArray;

      ProjectHandle ( database::ProjectFileHandlerPtr pfh,
                      const std::string & name,
                      const DataAccess::Interface::ObjectFactory* objectFactory );

      ~ProjectHandle ();

      /// start a new activity
      bool startActivity ( const std::string& name,
                           const DataAccess::Interface::Grid* grid,
                           bool saveAsInputGrid = false,
                           bool createResultsFile = true,
                           bool append = false );

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
      bool applyFctCorrections () const;

      /// @brief Return true if the project has a paleobathymetrie defined in the
      ///   surface depth history (SDH) for the specified age.
      bool hasSurfaceDepthHistory( const double age ) const;

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

      void printValidNeedles ( std::ostream& o = std::cout ) const;

      double getConstrainedBasaltTemperature() const;

      ConfigFileParameterAlc * getBasementLithologyProps() const;

      /// Compute the age and time step that will be used in permafrost modeling
      bool determinePermafrost( std::vector<double>& timeSteps, std::vector<double>& permafrostAges );

   protected :

      /// Loads the fluid property tables into the fluids.
      // Should this be performed during construction?
      void loadFluidPropertyTables ();

      void loadFracturePressureCalculator ();

      /// Correct the simple (constant) fluid densities to standard conditions.
      void correctSimpleFluidDensities ();

      /// Correct the simple-lithology thermal conductivity tables.
      ///
      /// I do not know the reason for requireing the correction.
      void correctThermalConductivityTables ();

      /// Add the simple-lithologies to the lithology-manager.
      void addSimpleLithologiesToLithologyManager ();

      /// Create the paleo-sea-bottom depth.
      bool createPaleoBathymetry ();

      /// Create the sea-bottom temperature.
      bool createSeaBottomTemperature () const;

      /// Create the mantle heat-flow.
      bool createMantleHeatFlow () const;

      /// Create the crust thickness history.
      void createCrustThickness () const;

      /// Create the crust thickness history (= effective crust thickness) and basalt thickness history.
      bool createBasaltThicknessAndECT () const;

      /// Find the global maximum and minimum in the thickness of each layer.
      bool determineLayerMinMaxThickness ();

      /// Determine the crust thinning ratio.
      // The main reason for this function is to provide easier error handling.
      bool determineCrustThinningRatio () const;

      /// Compute the maximum number of elements that will be used in each layer.
      bool determineMaximumNumberOfSegmentsPerLayer ( const bool readSizeFromVolumeData,
                                                      const bool printTable );

      void setBasinAge ();

      /// Compute the thickness history of the layer.
      ///
      /// Part of the solid-thickness initialisation.
      bool computeThicknessHistories ( const unsigned int i,
                                       const unsigned int j,
                                       GeoPhysics::GeoPhysicsFormation* formation,
                                       IntegerArray& numberOfErrorsPerLayer ) const;

      /// Set the deposition history of the layer.
      ///
      /// Part of the solid-thickness initialisation.
      static bool setDepositionHistory ( const unsigned int     i,
                                         const unsigned int     j,
                                         const double           thickness,
                                         GeoPhysics::GeoPhysicsFormation* formation );

      /// Part of the solid-thickness initialisation.
      bool setHistoriesForUnconformity ( const unsigned int     i,
                                         const unsigned int     j,
                                         const double           thickness,
                                         GeoPhysics::GeoPhysicsFormation* formation ) const;

      /// Part of the solid-thickness initialisation.
      bool setErosionHistory ( const unsigned int i,
                               const unsigned int j,
                               GeoPhysics::GeoPhysicsFormation* formation,
                               const double startErosionAge,
                               const double endErosionAge,
                               const double erodedThickness ) const;

      /// Part of the solid-thickness initialisation.
      bool setMobileLayerThicknessHistory ( const unsigned int i,
                                            const unsigned int j,
                                            GeoPhysics::GeoPhysicsFormation* formation,
                                            IntegerArray&          numberOfErrorsPerLayer ) const;

      /// \brief Part of the solid-thickness initialisation when layer is igneous intrusion.
      static bool setIgneousIntrusionThicknessHistory ( const unsigned int i,
                                                        const unsigned int j,
                                                        GeoPhysics::GeoPhysicsFormation* formation,
                                                        IntegerArray&          numberOfErrorsPerLayer );

      /// Part of the solid-thickness initialisation.
      static void storePresentDayThickness ( const unsigned int i,
                                             const unsigned int j,
                                             GeoPhysics::GeoPhysicsFormation* formation );

      /// Part of the solid-thickness initialisation.
      static bool updateMobileLayerOrIgneousIntrusionMaxVes ( const unsigned int i,
                                                              const unsigned int j,
                                                              GeoPhysics::GeoPhysicsFormation* formation,
                                                              double &maxVes );

      /// Part of the solid-thickness initialisation.
      static bool compFCThicknessHistories ( const unsigned int i,
                                             const unsigned int j,
                                             const bool overpressureCalculation,
                                             GeoPhysics::GeoPhysicsFormation* formation,
                                             int& nrActUnc,
                                             FloatStack &uncMaxVes,
                                             FloatStack &uncThickness );

      /// Part of the solid-thickness initialisation.
      static bool compactLayerThicknessHistory ( const unsigned int i,
                                                 const unsigned int j,
                                                 const bool overpressureCalculation,
                                                 GeoPhysics::GeoPhysicsFormation* formation,
                                                 FloatStack &uncMaxVes,
                                                 FloatStack &uncThickness,
                                                 const int nrActUnc );

      /// Part of the solid-thickness initialisation.
      static bool calcFullCompactedThickness ( const unsigned int i,
                                               const unsigned int j,
                                               const bool overpressureCalculation,
                                               GeoPhysics::GeoPhysicsFormation* formation,
                                               const double compThickness,
                                               double &uncMaxVes,
                                               double &fullCompThickness,
                                               double age );

      /// Load constants from configuration file ( ALC mode )
      bool loadALCConfigurationFile( const string & cfgFileName );

      /// Manages both the simple- and compound-lithologies.
      LithologyManager* m_lithologyManager;

      /// Provides functionality for calculating the fracture-pressure.
      FracturePressureCalculator* m_fracturePressureCalculator{};

      /// Manages the allochthonous-lithologies and the interpolator.
      AllochthonousLithologyManager* m_allochthonousLithologyManager;

      CauldronGridDescription m_cauldronGridDescription;

      // Only made mutable because the evaluate function is not const.
      mutable PolyFunction2DArray m_seaBottomDepth;
      mutable PolyFunction2DArray m_mantleHeatFlow;
      mutable PolyFunction2DArray m_seaBottomTemperature;
      mutable PolyFunction2DArray m_crustThicknessHistory;     ///< The crust thickness (effective crust thickness in case we use the alc)
      mutable PolyFunction2DArray m_basaltThicknessHistory;    ///< The oceanic crust thickness (only in case we use the alc)
      mutable PolyFunction2DArray m_contCrustThicknessHistory; ///< The continental crust thickness (only in case we use the alc)
      mutable DoubleLocal2DArray  m_endOfRiftEvent;


      /// The first indexes in the 'I' direction.
      unsigned int m_firstI [ 2 ]{};

      /// The first indexes in the 'J' direction.
      unsigned int m_firstJ [ 2 ]{};

      /// The last indexes in the 'I' direction.
      unsigned int m_lastI  [ 2 ]{};

      /// The last indexes in the 'J' direction.
      unsigned int m_lastJ  [ 2 ]{};

      /// Indicates whether or not the basin has any active faults.
      bool m_basinHasActiveFaults{};

      /// The age of the basin, i.e. the oldest snapshot time in the snapshot-list.
      double m_basinAge{};

      /// ALC bottom boundary conditions mode
      bool m_isALCMode;
      ConfigFileParameterAlc *m_basementLithoProps{};
      double m_constrainedBasaltTemperature; // defined in configuration file

      private:
         /// @brief If the ALc version used is v2017.05, checks that there are as much continental as oceanic
         /// crustal thicknesses inputs and that they are defined at the same age
         /// @throw std::invalid_argument if the check fails
         void checkAlcCrustHistoryInput() const;

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

inline bool GeoPhysics::ProjectHandle::getBasinHasActiveFaults () const {
   return m_basinHasActiveFaults;
}

//------------------------------------------------------------//

inline double GeoPhysics::ProjectHandle::getAgeOfBasin () const {
   return m_basinAge;
}

inline double GeoPhysics::ProjectHandle::getConstrainedBasaltTemperature() const {
   return m_constrainedBasaltTemperature;
}

//------------------------------------------------------------//

#endif // GEOPHYSICS__PROJECT_HANDLE_H
