#include "AdsorptionSimulator.h"

Genex6::AdsorptionSimulator::AdsorptionSimulator (DataAccess::Interface::ProjectHandle& projectHandle,
                                                   const bool isManaged ) :
   m_projectHandle ( projectHandle ) {

   m_adsorptionFunction = nullptr;
   m_priority = DefaultPriority;
   m_isManaged = isManaged;
}

Genex6::AdsorptionSimulator::~AdsorptionSimulator () {

   if ( m_adsorptionFunction != nullptr ) {
      delete m_adsorptionFunction;
      m_adsorptionFunction = nullptr;
   }

}

void Genex6::AdsorptionSimulator::setPriority ( const unsigned int newPriority ) {
   m_priority = newPriority;
}

void Genex6::AdsorptionSimulator::setAdsorptionFunction ( AdsorptionFunction* newAdsorptionFunction ) {
   m_adsorptionFunction = newAdsorptionFunction;
}
