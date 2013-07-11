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

   int createMatrixStructure ( const DM   mapDA, 
                               const DM   femGridDA,
                               const Boolean2DArray& validNeedle,
                               const Vec  dofs,
                                     Mat* jacobian,
                               const int  stencilWidth );

   void setDOFs ( const DM   femGrid, 
                  const Vec  depths, 
                        Vec  dofNumbers, 
                        int& stencilWidth );

   void setDepths ( const DM   femGrid, 
                    const bool includeBasement,
                          Vec  depths );

   /// Fills the vector femRealNodes with false or true ( 0.0 or  x > 0.0 )
   /// if the node is is an active node in the finite element mesh.
   void setRealNodes ( const DM  femGrid, 
                             Vec femDOFs, 
                             Vec femRealNodes );



protected :

   AppCtx* cauldron;

};



#endif // _FASTCALDRON__CAULDRON_CALCULATOR_H_
