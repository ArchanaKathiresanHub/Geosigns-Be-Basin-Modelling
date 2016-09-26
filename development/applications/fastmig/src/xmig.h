//
// Copyright (C) 2010-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

///////////////////////////////////////////////////////////////////////////////
//                                VERSION:1.0                                //
///////////////////////////////////////////////////////////////////////////////

#ifndef __rbygeologymodel_xmig
#define __rbygeologymodel_xmig
///////////////////////////////////////////////////////////////////////////////
//                             TRAP definitions                              //
///////////////////////////////////////////////////////////////////////////////

class XMig
{
public:
   enum Phase
   {
      NO_PHASE = -1,
      NONE = -1,
      GAS_PHASE = 0,
      GAS = 0,
      OIL_PHASE = 1,
      OIL = 1,
      ALL = 2,
      NUM_PHASES = 2
   };

   enum Component
   {
      C1_COMP_NUM = 0,
      C2_COMP_NUM = 1,
      C3_COMP_NUM = 2,
      C4_COMP_NUM = 3,
      C5_COMP_NUM = 4,
      N2_COMP_NUM = 5,
      COX_COMP_NUM = 6,
      Cox_COMP_NUM = 6,
      OilPL_COMP_NUM = 7,
      OilPM_COMP_NUM = 8,
      OilPH_COMP_NUM = 9,
      OilAL_COMP_NUM = 10,
      OilAM_COMP_NUM = 11,
      OilAH_COMP_NUM = 12,
      NUM_COMPONENT = 13,
      NUM_GASCOMPONENT = 5,
      NUM_LIGHTGASCOMPONENT = 3
   };

   static const char * RES_PROP_PREFIX;
   static const char * COMPONENT_PROP_NAME[];
   static const char * PHASE_NAME[];

   static const double COMP_MOL_WEIGHT[];
   static const double COMP_MOL_VOLUME[];
   static const double COMP_CRIT_TEMP[];

   static const char * FAULT_NAME[];

   typedef enum _FaultType
   {
      NOFAULT = 0,
      SEAL = 1,
      PASS = 2,
      WASTEGAS = 3,
      SEALOIL = 4,
      PASSOIL = 5,
      WASTE = 6
   } FaultType;


   enum
   {
      NEIGHBOUR_SEARCH = 1,
      MAXCELLSEARCH = 9
   };

   typedef enum _UnitType
   {
      METRIC = 0,
      OILFIELD = 1
   } UnitType;


   static const int IOFFSET[XMig::MAXCELLSEARCH];
   static const int JOFFSET[XMig::MAXCELLSEARCH];

   static const double G;		// Gravity (m/s^2)
   static const double RES_ZERO_MASS;	// Anything smaller than this ==> zero
   static const double WASTEDEPTH;	// reservoir depth at a waste point
   static const double SEALDEPTH;	// reservoir depth at a sealing point

   static const float MINIMUMWEIGHT;	// minimum weight at which we start migrating

   static const float STPRESSURE;	// Stock tank pressure
   static const float STTEMPERATURE;	// Stock tank temperature

   static const float MINPERMEABILITY;  // minimum permeability that can be calculated

   static int EqualQty (float a, float b, float maxdiff = 5000, float maxerror = 0.02);
   static int EqualVolume (float a, float b)
   {
      return EqualQty (a, b, 10, 0.0001);
   }
   static int EqualWeight (float a, float b)
   {
      return EqualQty (a, b, 1, 0.0001);
   }
   static int EqualDepth (float a, float b)
   {
      return EqualQty (a, b, 0.1, 0.000);
   }
};

#endif // __rbygeologymodel_xmig
