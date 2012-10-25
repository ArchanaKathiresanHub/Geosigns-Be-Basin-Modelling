#ifndef _ADSORPTION_FUNCTION_FACTORY_H_
#define _ADSORPTION_FUNCTION_FACTORY_H_

#include <string>

#include "AdsorptionFunction.h"

#include "AdsorptionProjectHandle.h"

/// Factory singleton object for allocating adsorption-functions.
class AdsorptionFunctionFactory {

   typedef Genex5::AdsorptionFunction* (*AllocateAdsorptionFunction)( AdsorptionProjectHandle* projectHandle,
                                                                      const std::string&       formationName );

   typedef std::map <bool, AllocateAdsorptionFunction> AdsorptionFunctionAllocatorMap;

public :

   /// Get singleton instance.
   static AdsorptionFunctionFactory& getInstance ();

   /// Allocate the adsorption function.
   ///
   /// The function allocated will depend on the adsorption-simulator parameters which-function.
   Genex5::AdsorptionFunction* getAdsorptionFunction ( AdsorptionProjectHandle* projectHandle,
						       bool isTOCDependent,
                                                       const std::string&       adsorptionCapacityFunctionName ) const;

private :

   AdsorptionFunctionFactory ();

   static AdsorptionFunctionFactory* s_adsorptionFunctionFactory;

   AdsorptionFunctionAllocatorMap m_adsorptionFunctionAllocatorMapping;

};

#endif // _ADSORPTION_FUNCTION_FACTORY_H_
