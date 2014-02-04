#include "BrooksCorey.h"

const double BrooksCorey::log10 = std::log ( 10.0 );

const double BrooksCorey::GasWaterContactAngle = 180.0;

const double BrooksCorey::OilWaterContactAngle = 180.0 - 30.0; // 150


const double BrooksCorey::OilGasContactAngle = 180.0 - 22.0; // 158

const double BrooksCorey::CosOilGasContactAngle = std::cos ( BrooksCorey::OilGasContactAngle * M_PI / 180.0 );

const double BrooksCorey::MercuryAirContactAngle = 140.0;

const double BrooksCorey::CosMercuryAirContactAngle = std::cos ( BrooksCorey::MercuryAirContactAngle * M_PI / 180.0 );

const double BrooksCorey::MercuryAirInterfacialTension = 0.48;

// entry pressure = 1 MPa
const double BrooksCorey::CapillaryEntryPressure = 1000000;

const double BrooksCorey::Adjustment = 1.0e-4;


//------------------------------------------------------------//

double BrooksCorey::computeCapillaryPressure ( const double sat,
                                               const double lambda,
                                               const double sir,
                                               const double pce ) {

   double sr;

   if ( sat <= sir + Adjustment ) {
      sr = Adjustment / ( 1.0 - sir ); 
   } else if ( sat == 1.0 ) {
      sr = 1; // is this condition correct?
   } else {
      sr = ( sat - sir ) / ( 1.0 - sir );
   }

   if ( sat == 1 ) {
      return pce; //  CapillaryEntryPressure = Pc(sat==1) 
   }

   assert(0 <= sr && sr <= 1);
	
   return pce*std::pow(sr, -lambda);
}

//------------------------------------------------------------//

double BrooksCorey::krw ( const double waterSaturation, 
                          const double lambda,
                          const double sir,
                          const double sgr,
                          const double sor,
                          const double krwMax ) {

   double swe;

   if ( waterSaturation <= sir + Adjustment ) {
      // We would like to always have a non-zero water relative-permeability.
      swe = Adjustment / ( 1.0 - ( sir + Adjustment ));
   } else {
      swe = ( waterSaturation - ( sir )) / ( 1.0 - ( sir + sgr + sor ));
   }

   if ( swe > 1.0 ) {
      swe = 1.0;
   }

   return krwMax * std::pow ( swe, lambda );
}

//------------------------------------------------------------//

double BrooksCorey::kro ( const pvtFlash::PVTPhase phase,
                          const double             hcPhaseSaturation, 
                          const double             lambda,
                          const double             sir,
                          const double             sgr,
                          const double             sor,
                          const double             krMax ) {

   double swe;
   double residualHc;

   if ( phase == pvtFlash::LIQUID_PHASE ) {
      residualHc = sor;
   } else {
      residualHc = sgr;
   }

   swe = ( hcPhaseSaturation - residualHc ) / ( 1.0 - ( sir + sgr + sor ));

   if ( swe > 1.0 ) {
      swe = 1.0;
   } else if ( swe < 0.0 ) {
      swe = 0.0;
   }

   return krMax * std::pow ( swe, lambda );
}
