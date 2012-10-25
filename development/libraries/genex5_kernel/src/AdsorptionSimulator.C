#include "AdsorptionSimulator.h"

Genex5::AdsorptionSimulator::AdsorptionSimulator () {
   m_adsorptionFunction = 0;
}

Genex5::AdsorptionSimulator::~AdsorptionSimulator () {

   if ( m_adsorptionFunction != 0 ) {
      delete m_adsorptionFunction;
      m_adsorptionFunction = 0;
   }

}

void Genex5::AdsorptionSimulator::setAdsorptionFunction ( AdsorptionFunction* newAdsorptionFunction ) {
   m_adsorptionFunction = newAdsorptionFunction;
}
