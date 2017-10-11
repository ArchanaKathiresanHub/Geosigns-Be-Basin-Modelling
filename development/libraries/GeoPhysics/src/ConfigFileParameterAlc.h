//                                                                      
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS_CONFIGFILEPARAMETERALC_H_
#define _GEOPHYSICS_CONFIGFILEPARAMETERALC_H_

#include <fstream>

namespace GeoPhysics {

   /// @class ConfigFileParameterAlc Contains the ALC constants provided by the configuration file
   class ConfigFileParameterAlc
   {
   public:
      ConfigFileParameterAlc();

      /// @brief Load the configuration file ALC parameters
      /// @param ConfigurationFile The configuration file
      bool loadConfigurationFileAlc( std::ifstream &ConfigurationFile );

      double m_csRho;
      double m_clRho;
      double m_bRho;
      double m_mRho;

      double m_csA;
      double m_csB;
      double m_clA;
      double m_clB;
      double m_bA;
      double m_bB;
      double m_mA;
      double m_mB;
      double m_bT;
      double m_bHeat;

      double m_HLmin;
      double m_NLMEmax;

   private:
      void clean();

   };

}

#endif
