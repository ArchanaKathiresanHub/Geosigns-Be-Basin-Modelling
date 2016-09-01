//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_CONFIGFILEPARAMETERCTC_H_
#define _CRUSTALTHICKNESS_CONFIGFILEPARAMETERCTC_H_

#include "ConfigFileParameter.h"

// utilities
#include "FormattingException.h"

namespace CrustalThickness {

   /// @class ConfigFileParameterCtc Contains the CTC constants provided by the configuration file
   class ConfigFileParameterCtc
   {
      typedef formattingexception::GeneralException ConfigFileCtcException;

   public:
      ConfigFileParameterCtc();

      /// @defgroup Accessors
      /// @{
      double getCoeffThermExpansion()         const { return m_coeffThermExpansion;         };
      double getInitialSubsidenceMax()        const { return m_initialSubsidenceMax;        };
      double getE0()                          const { return m_E0;                          };
      double getTau()                         const { return m_tau;                         };
      double getModelTotalLithoThickness()    const { return m_modelTotalLithoThickness;    };
      double getBackstrippingMantleDensity()  const { return m_backstrippingMantleDensity;  };
      double getLithoMantleDensity()          const { return m_lithoMantleDensity;          };
      double getBaseLithosphericTemperature() const { return m_baseLithosphericTemperature; };
      double getReferenceCrustThickness()     const { return m_referenceCrustThickness;     };
      double getReferenceCrustDensity()       const { return m_referenceCrustDensity;       };
      double getWaterDensity()                const { return m_waterDensity;                };
      double getA()                           const { return m_A;                           };
      double getB()                           const { return m_B;                           };
      double getC()                           const { return m_C;                           };
      double getD()                           const { return m_D;                           };
      double getE()                           const { return m_E;                           };
      double getF()                           const { return m_F;                           };
      double getDecayConstant()               const { return m_decayConstant;               };
      /// @}

      /// @defgroup Mutators
      /// @{
      void setDecayConstant( const double decayConstant ) { m_decayConstant = decayConstant; };
      /// @}

      ConfigFileParameterCtc& operator=(const ConfigFileParameterCtc& toCopy);

      /// @brief Load the configuration file CTC parameters
      /// @param ConfigurationFile The configuration file
      void loadConfigurationFileCtc( const std::string &inFile );

   private:

      /// @defgroup LoadData_cfg
      ///    Load data from configuration file
      /// @{
      void loadBasicConstants         ( std::ifstream &ConfigurationFile );
      void loadLithoAndCrustProperties( std::ifstream &ConfigurationFile );
      void loadTemperatureData        ( std::ifstream &ConfigurationFile );
      void loadSolidus                ( std::ifstream &ConfigurationFile );
      void loadMagmaLayer             ( std::ifstream &ConfigurationFile );
      /// @}

   protected:

      /// @defgroup Basic_constants
      /// @{
      double m_coeffThermExpansion;   ///< Thermal expension coefficeint
      double m_initialSubsidenceMax;  ///< Maximum initial subsidence
      double m_E0;                    ///< E0 for McKenzie equations
      double m_tau;                   ///< Tau for McKenzie equations
      /// @}

      /// @defgroup Lithosphere_and_crust_properties
      /// @{
      double m_modelTotalLithoThickness;    ///< Total lithospher thickness (crust + lithospheric mantle)
      /// @todo Why do we use two mantle and crust density?
      double m_backstrippingMantleDensity;  ///< The backstriped lithospheric mantle density
      double m_lithoMantleDensity;          ///< The lithospheric mantle density
      double m_baseLithosphericTemperature; ///< The bottom lithospheric mantle temperature
      double m_referenceCrustThickness;     ///< The reference continental crust thickness
      double m_referenceCrustDensity;       ///< The reference continental crust density
      double m_waterDensity;                ///< The water density
      /// @}

      /// @defgroup Asthenosphere_potential_temperature_data
      /// @{
      double m_A;
      double m_B;
      /// @}

      /// @defgroup Solidus_data
      ///    onsetof adiabatic melting
      /// @{
      double m_C;
      double m_D;
      /// @}

      /// @defgroup Magma-layer_density_data
      /// @{
      double m_E;
      double m_F;
      double m_decayConstant;
      /// @}

      /// @brief Clean all class members
      void clean();

   };

}

#endif

