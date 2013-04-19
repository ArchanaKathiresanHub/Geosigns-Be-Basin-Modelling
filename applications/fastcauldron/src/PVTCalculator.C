#include "PVTCalculator.h"

#include <sstream>
#include <iomanip>

#include "NumericFunctions.h"
#include "ComponentManager.h"

PVTCalc* PVTCalc::m_theInstance = 0;



PVTComponents::PVTComponents () {
   zero ();
}

void PVTComponents::zero () {
   fill ( 0.0 );
}

void PVTComponents::fill ( const double withTheValue ) {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      m_components [ i ] = withTheValue;
   }

}

PVTComponents& PVTComponents::operator=( const double value ) {

   fill ( value );

   return *this;
}


PVTComponents& PVTComponents::operator=( const PVTComponents& values ) {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      m_components [ i ] = values.m_components [ i ];
   }

   return *this;
}

double PVTComponents::sum () const {

   double result = 0.0;
   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      result += m_components [ i ];
   }

   return result;
}

double PVTComponents::sumRatios ( const PVTComponents& divisors ) const {

   double result = 0.0;
   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      result += m_components [ i ] / divisors.m_components [ i ];
   }

   return result;
}

double PVTComponents::sum ( const pvtFlash::PVTPhase phase ) const {

   double result = 0.0;
   int i;

   if ( phase == pvtFlash::VAPOUR_PHASE ) {

      for ( i = pvtFlash::C5; i <= pvtFlash::C1; ++i ) {
         result += m_components [ i ];
      }

   } else {

      for ( i = pvtFlash::ASPHALTENES; i <= pvtFlash::C6_14SAT; ++i ) {
         result += m_components [ i ];
      }

   }

   return result;
}

double dot ( const PVTComponents& components1,
             const PVTComponents& components2 ) {

   double result = 0.0;
   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      result += components1.m_components [ i ] * components2.m_components [ i ];
   }

   return result;
}


PVTComponents& PVTComponents::operator += ( const PVTComponents& components ) {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      m_components [ i ] += components.m_components [ i ];
   }

   return *this;
}

PVTComponents& PVTComponents::operator-= ( const PVTComponents& components ) {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      m_components [ i ] -= components.m_components [ i ];
   }

   return *this;
}

PVTComponents& PVTComponents::operator*= ( const PVTComponents& components ) {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      m_components [ i ] *= components.m_components [ i ];
   }

   return *this;
}

PVTComponents& PVTComponents::operator/= ( const PVTComponents& components ) {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      m_components [ i ] /= components.m_components [ i ];
   }

   return *this;
}

PVTComponents& PVTComponents::operator += ( const double value ) {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      m_components [ i ] += value;
   }

   return *this;
}

PVTComponents& PVTComponents::operator *= ( const double scale ) {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      m_components [ i ] *= scale;
   }

   return *this;
}

std::string PVTComponents::image ( const bool reverseOrder ) const {

   static const int NumberOfComponentsToOutput = 5;
   static const int ComponentsToOutput [ NumberOfComponentsToOutput ] = { 1, 2, 3, 9, 10 };

   std::stringstream buffer;
   int i;
   int j;
   int position;

   buffer.flags ( std::ios::scientific );
   buffer.precision ( 6 );

   buffer << " {";

   // for ( j = 0; j < NumberOfComponentsToOutput; ++j ) {
   //    i = ComponentsToOutput [ j ];
   // for ( i = 2; i < 7; ++i ) {
   for ( i = 0; i < NumberOfPVTComponents; ++i ) {

      if ( reverseOrder ) {
         position = NumberOfPVTComponents - i - 1;
      } else {
         position = i;
      }

      buffer << "( " << std::setw ( 11 ) << pvtFlash::ComponentIdNames [ position ] << " = " << std::setw ( 13 ) << m_components [ position ] << " )";

      if ( i < NumberOfPVTComponents - 1 ) {
         buffer << ", ";
      }

   }

   buffer << " }";

   return buffer.str ();
}


//------------------------------------------------------------//

PVTComponents operator+( const PVTComponents& left, const PVTComponents& right ) {

   PVTComponents result = left;

   result += right;

   return result;
}

//------------------------------------------------------------//

PVTComponents operator-( const PVTComponents& left, const PVTComponents& right ) {

   PVTComponents result = left;

   result -= right;

   return result;
}

//------------------------------------------------------------//

PVTComponents operator*( const double left, const PVTComponents& right ) {

   PVTComponents result = right;

   result *= left;

   return result;
}

//------------------------------------------------------------//

PVTComponents operator*( const PVTComponents& left, const double right ) {

   PVTComponents result = left;

   result *= right;

   return result;
}

//------------------------------------------------------------//

PVTComponents operator/( const PVTComponents& left, const double right ) {

   PVTComponents result = left;

   result *= 1.0 / right;

   return result;
}

//------------------------------------------------------------//

PVTComponents operator*( const PVTComponents& left, const PVTComponents& right ) {

   PVTComponents result = left;

   result *= right;

   return result;
}

//------------------------------------------------------------//

PVTComponents operator/( const PVTComponents& left, const PVTComponents& right ) {

   PVTComponents result = left;

   result /= right;

   return result;
}

//------------------------------------------------------------//

PVTComponents maximum ( const PVTComponents& left,
                        const PVTComponents& right ) {

   PVTComponents result;
   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      pvtFlash::ComponentId id = static_cast<pvtFlash::ComponentId>( i );
      result ( id ) = NumericFunctions::Maximum<double>( left ( id ), right ( id ));
   }

   return result;
}

//------------------------------------------------------------//

PVTComponents maximum ( const PVTComponents& left,
                        const double         right ) {

   PVTComponents result;
   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      pvtFlash::ComponentId id = static_cast<pvtFlash::ComponentId>( i );
      result ( id ) = NumericFunctions::Maximum<double>( left ( id ), right );
   }

   return result;
}

//------------------------------------------------------------//

double minimum ( const PVTComponents& cmps ) {

   pvtFlash::ComponentId id;
   int i;
   double val = cmps ( static_cast<pvtFlash::ComponentId>( 0 ));

   for ( i = 1; i < NumberOfPVTComponents; ++i ) {
      pvtFlash::ComponentId id = static_cast<pvtFlash::ComponentId>( i );
      val = NumericFunctions::Minimum<double>( val, cmps ( id ));
   }

   return val;
}

//------------------------------------------------------------//

double minimum ( const PVTPhaseComponents& cmps ) {

   pvtFlash::ComponentId id = static_cast<pvtFlash::ComponentId>( 0 );
   int i;
   double val = NumericFunctions::Minimum<double>( cmps ( pvtFlash::VAPOUR_PHASE, id ), cmps ( pvtFlash::LIQUID_PHASE, id ));

   for ( i = 1; i < NumberOfPVTComponents; ++i ) {
      id = static_cast<pvtFlash::ComponentId>( i );
      val = NumericFunctions::Minimum3<double>( val, cmps ( pvtFlash::VAPOUR_PHASE, id ), cmps ( pvtFlash::LIQUID_PHASE, id ));
   }

   return val;
}

//------------------------------------------------------------//

bool PVTComponents::isFinite () const {

int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
     if(!isfinite(m_components[i]))
	   return false;
   }
  
  return true;
}


//------------------------------------------------------------//

bool PVTComponents::isNonNegative () const {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {

      if ( m_components [ i ] < 0.0 ) {
         return false;
      }

   }

   return true;
}

//------------------------------------------------------------//


PVTPhaseComponents::PVTPhaseComponents () {
   zero ();
}

void PVTPhaseComponents::zero () {

   int j;

   for ( j = 0; j < NumberOfPVTComponents; ++j ) {
      m_masses [ 0 ][ j ] = 0.0;
      m_masses [ 1 ][ j ] = 0.0;
   }

}

double PVTPhaseComponents::sum ( const pvtFlash::PVTPhase phase ) const {

   double result = 0.0;
   int j;

   for ( j = 0; j < NumberOfPVTComponents; ++j ) {
      result += m_masses [ phase ][ j ];
   }

   return result;
}

PVTPhaseComponents& PVTPhaseComponents::operator=( const PVTPhaseComponents& components ) {

   int i;
   int j;

   for ( i = 0; i < pvtFlash::N_PHASES; ++i ) {

      for ( j = 0; j < NumberOfPVTComponents; ++j ) {
         m_masses [ i ][ j ] = components.m_masses [ i ][ j ];
      }

   }

   return *this;
}

PVTPhaseComponents& PVTPhaseComponents::operator+=( const PVTPhaseComponents& components ) {

   int i;
   int j;

   for ( i = 0; i < pvtFlash::N_PHASES; ++i ) {

      for ( j = 0; j < NumberOfPVTComponents; ++j ) {
         m_masses [ i ][ j ] += components ( pvtFlash::PVTPhase ( i ), pvtFlash::ComponentId ( j ));
      }

   }

   return *this;
}

PVTPhaseComponents& PVTPhaseComponents::operator*= ( const PVTComponents& components ) {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      m_masses [ 0 ][ i ] *= components ( pvtFlash::ComponentId ( i ));
      m_masses [ 1 ][ i ] *= components ( pvtFlash::ComponentId ( i ));
   }

   return *this;
}

PVTPhaseComponents& PVTPhaseComponents::operator*= ( const double value ) {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      m_masses [ 0 ][ i ] *= value;
      m_masses [ 1 ][ i ] *= value;
   }

   return *this;
}

PVTPhaseComponents& PVTPhaseComponents::operator/= ( const PVTComponents& components ) {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      m_masses [ 0 ][ i ] /= components ( pvtFlash::ComponentId ( i ));
      m_masses [ 1 ][ i ] /= components ( pvtFlash::ComponentId ( i ));
   }

   return *this;
}

PVTPhaseComponents& PVTPhaseComponents::operator/= ( const PVTPhaseValues& phases ) {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      m_masses [ pvtFlash::VAPOUR_PHASE ][ i ] /= phases ( pvtFlash::VAPOUR_PHASE );
      m_masses [ pvtFlash::LIQUID_PHASE ][ i ] /= phases ( pvtFlash::LIQUID_PHASE );
   }

   return *this;
}

void PVTPhaseComponents::sum ( PVTPhaseValues& phases ) const {

   int j;

   phases ( pvtFlash::LIQUID_PHASE ) = 0.0;
   phases ( pvtFlash::VAPOUR_PHASE ) = 0.0;

   for ( j = 0; j < NumberOfPVTComponents; ++j ) {
      phases ( pvtFlash::LIQUID_PHASE ) += m_masses [ pvtFlash::LIQUID_PHASE ][ j ];
      phases ( pvtFlash::VAPOUR_PHASE ) += m_masses [ pvtFlash::VAPOUR_PHASE ][ j ];
   }

}

void PVTPhaseComponents::sum ( PVTComponents& components ) const {

   int j;

   for ( j = 0; j < NumberOfPVTComponents; ++j ) {
      components ( pvtFlash::ComponentId ( j )) = m_masses [ 0 ][ j ] + m_masses [ 1 ][ j ];
   }

}

void PVTPhaseComponents::setPhaseComponents ( const pvtFlash::PVTPhase phase,
                                              const PVTComponents&    components ) {


   int j;

   for ( j = 0; j < NumberOfPVTComponents; ++j ) {
      m_masses [ phase ][ j ] = components ( pvtFlash::ComponentId ( j ));
   }

}

void PVTPhaseComponents::getPhaseComponents ( const pvtFlash::PVTPhase phase,
                                                    PVTComponents&    components ) const {


   int j;

   for ( j = 0; j < NumberOfPVTComponents; ++j ) {
      components ( pvtFlash::ComponentId ( j )) = m_masses [ phase ][ j ];
   }

}

PVTComponents PVTPhaseComponents::getPhaseComponents ( const pvtFlash::PVTPhase phase ) const {

   PVTComponents components;

   getPhaseComponents ( phase, components );

   return components;
}

std::string PVTPhaseComponents::image ( const bool reverseOrder ) const {

   static const int NumberOfComponentsToOutput = 5;
   static const int ComponentsToOutput [ NumberOfComponentsToOutput ] = { 1, 2, 3, 9, 10 };

   std::stringstream buffer;
   int i;
   int j;
   int position;

   buffer.flags ( std::ios::scientific );
   buffer.precision ( 6 );

   buffer  << " Vapour => {";

   // for ( j = 0; j < NumberOfComponentsToOutput; ++j ) {
   //    i = ComponentsToOutput [ j ];
   // for ( i = 2; i < 7; ++i ) {
   for ( i = 0; i < NumberOfPVTComponents; ++i ) {

      if ( reverseOrder ) {
         position = NumberOfPVTComponents - i - 1;
      } else {
         position = i;
      }

      buffer << "( " << std::setw ( 11 ) << pvtFlash::ComponentIdNames [ position ] << " = " << std::setw ( 13 ) << m_masses [ 0 ][ position ] << " )";

      if ( i < NumberOfPVTComponents - 1 ) {
         buffer << ", ";
      }

   }

   buffer << " }" << std::endl;

   buffer  << " Liquid => {";

   // for ( j = 0; j < NumberOfComponentsToOutput; ++j ) {
   //    i = ComponentsToOutput [ j ];
   // for ( i = 2; i < 7; ++i ) {
   for ( i = 0; i < NumberOfPVTComponents; ++i ) {

      if ( reverseOrder ) {
         position = NumberOfPVTComponents - i - 1;
      } else {
         position = i;
      }

      buffer << "( " << std::setw ( 11 ) << pvtFlash::ComponentIdNames [ position ] << " = " << std::setw ( 13 ) << m_masses [ 1 ][ position ] << " )";

      if ( i < NumberOfPVTComponents - 1 ) {
         buffer << ", ";
      }

   }

   buffer << " }" << std::endl;

   return buffer.str ();
}

//------------------------------------------------------------//

PVTPhaseValues::PVTPhaseValues () {
   zero ();
}

void PVTPhaseValues::zero () {

   int i;

   for ( i = 0; i < pvtFlash::N_PHASES; ++i ) {
      m_values [ i ] = 0.0;
   }

}

PVTPhaseValues& PVTPhaseValues::operator=( const PVTPhaseValues& values ) {

   m_values [ 0 ] = values.m_values [ 0 ];
   m_values [ 1 ] = values.m_values [ 1 ];

   return *this;
}

PVTPhaseValues& PVTPhaseValues::operator*=( const double scalar ) {

   m_values [ 0 ] *= scalar;
   m_values [ 1 ] *= scalar;

   return *this;
}


std::string PVTPhaseValues::image () const {

   std::stringstream buffer;

   buffer.flags ( std::ios::scientific );
   buffer.precision ( 6 );

   buffer << " { Vapour => " << m_values [ 0 ] << ", Liquid => " << m_values [ 1 ] << " }";

   return buffer.str ();
}


PVTPhaseValues maximum ( const PVTPhaseValues& values, 
                         const double          scalar ) {

   PVTPhaseValues result;

   result ( pvtFlash::VAPOUR_PHASE ) = NumericFunctions::Maximum<double>( values ( pvtFlash::VAPOUR_PHASE ), scalar );
   result ( pvtFlash::LIQUID_PHASE ) = NumericFunctions::Maximum<double>( values ( pvtFlash::LIQUID_PHASE ), scalar );

   return result;
}

PVTPhaseValues maximum ( const PVTPhaseValues& values1, 
                         const PVTPhaseValues& values2 ) {

   PVTPhaseValues result;

   result ( pvtFlash::VAPOUR_PHASE ) = NumericFunctions::Maximum<double>( values1 ( pvtFlash::VAPOUR_PHASE ), values2 ( pvtFlash::LIQUID_PHASE ));
   result ( pvtFlash::LIQUID_PHASE ) = NumericFunctions::Maximum<double>( values1 ( pvtFlash::LIQUID_PHASE ), values2 ( pvtFlash::LIQUID_PHASE ));

   return result;
}


//------------------------------------------------------------//

PVTCalc::PVTCalc () {

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      m_molarMass ( pvtFlash::ComponentId ( i )) = 0.0;
   }


   // The default molar masses.
   m_molarMass ( pvtFlash::ASPHALTENES ) = 7.979050e+02;
   m_molarMass (      pvtFlash::RESINS ) = 6.105592e+02;
   m_molarMass (     pvtFlash::C15_ARO ) = 4.633910e+02;
   m_molarMass (     pvtFlash::C15_SAT ) = 2.646560e+02;
   m_molarMass (    pvtFlash::C6_14ARO ) = 1.564148e+02;
   m_molarMass (    pvtFlash::C6_14SAT ) = 1.025200e+02;
   m_molarMass (          pvtFlash::C5 ) = 7.215064e+01;
   m_molarMass (          pvtFlash::C4 ) = 5.812370e+01;
   m_molarMass (          pvtFlash::C3 ) = 4.409676e+01;
   m_molarMass (          pvtFlash::C2 ) = 3.006982e+01;
   m_molarMass (          pvtFlash::C1 ) = 1.604288e+01;
   m_molarMass (         pvtFlash::COX ) = 4.400980e+01;
   m_molarMass (          pvtFlash::N2 ) = 2.801352e+01;
   m_molarMass (         pvtFlash::H2S ) = 3.4080000+01;
   m_molarMass (         pvtFlash::LSC ) = 2.646560e+02;
   m_molarMass (      pvtFlash::C15_AT ) = 2.646560e+02;
   m_molarMass (     pvtFlash::C6_14BT ) = 1.564147e+02;
   m_molarMass (    pvtFlash::C6_14DBT ) = 1.564147e+02;
   m_molarMass (     pvtFlash::C6_14BP ) = 1.564147e+02;
   m_molarMass (    pvtFlash::C15_AROS ) = 2.646560e+02;
   m_molarMass (    pvtFlash::C15_SATS ) = 2.646560e+02;
   m_molarMass (   pvtFlash::C6_14SATS ) = 1.564147e+02;
   m_molarMass (   pvtFlash::C6_14AROS ) = 1.564147e+02;

   

}

PVTComponents PVTCalc::computeMolarMass ( const PVTComponents& weights ) const {

   PVTComponents molarMass;
   double gorm = computeGorm ( weights );
   pvtFlash::ComponentId component;

   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      component = pvtFlash::ComponentId ( i );
      //molarMass ( component ) = pvtFlash::EosPack::getInstance ().getMolWeight ( i, gorm );
	  molarMass ( component ) = pvtFlash::EosPack::getInstance ().getMolWeightLumped ( i, gorm );
   }

   return molarMass;
}


PVTPhaseValues PVTCalc::computeCriticalTemperature ( const PVTPhaseComponents& composition,
                                                     const bool                gormIsPrescribed,
                                                     const double              prescribedGorm ) const {

   PVTPhaseValues critialTemperature;
   PVTPhaseValues norm;
   pvtFlash::ComponentId component;
   double gorm = ( gormIsPrescribed ? prescribedGorm : computeGorm ( composition ));
   double lambda;

   double componentCriticalVolume;
   double componentMolarWeight;
   double componentCriticalTemperature;

   int i;

   critialTemperature.zero ();
   norm.zero ();

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      component = pvtFlash::ComponentId ( i );

      if ( composition ( pvtFlash::VAPOUR_PHASE, component ) == 0.0 and
           composition ( pvtFlash::LIQUID_PHASE, component ) == 0.0 ) {
         continue;
      }

      componentCriticalVolume      = pvtFlash::EosPack::getInstance ().getCriticalVolumeLumped ( i, gorm );
      componentMolarWeight         = pvtFlash::EosPack::getInstance ().getMolWeightLumped ( i, gorm );
      componentCriticalTemperature = pvtFlash::EosPack::getInstance ().getCriticalTemperatureLumped ( i, gorm );

      lambda = composition ( pvtFlash::VAPOUR_PHASE, component ) * componentCriticalVolume / componentMolarWeight;
      critialTemperature ( pvtFlash::VAPOUR_PHASE ) += lambda * componentCriticalTemperature;
      norm ( pvtFlash::VAPOUR_PHASE ) += lambda;

      lambda = composition ( pvtFlash::LIQUID_PHASE, component ) * componentCriticalVolume / componentMolarWeight;
      critialTemperature ( pvtFlash::LIQUID_PHASE ) += lambda * componentCriticalTemperature;
      norm ( pvtFlash::LIQUID_PHASE ) += lambda;
   }   

   // assert(norm > 0.0);
   critialTemperature /= norm;

   return critialTemperature;
}

double PVTCalc::computeCriticalTemperature ( const PVTComponents& composition,
                                             const bool           gormIsPrescribed,
                                             const double         prescribedGorm ) const {

   double critialTemperature;
   double norm;
   double gorm = ( gormIsPrescribed ? prescribedGorm : computeGorm ( composition ));
   double lambda;
   pvtFlash::ComponentId component;

   double componentCriticalVolume;
   double componentMolarWeight;
   double componentCriticalTemperature;

   int i;

   critialTemperature = 0.0;
   norm = 0.0;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      component = pvtFlash::ComponentId ( i );

      if ( composition ( component ) == 0.0 ) {
         continue;
      }

      componentCriticalVolume      = pvtFlash::EosPack::getInstance ().getCriticalVolumeLumped ( i, gorm );
      componentMolarWeight         = pvtFlash::EosPack::getInstance ().getMolWeightLumped ( i, gorm );
      componentCriticalTemperature = pvtFlash::EosPack::getInstance ().getCriticalTemperatureLumped ( i, gorm );

      lambda = composition ( component ) * componentCriticalVolume / componentMolarWeight;
      critialTemperature += lambda * componentCriticalTemperature;
      norm += lambda;
   }   

   // assert(norm > 0.0);
   critialTemperature /= norm;

   return critialTemperature;
}

bool PVTCalc::compute ( const double               temperature,
                        const double               pressure,
                              PVTComponents&       components,
                              PVTPhaseComponents&  masses,
                              PVTPhaseValues&      densities,
                              PVTPhaseValues&      viscosities,
                        const bool                 gormIsPrescribed,
                        const double               gorm ) {


   return pvtFlash::EosPack::getInstance ().computeWithLumping ( temperature,
                                                                 pressure,
                                                                 components.m_components,
                                                                 masses.m_masses,
                                                                 densities.m_values,
                                                                 viscosities.m_values,
                                                                 gormIsPrescribed,
                                                                 gorm );

}

double PVTCalc::computeGorm ( const PVTComponents& weights ) const {

  double denom = 0.0;
  denom += weights(pvtFlash::C6_14ARO);
  denom += weights(pvtFlash::C6_14SAT);
  denom += weights(pvtFlash::C15_ARO);
  denom += weights(pvtFlash::C15_SAT);
  denom += weights(pvtFlash::RESINS);
  denom += weights(pvtFlash::ASPHALTENES);
  
  denom += weights(pvtFlash::LSC);
  denom += weights(pvtFlash::C15_AT);
  denom += weights(pvtFlash::C15_AROS);
  denom += weights(pvtFlash::C15_SATS);
  
  denom += weights(pvtFlash::C6_14BT);
  denom += weights(pvtFlash::C6_14DBT);
  denom += weights(pvtFlash::C6_14BP);
  denom += weights(pvtFlash::C6_14SATS);
  denom += weights(pvtFlash::C6_14AROS);
  
  //assert(denom >= 0.0);
  
  if (denom == 0.0) {
	// return CBMGenerics::undefined;
	return 1.0e+80; 
  }
  
  double num = 0.0;
  num += weights(pvtFlash::C1);
  num += weights(pvtFlash::C2);
  num += weights(pvtFlash::C3);
  num += weights(pvtFlash::C4);
  num += weights(pvtFlash::C5);
  
  num += weights(pvtFlash::H2S);

  //what about N2
  
  return num / denom;
  
}


double PVTCalc::computeGorm ( const PVTComponents& vapour,
                              const PVTComponents& liquid ) const {

   int i;
   double vapourMass = 0.0;
   double liquidMass = 0.0;
   double gorm;

   for ( i = pvtFlash::ASPHALTENES; i <= pvtFlash::C6_14SAT; ++i ) {
      liquidMass += vapour ( pvtFlash::ComponentId ( i )) + liquid ( pvtFlash::ComponentId ( i ));
   }

   for ( i = pvtFlash::LSC; i <= pvtFlash::C6_14AROS; ++i )
	 {
	   liquidMass += vapour ( pvtFlash::ComponentId ( i )) + liquid ( pvtFlash::ComponentId ( i ));
	 }
   //what about N2 

   
   if ( liquidMass == 0.0 ) {
      gorm = 1.0e80;
   } else {

      for ( i = pvtFlash::C5; i <= pvtFlash::C1; ++i ) {
         vapourMass += vapour ( pvtFlash::ComponentId ( i )) + liquid ( pvtFlash::ComponentId ( i ));
      }
	  vapourMass +=  vapour ( pvtFlash::ComponentId (pvtFlash::H2S));

      gorm = vapourMass / liquidMass;
   }

   return gorm;
}

double PVTCalc::computeGorm ( const PVTPhaseComponents& phaseComponents ) const {

   int i;
   double vapourMass = 0.0;
   double liquidMass = 0.0;
   double gorm;

   for ( i = pvtFlash::ASPHALTENES; i <= pvtFlash::C6_14SAT; ++i ) {
      liquidMass += phaseComponents ( pvtFlash::VAPOUR_PHASE, pvtFlash::ComponentId ( i )) +
                    phaseComponents ( pvtFlash::LIQUID_PHASE, pvtFlash::ComponentId ( i ));
   }
   for ( i = pvtFlash::LSC; i <= pvtFlash::C6_14AROS; ++i )
   {
      liquidMass += phaseComponents ( pvtFlash::VAPOUR_PHASE, pvtFlash::ComponentId ( i )) +
                    phaseComponents ( pvtFlash::LIQUID_PHASE, pvtFlash::ComponentId ( i )); 
   }

    //what about N2 

   if ( liquidMass == 0.0 ) {
      gorm = 1.0e80;
   } else {

      for ( i = pvtFlash::C5; i <= pvtFlash::C1; ++i ) {
         vapourMass += phaseComponents ( pvtFlash::VAPOUR_PHASE, pvtFlash::ComponentId ( i )) +
                       phaseComponents ( pvtFlash::LIQUID_PHASE, pvtFlash::ComponentId ( i ));
      }
	  //H2S
	  vapourMass += phaseComponents ( pvtFlash::VAPOUR_PHASE, pvtFlash::ComponentId (pvtFlash::H2S));
      gorm = vapourMass / liquidMass;
   }

   return gorm;
}

PVTComponents operator* ( const PVTPhaseComponents& cmps, 
                          const PVTPhaseValues&     phases ) {

   PVTComponents result;

   pvtFlash::ComponentId id;
   int i;

   for ( i = 0; i < NumberOfPVTComponents; ++i ) {
      id = pvtFlash::ComponentId ( i );
      result ( id ) = phases ( pvtFlash::VAPOUR_PHASE ) * cmps.m_masses [ 0 ][ i ] + phases ( pvtFlash::LIQUID_PHASE ) * cmps.m_masses [ 1 ][ i ];
   }

   return result;
}


//------------------------------------------------------------//
