#ifndef _FASTCALDRON__CAULDRON_CALCULATOR_H_
#define _FASTCALDRON__CAULDRON_CALCULATOR_H_

#include "petscts.h"
#include "petscdmda.h"
#include "propinterface.h"
#include "globaldefs.h"

class CauldronCalculator {

public :

   CauldronCalculator ( AppCtx* appl );

   virtual ~CauldronCalculator ();

protected :

   AppCtx* cauldron;

};



#endif // _FASTCALDRON__CAULDRON_CALCULATOR_H_
