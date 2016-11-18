// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef EOSPACK_H
#define EOSPACK_H

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;
typedef CBMGenerics::ComponentManager::PhaseId PhaseId;

// Eospack library
#include "polynomials.h"

// std library
#include <vector>
#include <string>
using std::vector;
using std::string;

namespace pvtFlash
{

   extern std::string pvtPropertiesConfigFile;
   constexpr int numberOfPhases = PhaseId::NUMBER_OF_PHASES;

   /// \brief provides interface to PVT-flash functionality of the EosCauldron implementation
   /// The class EosPack class encapsulates the EosCauldron functionality for PVT-flash
   /// in the member function compute(), which computes from pressure, temperature and
   /// component masses the corresponding phase component masses, densities and viscosities.
   /// In order to get the (only) instance of this class, call member getInstance(), which calls the
   /// constructor only at first time being called, otherwise it only returns a reference
   /// to a static object.(singleton-pattern). During the only 
   /// constructor-execution, the component properties and additional parameters describing 
   /// the equation of state are read from a configuration file. The properties and parameters are
   /// possibly GORM-dependent and therefore stored as piecewise polynomials.
   class EosPack
   {
   public:
      /// \brief returns the only instance of this (singleton) class
      static EosPack& getInstance();

      static const char * getEosPackDir ();
 
      /// \brief In multithreading cases we need separate instance for each thread. Creates such instance
      static EosPack* createNewInstance() { return new EosPack(); }
       
      int getLumpedIndex( int componentId ) const;

      /// \brief Computes from pressure, temperature and component masses the corresponding phase component masses, densities and viscosities
      /// \param[in] temperature   (in K)
      /// \param[in] pressure      (in Pa)
      /// \param[in] compMasses [number of components]  masses of feed components (in kg)
      /// \param[in] isGormPrescribed if true, a prescribed GORM is passed to the function used to evaluate the 
      ///            GORM-dependent component properties. if false (default) the GORM is computed as 
      ///            the ratio
      ///            (mass C1-C5)/(mass of oil components)
      /// \param[in]  gorm prescribed GORM (only significant when isPrescribed is true)       
      /// \param[out] phaseCompMasses [gas==0, oil==1][number of components]: masses of components per phase (in kg)
      /// \param[out] phaseDensity [gas==0, oil==1]:   density per phase (in kg/m3)
      /// \param[out] phaseViscosity [gas==0, oil==1]:  viscosity per phase (in cP)
      /// \param[in,out] kValues Initialise the flash newton solve with a set of k-values.
      ///                The array must have at least as many entries as there are species modelled in the flasher.
      ///                If this array is null then the normal initialisation will occur and the k-values will not be passed back to the calling procedure.
      ///                If the array is not null and the first value is -1.0 then normal initialisation will occur
      ///                and the k-values will be stored in the array.
      bool compute( double temperature, 
                    double pressure, 
                    double compMasses[],
                    double phaseCompMasses[][ComponentId::NUMBER_OF_SPECIES_TO_FLASH],
                    double phaseDensity[],
                    double phaseViscosity[],
                    bool   isGormPrescribed = false,
                    double gorm = 0.0,
                    double* kValues = 0
                  );

      /// \brief Compute with lumped sulphur species into C15+Sat and C6-14Aro
      bool computeWithLumping( double temperature, 
                               double pressure, 
                               double in_compMasses[],  				       
                               double out_phaseCompMasses[][ComponentId::NUMBER_OF_SPECIES],
                               double phaseDensity [],
                               double phaseViscosity[],
                               bool   isGormPrescribed = false,
                               double gorm = 0.0,
                               double* pKValues = 0
                             );

      /// \brief returns gas/oil mass ratio
      double gorm( const double in_compMasses[ComponentId::NUMBER_OF_SPECIES] );
       
      /// \brief returns the molecular weight of componentId for a prescribed gorm
      double getMolWeight( int componentId, double gorm );
       
      /// \brief returns the molecular weight of componentId for a prescribed gorm. For Sulphur component use MolWeight of component to which it has to be lumped.
      double getMolWeightLumped( int componentId, double gorm );
       
      /// \brief returns the molecular weight of componentId for a prescribed gorm
      double getCriticalTemperature( int componentId, double gorm );
       
      /// \brief returns the molecular weight of componentId for a prescribed gorm
      double getCriticalVolume( int componentId, double gorm );

      /// \brief returns critical temperature with weight lumped of componentId for a prescribed gorm
      double getCriticalTemperatureLumped( int componentId, double gorm );

      /// \brief returns critical volume with weight lumped of componentId for a prescribed gorm
      double getCriticalVolumeLumped( int componentId, double gorm );

      /// \brief lump/unlump sulphur components before compute 
      /// \param[in] in_compMasses input array of size ComponentManager::NUMBER_OF_SPECIES with component masses
      /// \param[out] out_compMasses output array of size ComponentManager::NumberOfSpeciesToFlash with lumped component masses
      /// \param unlump_fraction array of size ComponentManager::NUMBER_OF_SPECIES with components fractions
      void lumpComponents( const double in_compMasses[], double out_compMasses[], double unlump_fraction[] );

      /// \brief calculate unlumping fractions. 
      /// \param[in] weights array of size ComponentManager::NUMBER_OF_SPECIES
      /// \param[out] unlump_fraction array of size ComponentManager::NUMBER_OF_SPECIES
      void getLumpingFractions( const vector<double>& weights, double unlump_fraction[] );

      /// \param[in] in_paseCompMasses masses for each lumped component for each phase
      /// \param[out] out_phaseCompMasses masses for each unlumped component for each phase
      /// \param unlump_fraction array of size ComponentManager::NUMBER_OF_SPECIES
      void  unlumpComponents( double in_paseCompMasses[][ComponentId::NUMBER_OF_SPECIES_TO_FLASH], 
                              double out_phaseCompMasses[][ComponentId::NUMBER_OF_SPECIES], 
                              double unlump_fraction[]);

      /// \brief Change the default value which is used for single phase lableing in PVT library. Also this call
      ///        changes method to labeling single phase to 
      /// \param val the new value. The default value is 5.0
      void setCritAoverBterm( double val );

      /// \brief Cange back to default value CritAoverB term and single phase labeling method
      void resetToDefaultCritAoverBterm();

      /// \brief Change nonlinear solver maximal number of iteration and convergence tolerance
      /// \param maxItersNum new value for maximal iterations number (default is 50)
      /// \param stopTol new value for stop tolerance (default is 1e-4)
      /// \param newtonRelCoeff new value for relaxation coefficient (default is 1.0, possible values: 0 < RelCoeff <= 1.0 )
      void setNonLinearSolverConvParameters( int maxItersNum = 50, double stopTol = 1.e-4, double newtonRelCoeff = 1.0 );

      ~EosPack();

   private:
      EosPack();

      int m_isRK;

      /// \brief [NUM_COMP][PropertyId] (component-based data)
      /// \details PropertyId=0: molecular weight
      ///          PropertyId=1: acentric factor
      ///          PropertyId=2: critical volume
      ///          PropertyId=3: volume shift
      ///          PropertyId=4: critical pressure
      ///          PropertyId=5: critical temperature
      polynomials::PiecewisePolynomial** m_propertyFunc;

      polynomials::PiecewisePolynomial* m_omegaA;          // [1], general data
      polynomials::PiecewisePolynomial* m_omegaB;          // [1], general data
      polynomials::PiecewisePolynomial* m_corrLBC;         // [5], general data

      bool m_isReadInOk;

      int    m_phaseIdMethod;    ///< Method for labeling a single phase
      double m_CritAoverB;       ///< if m_phaseIdMethode is set to EOS_SINGLE_PHASE_AOVERB, this value will 
                                 ///  be used for labeling single phase as liquid or vapor

      int    m_maxItersNum;      ///< maximal iterations number for the nonlinear solver
      double m_stopTolerance;    ///< convergence stop tolerance for nonliner solver
      double m_NewtonRelaxCoeff; ///< relaxation coefficient for Newton solver  min( 1.0, RelCoef * 1.1 * IterNum )

      int lumpedSpeciesIndex[ComponentId::NUMBER_OF_SPECIES];
   };
      
   /// Size(weights) = ComponentManager::NUMBER_OF_SPECIES
   double gorm( const vector<double>& weights );
   
   /// Size(weights) = ComponentManager::NUMBER_OF_SPECIES
   double getMolWeight( int componentId, const vector<double>& weights );

   /// Crtical Temperature per component
   double getCriticalTemperature (int componentId, double gorm);

   double criticalTemperatureAccordingToLiMixingRule           ( const vector<double>& weights, const double& gorm );
   double criticalTemperatureAccordingToLiMixingRuleWithLumping( const vector<double>& weights, const double& gorm );

} // namespace pvtFlash

#endif
