#ifndef _GENEX6_KERNEL__ADSORPTION_FUNCTION_FACTORY_H_
#define _GENEX6_KERNEL__ADSORPTION_FUNCTION_FACTORY_H_

#include <string>

#include "AdsorptionFunction.h"

#include "Interface/ProjectHandle.h"

namespace Genex6 {

   /// Factory singleton object for allocating adsorption-functions.
   class AdsorptionFunctionFactory {

      typedef Genex6::AdsorptionFunction* (*AllocateAdsorptionFunction)( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                         const std::string&                         functionName );

      typedef std::map <bool, AllocateAdsorptionFunction> AdsorptionFunctionAllocatorMap;

   public :

      /// Get singleton instance.
      static AdsorptionFunctionFactory& getInstance ();

      /// Remove singleton instance.
      static void clear ();

 
      /// Allocate the adsorption function.
      ///
      /// The function allocated will depend on the adsorption-simulator parameters which-function.
      AdsorptionFunction* getAdsorptionFunction ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                  const bool                                 isTOCDependent,
                                                  const std::string&                         adsorptionCapacityFunctionName ) const;

   private :

      AdsorptionFunctionFactory ();

      static AdsorptionFunctionFactory* s_adsorptionFunctionFactory;

      AdsorptionFunctionAllocatorMap m_adsorptionFunctionAllocatorMapping;

   };

}

#endif // _ADSORPTION_FUNCTION_FACTORY_H_
