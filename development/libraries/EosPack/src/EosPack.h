#ifndef EOSPACK_H
#define EOSPACK_H

///#include "Genex5Framework.h"
#include "ComponentManager.h"
#include "polynomials.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace pvtFlash
{
   enum ComponentId
   {
      FIRST_COMPONENT = 0,
      ASPHALTENES = 0,
      RESINS = 1,
      C15_ARO = 2,
      C15_SAT = 3,
      C6_14ARO = 4,
      C6_14SAT = 5,
      C5 = 6,
      C4 = 7,
      C3 = 8,
      C2 = 9,
      C1 = 10,
      COX = 11,
      N2 = 12,
      H2S = 13,
      LSC = 14,
      C15_AT = 15,
      C6_14BT = 16,
      C6_14DBT = 17,
      C6_14BP = 18,
      C15_AROS = 19,
      C15_SATS = 20,
      C6_14SATS = 21,
      C6_14AROS = 22,
      LAST_COMPONENT = 22,
      UNKNOWN = 23,
      NUM_COMPONENTS = 23
      
    };

   const string ComponentIdNames[] =
    {
      "ASPHALTENES",
      "RESINS",
      "C15_ARO",
      "C15_SAT",
      "C6_14ARO",
      "C6_14SAT",
      "C5",
      "C4",
      "C3",
      "C2",
      "C1",
      "COX",
      "N2",
      "H2S",
      "LSC",
      "C15_AT",
      "C6_14BT",
      "C6_14DBT",
      "C6_14BP",
      "C15_AROS",
      "C15_SATS",
      "C6_14SATS",
      "C6_14AROS"
    };
  
   const int N_PHASES=2;

   enum PVTPhase { VAPOUR_PHASE = 0, LIQUID_PHASE = 1 };

   extern std::string pvtPropertiesConfigFile;

///provides interface to PVT-flash functionality of the EosCauldron implementation
///
///The class EosPack class encapsulates the EosCauldron functionality for PVT-flash
///in the member function compute(), which computes from pressure, temperature and
///component masses the corresponding phase component masses, densities and viscosities.
///In order to get the (only) instance of this class, call member getInstance(), which calls the
///constructor only at first time being called, otherwise it only returns a reference
///to a static object.(singleton-pattern). During the only 
///constructor-execution, the component properties and additional parameters describing 
///the equation of state are read from a configuration file. The properties and parameters are
///possibly GORM-dependent and therefore stored as piecewise polynomials.
  class EosPack
    {
    private:
    
       EosPack();

       int isRK;
       polynomials::PiecewisePolynomial** m_propertyFunc;   //[NUM_COMP][PropertyId] (component-based data)
       //PropertyId=0: molecular weight
       //PropertyId=1: acentric factor
       //PropertyId=2: critical volume
       //PropertyId=3: volume shift
       //PropertyId=4: critical pressure
       //PropertyId=5: critical temperature
       
       polynomials::PiecewisePolynomial* m_omegaA;          //[1], general data
       polynomials::PiecewisePolynomial* m_omegaB;          //[1], general data
       polynomials::PiecewisePolynomial* m_corrLBC;         //[5], general data
       
       bool m_isReadInOk;
       
       int lumpedSpeciesIndex[NUM_COMPONENTS];
    public:
       
       ///returns the only instance of this (singleton) class
       static EosPack& getInstance();
       
       int    getLumpedIndex(int componentId) const;
///computes from pressure, temperature and component masses the corresponding phase component masses, densities and viscosities 
///     
///computes from pressure, temperature and component masses the corresponding phase component masses, densities and viscosities 
///@param[in] temperature   (in K)
///@param[in] pressure      (in Pa)
///@param[in] compMasses [number of components]  masses of feed components (in kg)
///@param[in] isGormPrescribed if true, a prescribed GORM is passed to the function used to evaluate the 
///                    GORM-dependent component properties. if false (default) the GORM is computed as 
///                  the ratio
///                  (mass C1-C5)/(mass of oil components)
///@param[in] gorm             prescribed GORM (only significant when isPrescribed is true)       
///@param[out] phaseCompMasses [gas==0, oil==1][number of components]: masses of components per phase (in kg)
///@param[out] phaseDensity [gas==0, oil==1]:   density per phase (in kg/m3)
///@param[out] phaseViscosity [gas==0, oil==1]:  viscosity per phase (in Pa*s)
       bool compute(double temperature, 
                    double pressure, 
                    double compMasses[],                                        
                    //double  phaseCompMasses[][CBMGenerics::ComponentManager::NumberOfOutputSpecies],
                    double  phaseCompMasses[][CBMGenerics::ComponentManager::NumberOfSpeciesToFlash],
                    double  phaseDensity [],
                    double phaseViscosity[],
                    bool   isGormPrescribed=false,
                    double gorm=0.0 
                    );
       /// Compute with lumped sulphur species into C15+Sat and C6-14Aro
       bool computeWithLumping(double temperature, 
                               double pressure, 
                               double in_compMasses[],  				       
                               double out_phaseCompMasses[][CBMGenerics::ComponentManager::NumberOfOutputSpecies],
                               double phaseDensity [],
                               double phaseViscosity[],
                               bool   isGormPrescribed = false,
                               double gorm = 0.0
                               );   
       
       ///returns gas/oil mass ratio
       double gorm(const double in_compMasses[CBMGenerics::ComponentManager::NumberOfOutputSpecies]);
       
       ///returns the molecular weight of componentId for a prescribed gorm
       double getMolWeight(int componentId, double gorm);
       
       ///returns the molecular weight of componentId for a prescribed gorm. For Sulphur component use MolWeight of component to which it has to be lumped.
       double getMolWeightLumped(int componentId, double gorm);
       
       ///returns the molecular weight of componentId for a prescribed gorm
       double getCriticalTemperature(int componentId, double gorm);
       
       ///returns the molecular weight of componentId for a prescribed gorm
       double getCriticalVolume(int componentId, double gorm);

	    ///returns critical temperature with weight lumped of componentId for a prescribed gorm
       double getCriticalTemperatureLumped(int componentId, double gorm);
       
       ///returns critical volume with weight lumped of componentId for a prescribed gorm
       double getCriticalVolumeLumped(int componentId, double gorm);
       
       /// lumped/unlumped sulphur components before compute 
       /// Size(in_compMasses) = ComponentManager::NumberOfOutputSpecies; Size(out_compMasses) = ComponentManager::NumberOfSpeciesToFlash;
       /// Size(unlump_fraction) = ComponentManager::NumberOfOutputSpecies
       void  lumpComponents(const double in_compMasses[], double out_compMasses[], double unlump_fraction[]);
       
       /// calculate unlumping fractions. 
       /// Size(weights) = ComponentManager::NumberOfOutputSpecies; Size(unlump_fraction) = ComponentManager::NumberOfOutputSpecies
       void  getLumpingFractions(const vector<double>& weights, double unlump_fraction[]);
       
       /// Size(unlump_fraction) = ComponentManager::NumberOfOutputSpecies
       void  unlumpComponents(double in_paseCompMasses[][CBMGenerics::ComponentManager::NumberOfSpeciesToFlash], 
                              double out_phaseCompMasses[][CBMGenerics::ComponentManager::NumberOfOutputSpecies], 
                              double unlump_fraction[]); 
       
       
       ~EosPack();
  };
   
   
   /// Size(weights) = ComponentManager::NumberOfOutputSpecies
   double gorm(const vector<double>& weights);
   
   /// Size(weights) = ComponentManager::NumberOfOutputSpecies
   double getMolWeight(int componentId, const vector<double>& weights);
   
   double criticalTemperatureAccordingToLiMixingRule(const vector<double>& weights, const double& gorm);
   double criticalTemperatureAccordingToLiMixingRuleWithLumping(const vector<double>& weights, const double& gorm);
  

} // namespace pvtFlash

#endif
