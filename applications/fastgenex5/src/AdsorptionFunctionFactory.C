#include "AdsorptionFunctionFactory.h"

#include "LangmuirAdsorptionFunction.h"
#include "LangmuirAdsorptionTOCFunction.h"


AdsorptionFunctionFactory* AdsorptionFunctionFactory::s_adsorptionFunctionFactory = 0;


AdsorptionFunctionFactory::AdsorptionFunctionFactory () {
   m_adsorptionFunctionAllocatorMapping [ false ] = allocateLangmuirAdsorptionFunction;
   m_adsorptionFunctionAllocatorMapping [ true ] = allocateLangmuirAdsorptionTOCFunction;
}

Genex5::AdsorptionFunction* AdsorptionFunctionFactory::getAdsorptionFunction ( AdsorptionProjectHandle* projectHandle, bool isTOCDependent, const std::string& adsorptionCapacityFunctionName ) const {

   AdsorptionFunctionAllocatorMap::const_iterator functionIter = m_adsorptionFunctionAllocatorMapping.find ( isTOCDependent );

   if ( functionIter != m_adsorptionFunctionAllocatorMapping.end ()) {
      return (functionIter->second)( projectHandle, adsorptionCapacityFunctionName );
   } else {
      return 0;
   }

}

AdsorptionFunctionFactory& AdsorptionFunctionFactory::getInstance () {

   if ( s_adsorptionFunctionFactory == 0 ) {
      s_adsorptionFunctionFactory = new AdsorptionFunctionFactory;
   }

   return *s_adsorptionFunctionFactory;
}
