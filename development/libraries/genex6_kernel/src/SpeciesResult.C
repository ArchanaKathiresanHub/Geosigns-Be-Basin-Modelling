#include "SpeciesResult.h"
namespace Genex6
{
   SpeciesResult::SpeciesResult (const SpeciesResult &theResult) {
 
      m_concentration = theResult.m_concentration;
      m_concentrationPrev = theResult.m_concentrationPrev;
      m_flux          = theResult.m_flux;
      m_expelledMass  = theResult.m_expelledMass;
      m_generatedMass = theResult.m_generatedMass;
      m_generatedRate = theResult.m_generatedRate;
      m_generatedCum  = theResult.m_generatedCum;
      m_adsorpedMol   = theResult.m_adsorpedMol;
      m_freeMol       = theResult.m_freeMol;
      m_expelledMol   = theResult.m_expelledMol;
      m_expelledMassTransient = theResult.m_expelledMassTransient;
      m_expelledMassPrev  = theResult.m_expelledMassPrev;
   }

   SpeciesResult & SpeciesResult::operator=(const SpeciesResult &theResult)
   {
      m_concentration = theResult.m_concentration;
      m_concentrationPrev = theResult.m_concentrationPrev;
      m_flux          = theResult.m_flux;
      m_expelledMass  = theResult.m_expelledMass;
      m_generatedMass = theResult.m_generatedMass;
      m_generatedRate = theResult.m_generatedRate;
      m_generatedCum  = theResult.m_generatedCum;
      m_adsorpedMol   = theResult.m_adsorpedMol;
      m_freeMol       = theResult.m_freeMol;
      m_expelledMol   = theResult.m_expelledMol;
      m_expelledMassTransient = theResult.m_expelledMassTransient;
      m_expelledMassPrev  = theResult.m_expelledMassPrev;
      
      return (*this);
   }

}
