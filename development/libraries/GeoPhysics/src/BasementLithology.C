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

BasementLithology::BasementLithology (  DataAccess::Interface::ProjectHandle* projectHandle, 
                                        database::Record*                     record ) : 
  SimpleLithology( projectHandle, record ){ 
   
   setLithoType();
   
}


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

void BasementLithology::setBasementLithoProperties( BasementLithologyProps & aBP ) {
    m_constants = aBP;
}

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

double BasementLithology::thermCondPointXu (const double inTemperature, const double inPressure) const  {
   // Pa in GPa, K in Kelvin

  const double pressure = inPressure * 0.001; // convert to GPa
  const double temperature = inTemperature + 273.15; // convert C to K

  double termCond = 4.1 * pow(298.0 / temperature, 0.493) * (1 + 0.032 * pressure);
  return termCond;
} 

double BasementLithology::thermCondPointWillis (const double inTemperature)  const {
   // Pa in GPa, K in Kelvin

  const double temperature = inTemperature + 273.15; // convert C to K

  double termCond = 4.6738 + temperature * (-0.0069356 + 0.0000031749 * temperature);
  return termCond;
} 

double BasementLithology::thermalconductivity(double t, double p) const 
{

   if(  m_lithotype == BASALT ) {
      return basaltThermalConductivity( t, p );
   }

   switch (m_thermalcondmodel) {

   case Interface::CONSTANT_MODEL: 
   case Interface::TABLE_MODEL: 
      return SimpleLithology::thermalconductivity( t );

   case Interface::LOWCOND_MODEL: {
      if( m_lithotype == CRUST ) {
         return crustThermCondLow( t );
      } else if ( m_lithotype == MANTLE ) {
         return mantleThermCondLow( t, p );
      } else {
         cout << m_lithoname << endl;
         assert(0);
      }
   }
   case Interface::HIGHCOND_MODEL: {
      if ( m_lithotype == MANTLE ) {
         return mantleThermCondHigh( t, p );
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

double BasementLithology::basaltThermalConductivity( double t, double p ) const 
{

   switch ( m_thermalcondmodel ) {

   case Interface::CONSTANT_MODEL: 
   case Interface::TABLE_MODEL: 
      return  thermCondBasalt( t );

   case Interface::LOWCOND_MODEL: 
      return thermCondBasalt( t );

   case Interface::HIGHCOND_MODEL: 
      return thermCondBasalt( t );

   case Interface::STANDARD_MODEL: 
      return thermCondBasalt( t );
      
   default: {
      cout << "basaltThermalConductivity: " <<  m_lithoname << endl;
      assert(0);
   }
   }
   return 0;
   
}


double BasementLithology::getDensity( double t, double p ) const
{
 
  if( m_lithotype == BASALT ) {
     return getBasaltDensity( t, p ) ;
  }

  switch ( m_thermalcondmodel ) {

  case Interface::TABLE_MODEL:
  case Interface::CONSTANT_MODEL:
     return SimpleLithology::getDensity();

  case Interface::LOWCOND_MODEL: {
     if( m_lithotype == CRUST ) {
        return crustDensityLow( t, p );
     } else if ( m_lithotype == MANTLE ) {
        return mantleDensityLow( t, p );
     }  else {
        cout << m_lithoname << endl;
        assert(0);
     }
  }
  case Interface::HIGHCOND_MODEL: {
     if ( m_lithotype == MANTLE ) {
        return mantleDensityHigh( t, p );
     }  else {
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
double BasementLithology::getBasaltDensity( double t, double p ) const
{

  switch (m_thermalcondmodel) {

  case Interface::TABLE_MODEL:
  case Interface::CONSTANT_MODEL:

     return densityBasalt( t, p );

  case Interface::LOWCOND_MODEL: 
     return densityBasalt( t, p );

  case Interface::HIGHCOND_MODEL:
     return densityBasalt( t, p );

  case Interface::STANDARD_MODEL: 
     return densityBasalt( t, p );
     
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

double BasementLithology::heatcapacity(const double t) const
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

bool BasementLithologyProps::loadConfigurationFile( ifstream &ConfigurationFile ) {
   string line;
   vector<string> theTokens;
   string delim = ",";
   
   clean();

   size_t firstNotSpace;
      
   while( !ConfigurationFile.eof() ) {
         
      getline ( ConfigurationFile, line, '\n' );
      if( line.size() != 0 ) { 
         firstNotSpace = line.find_first_not_of(" "); 
         
         if( line[firstNotSpace] != '#' ) {
            
            if( line == CrustalThicknessInterface::TableLithoAndCrustProperties || line.find( CrustalThicknessInterface::TableLithoAndCrustProperties, 0) != string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
                     break;
                  }
      
                  CrustalThicknessInterface::parseLine( line, delim, theTokens );
      
                  if( theTokens.size() == 2 ) {
                     if( theTokens[0] == CrustalThicknessInterface::lithosphereThicknessMin ) {
                        m_HLmin = atof( theTokens[1].c_str() );
                     } else if( theTokens[0] == CrustalThicknessInterface::maxNumberOfMantleElements ) {
                        m_NLMEmax = atoi( theTokens[1].c_str() );
                     }
                     
                  } else {
                     theTokens.clear();
                     string s = "More or less arguments than expected.";
                     cerr << s << endl;
                     return false;
                  }
                  theTokens.clear();
               }
            } else if( line == CrustalThicknessInterface::TableStandardCrust || line.find( CrustalThicknessInterface::TableStandardCrust, 0) != string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
                     break;
                  }
      
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
             } else if( line == CrustalThicknessInterface::TableLowCondCrust || line.find( CrustalThicknessInterface::TableLowCondCrust, 0) != string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
                     break;
                  }
      
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
             } else if( line == CrustalThicknessInterface::TableBasalt || line.find( CrustalThicknessInterface::TableBasalt, 0) != string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
                     break;
                  }
      
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
             } else if( line == CrustalThicknessInterface::TableMantle || line.find( CrustalThicknessInterface::TableMantle, 0) != string::npos ) {
               for(;;) {
                  getline (ConfigurationFile, line, '\n');
        
                  if( line == CrustalThicknessInterface::EndOfTable || line.size() == 0) {
                     break;
                  }
      
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

   /*
   if( m_csRho == 0 || m_clRho == 0 ||  m_bRho == 0 || m_mRho == 0 ||
       m_csA  == 0 || m_clA == 0 || m_bA == 0 || m_mA == 0 ||
       m_csB  == 0 || m_clB == 0 || m_bB == 0 || m_mB == 0 ||
       m_bT == 0|| m_bHeat == 0 || m_HLmin == 0 || m_NLMEmax == 0 ) {
      cerr << "Less arguments than expected in CTC configuraton file." << endl;
      return false;
   }
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
   
   m_csRho = 0;
   m_clRho = 0;
   m_bRho = 0;
   m_mRho = 0;
   m_csA = 0;
   m_csB = 0;
   m_clA = 0;
   m_clB = 0;
   m_bA = 0;
   m_bB = 0;
   m_mA = 0;
   m_mB = 0;
   m_bT = 0;
   m_bHeat = 0;

   m_HLmin = 0;
   m_NLMEmax = 0;

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
   
   return * this;
}
