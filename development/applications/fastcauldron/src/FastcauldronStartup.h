#ifndef FASTCAULDRON__FASTCAULDRON_STARTUP__H
#define FASTCAULDRON__FASTCAULDRON_STARTUP__H

#include <string>

#include "propinterface.h"
#include "FastcauldronFactory.h"

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
