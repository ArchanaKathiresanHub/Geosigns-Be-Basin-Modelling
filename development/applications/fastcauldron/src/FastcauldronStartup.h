//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef FASTCAULDRON__FASTCAULDRON_STARTUP__H
#define FASTCAULDRON__FASTCAULDRON_STARTUP__H

#include "propinterface.h"
#include "FastcauldronFactory.h"

#include <string>

#ifdef FLEXLM
#undef FLEXLM
#endif

#ifdef DISABLE_FLEXLM
#undef FLEXLM
#else
#define FLEXLM 1
#endif

#ifdef FLEXLM
// FlexLM license handling
#include <EPTFlexLm.h>
#endif

/// \brief Contains the sequence of operations that are required to initialise the fastcauldron data structures.
class FastcauldronStartup {

public :

   static int prepare( bool                 canRunSaltModelling );

   static int startup ( int                  argc,
                        char**               argv,
                        const bool           canRunSaltModelling,
                        const bool           saveAsInputGrid = false,
                        const bool           createResultsFile = true );

   static int run();

   static int finalise( bool returnStatus );

   static bool determineSaltModellingCapability( );

   static int ourRank();

private:
   
   static AppCtx * s_cauldron;
   static FastcauldronFactory* s_factory;
   static std::string s_errorMessage;
   static bool s_solverHasConverged;
   static bool s_errorInDarcy;
   static bool s_geometryHasConverged;

#ifdef FLEXLM
   static char s_feature[EPTFLEXLM_MAX_FEATURE_LEN];
#else
   static char s_feature[256];
#endif
                        
};

#endif // FASTCAULDRON__FASTCAULDRON_STARTUP__H
