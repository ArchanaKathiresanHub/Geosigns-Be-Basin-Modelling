#include "SpeciesResult.h"
namespace Genex5
{
SpeciesResult & SpeciesResult::operator=(const SpeciesResult &theResult)
{
   m_concentration = theResult.m_concentration;
   m_flux          = theResult.m_flux;
   m_expelledMass  = theResult.m_expelledMass;
   m_generatedRate = theResult.m_generatedRate;
   return (*this);
}
}
