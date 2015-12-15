#ifndef FASTCAULDRON__FASTCAULDRON_STARTUP__H
#define FASTCAULDRON__FASTCAULDRON_STARTUP__H

#include <string>

#include "propinterface.h"
#include "FastcauldronFactory.h"

/// \brief Contains the sequence of operations that are required to initialise the fastcauldron data structures.
class FastcauldronStartup {

public :

   static int startup ( int                  argc,
                        char**               argv,
                        AppCtx*              cauldron,
                        FastcauldronFactory* factory,
                        const bool           canRunSaltModelling,
                        std::string&         errorMessage );

};

#endif // FASTCAULDRON__FASTCAULDRON_STARTUP__H
