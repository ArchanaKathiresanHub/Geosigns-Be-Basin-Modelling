#ifndef _INTERFACE_MANTLE_FORMATION_H_
#define _INTERFACE_MANTLE_FORMATION_H_

#include "Interface/DAObject.h"
#include "Interface/Interface.h"
#include "Interface/BasementFormation.h"


namespace DataAccess
{
   namespace Interface
   {

      class MantleFormation : virtual public BasementFormation
      {
      public:

         /// constructor.
         MantleFormation (ProjectHandle * projectHandle, database::Record* record);

         /// destructor.
         ~MantleFormation (void);

         /// Return the present day, user-supplied Mantle thickness GridMap.
         const GridMap * getInputThicknessMap (void) const;
         const GridMap * getInitialThicknessMap (void) const;

         /// The present day thickness is always a constant, it can never be a map.
         ///
         /// The values depends on the bottom boundary condition.
         virtual double getPresentDayThickness () const;
         virtual double getInitialLithosphericMantleThickness () const;

         /// Return a sequence of paleo-thickness maps.
         ///
         /// The thickness of the mantle is controlled by both the present 
         /// day mantle thickness and the crust thinning history.
         PaleoFormationPropertyList * getPaleoThicknessHistory () const;

         virtual int getDepositionSequence () const;

         void asString (string & str) const;


      protected:

      };
   }
}
#endif // _INTERFACE_MANTLE_FORMATION_H_
