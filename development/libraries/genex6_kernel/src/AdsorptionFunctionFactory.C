#include "AdsorptionFunctionFactory.h"

#include "LangmuirAdsorptionFunction.h"
#include "LangmuirAdsorptionTOCFunction.h"


Genex6::AdsorptionFunctionFactory* Genex6::AdsorptionFunctionFactory::s_adsorptionFunctionFactory = 0;


Genex6::AdsorptionFunctionFactory::AdsorptionFunctionFactory () {
   m_adsorptionFunctionAllocatorMapping [ false ] = allocateLangmuirAdsorptionFunction;
   m_adsorptionFunctionAllocatorMapping [ true  ] = allocateLangmuirAdsorptionTOCFunction;
}

Genex6::AdsorptionFunction* Genex6::AdsorptionFunctionFactory::getAdsorptionFunction ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                                       const bool                                 isTOCDependent,
                                                                                       const std::string&                         adsorptionCapacityFunctionName ) const {

   AdsorptionFunctionAllocatorMap::const_iterator functionIter = m_adsorptionFunctionAllocatorMapping.find ( isTOCDependent );

   if ( functionIter != m_adsorptionFunctionAllocatorMapping.end ()) {
      return (functionIter->second)( projectHandle, adsorptionCapacityFunctionName );
   } else {
      return 0;
   }

}

Genex6::AdsorptionFunctionFactory& Genex6::AdsorptionFunctionFactory::getInstance () {

   if ( s_adsorptionFunctionFactory == 0 ) {
      s_adsorptionFunctionFactory = new AdsorptionFunctionFactory;
   }

   return *s_adsorptionFunctionFactory;
}
