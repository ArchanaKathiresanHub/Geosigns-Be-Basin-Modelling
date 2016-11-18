//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS_CONFIGFILEPARAMETER_H_
#define _GEOPHYSICS_CONFIGFILEPARAMETER_H_

#include <fstream>

/// @brief List of strings in the ALC-CTC configuration file
namespace ConfigFileAlcCtc {
   const std::string TableBasicConstants          = "Table:[BasicConstants]";
   const std::string TableLithoAndCrustProperties = "Table:[LithoAndCrustProperties]";
   const std::string TableTemperatureData         = "Table:[TemperatureData]";
   const std::string TableSolidus                 = "Table:[Solidus]";
   const std::string TableMagmaLayer              = "Table:[MagmaLayer]";
   const std::string TableUserDefinedData         = "Table:[UserDefinedData]";
   const std::string TableMantle                  = "Table:[Mantle]";
   const std::string TableStandardCrust           = "Table:[StandardCrust]";
   const std::string TableLowCondCrust            = "Table:[LowCondCrust]";
   const std::string TableBasalt                  = "Table:[Basalt]";
   const std::string EndOfTable                   = "[EndOfTable]";

   const std::string t_0                          = "t_0";
   const std::string t_r                          = "t_r";
   const std::string initialCrustThickness        = "initialCrustThickness";
   const std::string maxBasalticCrustThickness    = "maxBasalticCrustThickness";
   const std::string initialLithosphericThickness = "initialLithosphericThickness";
   const std::string seaLevelAdjustment           = "seaLevelAdjustment";
   const std::string coeffThermExpansion          = "coeffThermExpansion";
   const std::string initialSubsidenceMax         = "initialSubsidenceMax";
   const std::string pi                           = "pi";
   const std::string E0                           = "E0";
   const std::string tau                          = "tau";
   const std::string modelTotalLithoThickness     = "modelTotalLithoThickness";
   const std::string backstrippingMantleDensity   = "backstrippingMantleDensity";
   const std::string lithoMantleDensity           = "lithoMantleDensity";
   const std::string baseLithosphericTemperature  = "baseLithosphericTemperature";
   const std::string referenceCrustThickness      = "referenceCrustThickness";
   const std::string referenceCrustDensity        = "referenceCrustDensity";
   const std::string waterDensity                 = "waterDensity";
   const std::string A                            = "A";
   const std::string B                            = "B";
   const std::string C                            = "C";
   const std::string D                            = "D";
   const std::string E                            = "E";
   const std::string F                            = "F";
   const std::string T                            = "T";
   const std::string Heat                         = "Heat";
   const std::string Rho                          = "Rho";
   const std::string decayConstant                = "decayConstant";
   const std::string lithosphereThicknessMin      = "HLmin";
   const std::string maxNumberOfMantleElements    = "NLMEmax";
}

#endif
