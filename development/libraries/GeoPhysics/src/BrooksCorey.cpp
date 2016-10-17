//                                                                      
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BrooksCorey.h"

/// Added to keep using M_PI (below), but should be replaced by a global definition of pi in utilities, eventually.
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>

// Declare some static variables; they are defined in the header 

namespace GeoPhysics
{
   const double BrooksCorey::Sir = 0.1;
   const double BrooksCorey::Pe = 1000000;
   const double BrooksCorey::Sor = 0.3; 
   const double BrooksCorey::Krwor = 1.0; 
   const double BrooksCorey::Krocw = 1.0; 
   const double BrooksCorey::Adjustment = 1.0e-4;
   const double BrooksCorey::log10 = std::log(10.0);
   const double BrooksCorey::VapourWaterContactAngle = 180.0;
   const double BrooksCorey::LiquidWaterContactAngle = 180.0 - 30.0;  // 150
   const double BrooksCorey::LiquidVapourContactAngle = 180.0 - 22.0; // 158
   const double BrooksCorey::CosLiquidVapourContactAngle = std::cos(BrooksCorey::LiquidVapourContactAngle * M_PI / 180.0);
   const double BrooksCorey::MercuryAirContactAngle = 140.0;
   const double BrooksCorey::CosMercuryAirContactAngle = std::cos(BrooksCorey::MercuryAirContactAngle * M_PI / 180.0);
   const double BrooksCorey::MercuryAirInterfacialTension = 0.48;

   double BrooksCorey::liquidVapourInterfacialTension(const double pressure)
   {
      return 19.099e-3 * std::exp(-0.034878 * pressure * log10);
   }

   /// The capillary entry pressure.
   double BrooksCorey::computeCapillaryEntryPressure(const double permeability,
      const double c1,
      const double c2)
   {
      return 1.0e6 * c2 * pow(permeability, -c1);
   }

   double BrooksCorey::pc(const double Sw, const double lambda, const double pce)
   {

      double Sr;

      if (Sw <= Sir + Adjustment)
      {
         Sr = Adjustment / (1.0 - Sir);
      }
      else if (Sw == 1.0)
      {
         Sr = 1; // is this condition correct?
      }
      else
      {
         Sr = (Sw - Sir) / (1.0 - Sir);
      }

      if (Sw == 1)
      {
         return pce; //  Pe = Pc(Sw==1) 
      }

#if 0
      if (Sw <= Sir)
         return 0;
      //effective saturation

      double Sr = (Sw - Sir) / (1 - Sir);
#endif

      assert(0 <= Sr && Sr <= 1);

      return pce * pow(Sr, -lambda);
   }

   double BrooksCorey::krw(double Sw, double lambda)
   {

      double Swe;

      if (Sw >= 1.0 - Sir - Sor)
      {
         Swe = 1.0;
      }
      else if (Sw <= Sir + Adjustment)
      {
         Swe = Adjustment / (1.0 - Sir - Sor);
      }
      else
      {
         Swe = (Sw - Sir) / (1.0 - Sir - Sor);
      }

#if 0
      if (Sw < Sir)
      {
         return 0.0;
      }


      if (Sw > 1.0 - Sor)
      {
         return Krwor;
      }

      double Swe = (Sw - Sir) / (1 - Sir - Sor);
#endif

#if 0
      if (0 > Swe or Swe > 1 or not (0 <= Swe && Swe <= 1))
      {
         cout << endl << endl << " krw " << Sw << "  " << Swe << "  " << lambda << endl << endl << flush;
      }
#endif

      assert(0 <= Swe && Swe <= 1);

      return Krwor * pow(Swe, lambda);
   }

   double BrooksCorey::kro(double Sw, double lambda)
   {

      double Swe;

      if (Sw >= 1.0 - Sor)
      {
         Swe = 0.0;
      }
      else if (Sw <= Sir)
      {
         Swe = 1.0;
      }
      else
      {
         Swe = (1.0 - Sw - Sor) / (1.0 - Sir - Sor);
      }


#if 0
      if (Sw <= Sir) return 1.0;

      if (Sw > 1.0 - Sor)
      {
         return 0.0;
      }

      double Swe = (1 - Sw - Sor) / (1 - Sir - Sor);

#endif

      assert(0 <= Swe && Swe <= 1);
      return Krocw * pow(Swe, lambda);
   }

}
