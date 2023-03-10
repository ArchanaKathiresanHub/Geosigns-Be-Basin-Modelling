//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_TRANSLATEPROPS_H_
#define _MIGRATION_TRANSLATEPROPS_H_

#include "capillarySealStrength.h"

#include "LithoType.h"
#include "Formation.h"

using namespace DataAccess;

using Interface::LithoType;
using Interface::Formation;

namespace migration
{
   namespace translateProps
   {

      struct CreateCapillaryLithoProp
      {
         typedef CBMGenerics::capillarySealStrength::LithoProp output;

         CBMGenerics::capillarySealStrength::LithoProp operator()(const LithoType* lithoType)
         {
            return CBMGenerics::capillarySealStrength::LithoProp (lithoType->getLithoTypeAttributeValue (Interface::DepoPerm).getDouble (),
               lithoType->getLithoTypeAttributeValue (Interface::CapC1).getDouble (),
               lithoType->getLithoTypeAttributeValue (Interface::CapC2).getDouble ());
         }
      };

      struct CreateLithHydraulicFracturingFrac
      {
         typedef double output;

         double operator()(const LithoType* lithoType)
         {
            // Converting percentage to fraction through the 0.01 factor
            return (0.01 * lithoType->getLithoTypeAttributeValue (Interface::HydraulicFracturingPercent).getDouble ());
         }
      };

      template <typename CREATEPROP>
      void translate (const  Interface::Formation* formation,
         CREATEPROP create, std::vector<typename CREATEPROP::output>& lithProps)
      {
         const LithoType* lithoType1 = formation->getLithoType1 ();
         assert (lithoType1);
         lithProps.push_back (create (lithoType1));

         const LithoType* lithoType2 = formation->getLithoType2 ();
         if (lithoType2)
            lithProps.push_back (create (lithoType2));

         const LithoType* lithoType3 = formation->getLithoType3 ();
         if (lithoType3)
            lithProps.push_back (create (lithoType3));
      }

   }
} // namespace migration::translateProps

#endif
