//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_MOCKCONFIGFILEPARAMETERCTC_H_
#define _CRUSTALTHICKNESS_MOCKCONFIGFILEPARAMETERCTC_H_

#include "../src/ConfigFileParameterCtc.h"


namespace CrustalThickness {

   /// @class MockConfigFileParameterCtc Class used for to define constants for unit tests
   class MockConfigFileParameterCtc : public ConfigFileParameterCtc
   {

   public:
      MockConfigFileParameterCtc() :ConfigFileParameterCtc() {};

      /// @defgroup Accessors
      /// @{
      void setCoeffThermExpansion        ( double coef        ) { m_coeffThermExpansion         = coef;        }
      void setInitialSubsidenceMax       ( double subsidence  ) { m_initialSubsidenceMax        = subsidence;  }
      void setE0                         ( double E0          ) { m_E0                          = E0;          }
      void setTau                        ( double tau         ) { m_tau                         = tau;         }
      void setModelTotalLithoThickness   ( double thickness   ) { m_modelTotalLithoThickness    = thickness;   }
      void setBackstrippingMantleDensity ( double density     ) { m_backstrippingMantleDensity  = density;     }
      void setLithoMantleDensity         ( double density     ) { m_lithoMantleDensity          = density;     }
      void setBaseLithosphericTemperature( double temperature ) { m_baseLithosphericTemperature = temperature; }
      void setReferenceCrustThickness    ( double thickness   ) { m_referenceCrustThickness     = thickness;   }
      void setReferenceCrustDensity      ( double density     ) { m_referenceCrustDensity       = density;     }
      void setWaterDensity               ( double density     ) { m_waterDensity                = density;     }
      void setA                          ( double a           ) { m_A                           = a;           }
      void setB                          ( double b           ) { m_B                           = b;           }
      void setC                          ( double c           ) { m_C                           = c;           }
      void setD                          ( double d           ) { m_D                           = d;           }
      void setE                          ( double e           ) { m_E                           = e;           }
      void setF                          ( double f           ) { m_F                           = f;           }
      /// @}

   };

}

#endif

