//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BasementLithology.h"

#include "GeoPhysicalConstants.h"
#include "Interface/LithologyHeatCapacitySample.h"
#include "Interface/LithologyThermalConductivitySample.h"

// std library
#include <math.h>
#include <assert.h>
#include <sstream>
using std::stringstream;

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::MegaPaToPa;

using namespace GeoPhysics;

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

void BasementLithology::setBasementLithoProperties( ConfigFileParameterAlc & aBP ) {
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
   double standardDensity = m_constants.m_csRho * ((1.0 - m_constants.m_csA * inTemperature) + (m_constants.m_csB * inLithostaticPressure * MegaPaToPa)); // kgm-3

   return standardDensity;
}
//------------------------------------------------------------//
double BasementLithology::mantleDensityStandard(const double  inTemperature, const double inLithostaticPressure) const {

   // P in Pa, T in C
   double standardDensity = m_constants.m_mRho * ((1.0 - m_constants.m_mA * inTemperature) + (m_constants.m_mB * inLithostaticPressure * MegaPaToPa)); // kgm-3

   return standardDensity;
}
// High conductivity properties
//------------------------------------------------------------//

double BasementLithology::mantleThermCondHigh (const double  inTemperature, const double inLithostaticPressure)  const {
   // input T in C, k in W/mK, P in Pa
   double value = inTemperature + 273.15;
   assert( value >= 0 );

   double thermCond = 4.76861 * pow (273.15 / value, 0.3441) * (1 + 2.78482e-11 * inLithostaticPressure * MegaPaToPa);

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

   double thermCond = 4.2798 * pow( 273.15/value, 0.493 ) * (1 + 3.125e-11 * inLithostaticPressure * MegaPaToPa);

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
   double standardDensity = m_constants.m_bRho * (( 1 -  m_constants.m_bA * inTemperature ) + (  m_constants.m_bB * inLithostaticPressure * MegaPaToPa)); // kgm-3

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