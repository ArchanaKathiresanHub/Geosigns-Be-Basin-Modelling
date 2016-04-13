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

#include <string>

class AppCtx;
class FastcauldronFactory;

/// \brief Contains the sequence of operations that are required to initialise the fastcauldron data structures.
class FastcauldronStartup {

public :

   static int startup ( int                  argc,
                        char**               argv,
                        AppCtx*              cauldron,
                        FastcauldronFactory* factory,
                        const bool           canRunSaltModelling,
                        std::string&         errorMessage,
                        const bool           saveAsInputGrid = false,
                        const bool           createResultsFile = true );

};

#endif // FASTCAULDRON__FASTCAULDRON_STARTUP__H
