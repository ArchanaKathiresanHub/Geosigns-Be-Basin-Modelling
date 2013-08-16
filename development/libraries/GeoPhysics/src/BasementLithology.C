#include "BasementLithology.h"

#include <math.h>
#include <assert.h>
#include "GeoPhysicalConstants.h"
#include "Interface/LithologyHeatCapacitySample.h"
#include "Interface/LithologyThermalConductivitySample.h"
#include "InterfaceInput.h"

#include <sstream>
using std::stringstream;

using namespace GeoPhysics;
using namespace CrustalThicknessInterface;

namespace CrustalThicknessInterface {
void parseLine(const string &theString, const string &theDelimiter, vector<string> &theTokens);
}

//------------------------------------------------------------//
BasementLithology::BasementLithology (  DataAccess::Interface::ProjectHandle* projectHandle, 
                                        database::Record*                     record ) : 
  SimpleLithology( projectHandle, record ){ 
   
   setLithoType();
   
}


//------------------------------------------------------------//
bool BasementLithology::setThermalModel( const string& aThermalModel ) {

   if( aThermalModel == "Low Conductivity Crust" ) {
      if( m_lithotype == CRUST || m_lithotype == BASALT  ) {
         m_thermalcondmodel = m_heatcapmodel = Interface::LOWCOND_MODEL;
      } else {
         cerr << "Wrong property model " << aThermalModel << " for Mantle" << endl;
         return false;
      }
         
   } else if( aThermalModel == "Standard Conductivity Crust") {
      if ( m_lithotype == CRUST || m_lithotype == BASALT ) {
         m_thermalcondmodel =  m_heatcapmodel = Interface::STANDARD_MODEL;
      } else  {
         cerr << "Wrong property model " << aThermalModel << " for Mantle" << endl;
         return false;
      }
   } else if( aThermalModel == "Legacy Crust" ) {
      if(  m_lithotype == MANTLE ) { 
         cerr << "Wrong property model " << aThermalModel << " for Mantle" << endl;
         return false;
      }
   } else if( aThermalModel == "Low Conductivity Mantle") {
      if( m_lithotype == MANTLE || m_lithotype == BASALT ) {   
         m_thermalcondmodel = m_heatcapmodel = Interface::LOWCOND_MODEL;
      } else {
         cerr << "Wrong property model " << aThermalModel << " for Crust" << endl;
         return false;
      }
   } else if( aThermalModel == "Standard Conductivity Mantle") {
      if( m_lithotype == MANTLE || m_lithotype == BASALT ) {
         m_thermalcondmodel = m_heatcapmodel = Interface::STANDARD_MODEL;
      } else {
         cerr << "Wrong property model " << aThermalModel << " for Crust" << endl;
         return false;
      }
   } else if( aThermalModel == "Legacy Mantle" ) {
      if ( m_lithotype == CRUST ) { 
         cerr << "Wrong property model " << aThermalModel << " for Crust" << endl;
         return false;
      }
   } else if( aThermalModel == "High Conductivity Mantle") {
      if( m_lithotype == MANTLE || m_lithotype == BASALT ){
         m_thermalcondmodel = m_heatcapmodel = Interface::HIGHCOND_MODEL;
      } else {
         cerr << "Wrong property model " << aThermalModel << " for Crust" << endl;
         return false;
      } 
   } else {
      cerr << "Unknown property model: " << aThermalModel << endl;
      return false;
   }

   return true;
   

}
//------------------------------------------------------------//
void BasementLithology::setBasementLithoProperties( BasementLithologyProps & aBP ) {
    m_constants = aBP;
}

//------------------------------------------------------------//
void  BasementLithology::setLithoType() {

   if ( m_lithoname == "Crust" ) {
      m_lithotype = CRUST;
   } else if( m_lithoname == "Litho. Mantle" ) {
      m_lithotype = MANTLE;
   } else if( m_lithoname == DataAccess::Interface::ALCBasalt ) {
      m_lithotype = BASALT;
   } else {
      m_lithotype = UNKNOWN;
   }
}


//------------------------------------------------------------//
double BasementLithology::thermCondPointHofmeister (const double inTemperature, const double inPressure) const  {
   // Pa in GPa, K in Kelvin

  double klat, krad;
  const double val1 = - (4 * 1.28 + 1 / 3) * 0.00002;
  const double val2 = 4.6 / 128.1;

  const double pressure = inPressure * 0.001; // convert to GPa
  const double temperature = inTemperature + 273.15; // convert C to K

  klat = 4.70 * pow(298 / temperature, 0.25) * exp(val1 * (temperature - 298.0)) * (1.0 + val2 * pressure);
  krad = 1.753E-02 + temperature * (-1.0365E-04 + temperature * (2.2451E-07 - 3.4071E-11 * temperature));

  return klat + krad;
}  

//------------------------------------------------------------//
double BasementLithology::thermCondPointXu (const double inTemperature, const double inPressure) const  {
   // Pa in GPa, K in Kelvin

  const double pressure = inPressure * 0.001; // convert to GPa
  const double temperature = inTemperature + 273.15; // convert C to K

  double termCond = 4.1 * pow(298.0 / temperature, 0.493) * (1 + 0.032 * pressure);
  return termCond;
} 

//------------------------------------------------------------//
double BasementLithology::thermCondPointWillis (const double inTemperature)  const {
   // Pa in GPa, K in Kelvin

  const double temperature = inTemperature + 273.15; // convert C to K

  double termCond = 4.6738 + temperature * (-0.0069356 + 0.0000031749 * temperature);
  return termCond;
} 

//------------------------------------------------------------//
double BasementLithology::thermalconductivity(double t, double p) const 
{
   double val;

   switch (m_thermalcondmodel) {
   case Interface::CONSTANT_MODEL: 
   case Interface::TABLE_MODEL: {
      if( m_lithotype == BASALT ) {
         val = thermCondBasalt( t );
      } else {
         val = SimpleLithology::thermalconductivity( t );
      }
      return val;
   }
   case Interface::LOWCOND_MODEL: {
      if( m_lithotype == CRUST ) {
         return crustThermCondLow( t );
      } else if ( m_lithotype == MANTLE ) {
         return mantleThermCondLow( t, p );
      } else if( m_lithotype == BASALT ) {
         return thermCondBasalt( t );
      } else {
         cout << m_lithoname << endl;
         assert(0);
      }
   }
   case Interface::HIGHCOND_MODEL: {
      if ( m_lithotype == MANTLE ) {
         return mantleThermCondHigh( t, p );
      } else if( m_lithotype == BASALT ) {
         return thermCondBasalt( t );
      } else {
         cout << m_lithoname << endl;
         assert(0);
      }
   }
   case Interface::STANDARD_MODEL: {
      if( m_lithotype == CRUST ) {
         return crustThermCondStandard( t );
      } else if ( m_lithotype == MANTLE ) {
         return mantleThermCondStandard( t );
      } else if( m_lithotype == BASALT ) {
         return thermCondBasalt( t );
      } else {
         cout << m_lithoname << endl;
         assert(0);
      }
   }
      
   default: {
      cout << m_lithoname << endl;
      assert(0);
   }
   }
   return 0;
   
}

//------------------------------------------------------------//
double BasementLithology::getDensity( double t, double p ) const
{
  double val;

  switch (m_thermalcondmodel) {
  case Interface::TABLE_MODEL:
  case Interface::CONSTANT_MODEL:{
      if( m_lithotype == BASALT ) {
         val = densityBasalt( t, p );
      } else {
         val = SimpleLithology::getDensity();
      }
     return val;
  }
  case Interface::LOWCOND_MODEL: {
     if( m_lithotype == CRUST ) {
        return crustDensityLow( t, p );
     } else if ( m_lithotype == MANTLE ) {
        return mantleDensityLow( t, p );
     } else if( m_lithotype == BASALT ) {
        return densityBasalt( t, p );
     } else {
        cout << m_lithoname << endl;
        assert(0);
     }
  }
  case Interface::HIGHCOND_MODEL: {
     if ( m_lithotype == MANTLE ) {
        return mantleDensityHigh( t, p );
     } else if( m_lithotype == BASALT ) {
        return densityBasalt( t, p );
     } else {
        cout << m_lithoname << endl;
        assert(0);
     }
  }
  case Interface::STANDARD_MODEL: {
     if( m_lithotype == CRUST ) {
        return crustDensityStandard( t, p );
     } else if ( m_lithotype == MANTLE ) {
        return mantleDensityStandard( t, p );
     } else if( m_lithotype == BASALT ) {
        return densityBasalt( t, p );
     } else {
        cout << m_lithoname << endl;
        assert(0);
     }
  }
     
  default: {
     cout << m_lithoname << endl;
     assert(0);
  }
  }
  return 0;
  
}
// Standard properties
//------------------------------------------------------------//

double BasementLithology::crustThermCondStandard (const double  inTemperature) const   {
   // input T in C
   double value = inTemperature + 273.15;
   assert( value >= 0 );

   double thermCond = 4.0 * pow( 273.15 / value, 0.5 ); // W/mK
   /*
   if( inTemperature < 575 ) {
      thermCond = 3.972 - inTemperature * (0.005473 + 3.464e-06 * inTemperature);
   } else {
      thermCond = 1.91 + inTemperature * (8.409e-06 - 9.457e-08 * inTemperature);
   }
   */
  return thermCond;
} 
//------------------------------------------------------------//

double BasementLithology::mantleThermCondStandard (const double  inTemperature) const   {
   // input T in C, k in W/mK
   double value = inTemperature + 273.15;
   assert( value >= 0 );

   double thermCond = 4.881 * pow( 273.15 / value, 0.4062 );

  return thermCond;
} 

//------------------------------------------------------------//
double BasementLithology::crustHeatCapStandard (const double inTemperature) const  {
   // input T in C
   /*
   const double temperature = inTemperature + 273.15;
   if( inTemperature < 575 ) {
      heatCap = 899.54 + 0.3864 * temperature - 5.0e06 / (temperature * temperature);
   } else {
      heatCap = 229.32 + 0.1456 * temperature - 4.7e06 / (temperature * temperature);
   }
   */
   double heatCap = 1418.8 - (742.48 * ( 273.15 / ( inTemperature + 273.15 ))); // J/kgK

   return heatCap;
} 

//------------------------------------------------------------//
double BasementLithology::mantleHeatCapStandard (const double inTemperature) const  {
   // input T in C, Cp in J/kgK
   double value = inTemperature + 273.15;
   assert( value >= 0 );

   double heatCap = 920 * pow( 273.15 / value, - 0.23 );

   return heatCap;
} 

//------------------------------------------------------------//
double BasementLithology::crustDensityStandard(const double  inTemperature, const double inLithostaticPressure) const {
   
   // P in Pa, T in C
   double standardDensity = m_constants.m_csRho * ((1.0 - m_constants.m_csA * inTemperature) + (m_constants.m_csB * inLithostaticPressure * MPa_To_Pa)); // kgm-3

   return standardDensity;
}

//------------------------------------------------------------//
double BasementLithology::mantleDensityStandard(const double  inTemperature, const double inLithostaticPressure) const {
   
   // P in Pa, T in C
   double standardDensity = m_constants.m_mRho * ((1.0 - m_constants.m_mA * inTemperature) + (m_constants.m_mB * inLithostaticPressure * MPa_To_Pa)); // kgm-3

   return standardDensity;
}

// High conductivity properties
//------------------------------------------------------------//
double BasementLithology::mantleThermCondHigh (const double  inTemperature, const double inLithostaticPressure)  const {
   // input T in C, k in W/mK, P in Pa
   double value = inTemperature + 273.15;
   assert( value >= 0 );

   double thermCond = 4.76861 * pow (273.15 / value, 0.3441) * (1 + 2.78482e-11 * inLithostaticPressure * MPa_To_Pa);

  return thermCond;
} 

//------------------------------------------------------------//
double BasementLithology::mantleHeatCapHigh (const double inTemperature) const {
   return mantleHeatCapStandard( inTemperature );
}

//------------------------------------------------------------//
double BasementLithology::mantleDensityHigh(const double  inTemperature, const double inLithostaticPressure)  const {
   return mantleDensityStandard( inTemperature, inLithostaticPressure  );
}

// Low conductivity properties
//------------------------------------------------------------//
double BasementLithology::crustThermCondLow (const double  inTemperature) const {
   // T in C, k in W/mK
   double value = inTemperature + 273.15;
   assert( value >= 0 );

   double thermCond = 3.5 * pow( 273.15/value, 0.75 );

  return thermCond;
} 

//------------------------------------------------------------//
double BasementLithology::mantleThermCondLow (const double  inTemperature, const double inLithostaticPressure) const {
   // T in C, k in W/mK, P in Pa
   double value = inTemperature + 273.15;
   assert( value >= 0 );

   double thermCond = 4.2798 * pow( 273.15/value, 0.493 ) * (1 + 3.125e-11 * inLithostaticPressure * MPa_To_Pa);

  return thermCond;
} 

//------------------------------------------------------------//
double BasementLithology::crustHeatCapLow (const double inTemperature) const {
   // T in C, heatCap in  J/kgK
   double value = inTemperature + 273.15;
   assert( value >= 0 );

   double heatCap = 768.81 * pow( 273.15/ value, -0.33 );

   return heatCap;
}

//------------------------------------------------------------//
double BasementLithology::mantleHeatCapLow (const double inTemperature)  const {
   return  mantleHeatCapStandard( inTemperature );
}

//------------------------------------------------------------//
double BasementLithology::crustDensityLow(const double  inTemperature, const double inLithostaticPressure) const {
   
   return crustDensityStandard( inTemperature, inLithostaticPressure  ); 
}

//------------------------------------------------------------//
double BasementLithology::mantleDensityLow(const double  inTemperature, const double inLithostaticPressure) const {
   return mantleDensityStandard( inTemperature, inLithostaticPressure  ); 
}

//------------------------------------------------------------//
double BasementLithology::densityBasalt(const double  inTemperature, const double inLithostaticPressure) const {
   
   // P in Pa, T in C
   double standardDensity = m_constants.m_bRho * (( 1 -  m_constants.m_bA * inTemperature ) + (  m_constants.m_bB * inLithostaticPressure * MPa_To_Pa)); // kgm-3

   return standardDensity;
}

//------------------------------------------------------------//
double BasementLithology::heatCapBasalt (const double inTemperature) const {
   // T in C, heatCap in  J/kgK
   double value = inTemperature + 273.15;
   assert( value >= 0 );

   double heatCap = 838.5 * pow( 273.15/value, -0.298 );

   return heatCap;
}

//------------------------------------------------------------//
double BasementLithology::thermCondBasalt (const double inTemperature) const {
   // T in C, k in W/mK
   double value = inTemperature + 273.15;
   assert( value >= 0 );

   double termCond = 2.6 * pow( 273.15/value, 0.28 );

   return termCond;
} 

//------------------------------------------------------------//

double BasementLithology::heatcapacity(double t) const
{
   switch (m_heatcapmodel)
    {
    case Interface::TABLE_MODEL:
    case Interface::CONSTANT_MODEL:{
       if( m_lithotype == BASALT ) {
          return heatCapBasalt( t );
       } else { 
          return SimpleLithology::heatcapacity(t);
       }
    }
  case Interface::LOWCOND_MODEL: {
     if( m_lithotype == CRUST ) {
        return crustHeatCapLow( t );
     } else if ( m_lithotype == MANTLE ) {
        return mantleHeatCapLow( t );
     } else if( m_lithotype == BASALT ) {
        return heatCapBasalt( t );
     } else {
        cout << m_lithoname << endl;
        assert(0);
     }
  }
    case Interface::HIGHCOND_MODEL: {
       if ( m_lithotype == MANTLE ) {
          return mantleHeatCapHigh( t );
       } else if( m_lithotype == BASALT ) {
          return heatCapBasalt( t );
       } else {
          cout << m_lithoname << endl;
          assert(0);
       }
    }
    case Interface::STANDARD_MODEL: {
       if( m_lithotype == CRUST ) {
          return crustHeatCapStandard( t );
       } else if ( m_lithotype == MANTLE ) {
          return mantleHeatCapStandard( t );
       } else if( m_lithotype == BASALT ) {
          return heatCapBasalt( t );
       } else {
          cout << m_lithoname << endl;
          assert(0);
       }
    }
    default: {
       cout << m_lithoname << endl;
       assert(0);
    }
    }
   return 0;
}

//------------------------------------------------------------//
bool BasementLithologyProps::loadConfigurationFile( ifstream &ConfigurationFile ) {
   string line;
   vector<string> theTokens;
   string delim = ",";
   
   clean();

   size_t firstNotSpace;
      
   while( !ConfigurationFile.eof() ) {
         
      getline ( ConfigurationFile, line, '\n' );
      if( line.size() != 0 ) { 
         firstNotSpace = line.find_first_not_of(" \t"); 
         
         if( line[firstNotSpace] != '#' ) {
            
            if( line == CrustalThicknessInterface::TableLithoAndCrustProperties || line.find( CrustalThicknessInterface::TableLithoAndCrustProperties, 0) != string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
                     break;
                  }
                  firstNotSpace = line.find_first_not_of(" \t"); 
         
                  if( line[firstNotSpace] != '#' ) {
      
                     CrustalThicknessInterface::parseLine( line, delim, theTokens );
      
                     if( theTokens.size() == 2 ) {
                        if( theTokens[0] == CrustalThicknessInterface::lithosphereThicknessMin ) {
                           m_HLmin = atof( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::maxNumberOfMantleElements ||
                                   theTokens[0] == CrustalThicknessInterface::maxNumberOfMantleElementsOld ) {
                           m_NLMEmax = atoi( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::initNumberOfMantleElements ) {
                           m_InitMEmax = atoi( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::minECT ) {
                           m_ECTmin = atoi( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::minBoundaryLayer ) {
                           m_HCBLmin = atoi( theTokens[1].c_str() );
                        }                    
                     } else {
                        theTokens.clear();
                        string s = "More or less arguments than expected.";
                        cerr << s << endl;
                        return false;
                     }
                     theTokens.clear();
                  }
               }
            } else if( line == CrustalThicknessInterface::TableStandardCrust || line.find( CrustalThicknessInterface::TableStandardCrust, 0) != string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
                     break;
                  }
      
                  firstNotSpace = line.find_first_not_of(" \t"); 
         
                  if( line[firstNotSpace] != '#' ) {
                     CrustalThicknessInterface::parseLine( line, delim, theTokens );
                     
                     if( theTokens.size() == 2 ) {
                        if( theTokens[0] == CrustalThicknessInterface::Rho ) {
                           m_csRho = atof( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::A ) {
                           m_csA = atof( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::B ) {
                           m_csB = atof( theTokens[1].c_str() );
                        }
                        
                     } else {
                        theTokens.clear();
                        string s = "More or less arguments than expected.";
                        cerr << s << endl;
                        return false;
                     }
                     theTokens.clear();
                  }
               }
             } else if( line == CrustalThicknessInterface::TableLowCondCrust || line.find( CrustalThicknessInterface::TableLowCondCrust, 0) != string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
                     break;
                  }
                  firstNotSpace = line.find_first_not_of(" \t"); 
         
                  if( line[firstNotSpace] != '#' ) {
     
                     CrustalThicknessInterface::parseLine( line, delim, theTokens );
                     
                     if( theTokens.size() == 2 ) {
                        if( theTokens[0] == CrustalThicknessInterface::Rho ) {
                           m_clRho = atof( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::A ) {
                           m_clA = atof( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::B ) {
                           m_clB = atof( theTokens[1].c_str() );
                        }
                        
                     } else {
                        theTokens.clear();
                        string s = "More or less arguments than expected.";
                        cerr << s << endl;
                        return false;
                     }
                     theTokens.clear();
                  }
               }
             } else if( line == CrustalThicknessInterface::TableBasalt || line.find( CrustalThicknessInterface::TableBasalt, 0) != string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
                     break;
                  }
      
                  firstNotSpace = line.find_first_not_of(" \t"); 
         
                  if( line[firstNotSpace] != '#' ) {
                     CrustalThicknessInterface::parseLine( line, delim, theTokens );
                     
                     if( theTokens.size() == 2 ) {
                        if( theTokens[0] == CrustalThicknessInterface::Rho ) {
                           m_bRho = atof( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::A ) {
                           m_bA = atof( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::B ) {
                           m_bB = atof( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::T ) {
                           m_bT = atof( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::Heat ) {
                           m_bHeat = atof( theTokens[1].c_str() );
                        }
                        
                     } else {
                        theTokens.clear();
                        string s = "More or less arguments than expected.";
                        cerr << s << endl;
                        return false;
                     }
                     theTokens.clear();
                  }
               }
             } else if( line == CrustalThicknessInterface::TableMantle || line.find( CrustalThicknessInterface::TableMantle, 0) != string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
                     break;
                  }
      
                  firstNotSpace = line.find_first_not_of(" \t"); 
         
                  if( line[firstNotSpace] != '#' ) {
                     CrustalThicknessInterface::parseLine( line, delim, theTokens );
                     
                     if( theTokens.size() == 2 ) {
                        if( theTokens[0] == CrustalThicknessInterface::Rho ) {
                           m_mRho = atof( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::A ) {
                           m_mA = atof( theTokens[1].c_str() );
                        } else if( theTokens[0] == CrustalThicknessInterface::B ) {
                           m_mB = atof( theTokens[1].c_str() );
                        } 
                        
                     } else {
                        theTokens.clear();
                        string s = "More or less arguments than expected.";
                        cerr << s << endl;
                        return false;
                     }
                     theTokens.clear();
                  }
               }
            }
         }
      }
   } 

   const double v = Interface::DefaultUndefinedScalarValue;
  
   if( m_csRho == v || m_clRho == v || m_bRho  == v || m_mRho == v ||
       m_csA   == v || m_clA   == v || m_bA    == v || m_mA   == v ||
       m_csB   == v || m_clB   == v || m_bB    == v || m_mB   == v ||
       m_bT    == v || m_bHeat == v || m_HLmin == v || m_NLMEmax == v ||
       m_InitMEmax == v || m_ECTmin == v || m_HCBLmin == v ) {
      cerr << "Less paramenters than expected are found in ALC_CTC configuraton file." << endl;
      return false;
   }
   /*
   cout << "m: " << m_mRho << "; " << m_mA << "; " << m_mB << "; " <<  endl;
   cout << "cl: " << m_clRho << "; " << m_clA << "; " << m_clB << "; " <<  endl;
   cout << "cs: " << m_csRho << "; " << m_csA << "; " << m_csB << "; " <<  endl;
   cout << "b: " << m_bRho << "; " <<  m_bA << "; " << m_bB << "; " <<  m_bT << "; " << m_bHeat << endl;
   */
   return true;
}
//------------------------------------------------------------//
BasementLithologyProps::BasementLithologyProps() {
   clean();
}
   
//------------------------------------------------------------//
void BasementLithologyProps::clean() {
   
   m_csRho = Interface::DefaultUndefinedScalarValue;
   m_clRho = Interface::DefaultUndefinedScalarValue;
   m_bRho  = Interface::DefaultUndefinedScalarValue;
   m_mRho  = Interface::DefaultUndefinedScalarValue;
   m_csA   = Interface::DefaultUndefinedScalarValue;
   m_csB   = Interface::DefaultUndefinedScalarValue;
   m_clA   = Interface::DefaultUndefinedScalarValue;
   m_clB   = Interface::DefaultUndefinedScalarValue;
   m_bA    = Interface::DefaultUndefinedScalarValue;
   m_bB    = Interface::DefaultUndefinedScalarValue;
   m_mA    = Interface::DefaultUndefinedScalarValue;
   m_mB    = Interface::DefaultUndefinedScalarValue;
   m_bT    = Interface::DefaultUndefinedScalarValue;
   m_bHeat = Interface::DefaultUndefinedScalarValue;

   m_HLmin     = Interface::DefaultUndefinedScalarValue;
   m_NLMEmax   = Interface::DefaultUndefinedScalarValue;
   m_InitMEmax = Interface::DefaultUndefinedScalarValue;
   m_ECTmin    = Interface::DefaultUndefinedScalarValue;
   m_HCBLmin   = Interface::DefaultUndefinedScalarValue;
}

//------------------------------------------------------------//
BasementLithologyProps& BasementLithologyProps::operator=( const BasementLithologyProps& aBP ) {
   
   m_csRho = aBP.m_csRho;
   m_clRho = aBP.m_clRho;
   m_bRho = aBP.m_bRho;
   m_mRho = aBP.m_mRho;
   m_csA = aBP.m_csA;
   m_csB = aBP.m_csB;
   m_clA = aBP.m_clA;
   m_clB = aBP.m_clB;
   m_bA = aBP.m_bA;
   m_bB = aBP.m_bB;
   m_mA = aBP.m_mA;
   m_mB = aBP.m_mB;
   m_bT = aBP.m_bT;
   m_bHeat = aBP.m_bHeat;
   
   m_HLmin = aBP.m_HLmin;
   m_NLMEmax = aBP.m_NLMEmax;
   m_InitMEmax = aBP.m_InitMEmax;
   m_ECTmin = aBP.m_ECTmin;
   m_HCBLmin = aBP.m_ECTmin;
   
   return * this;
}
