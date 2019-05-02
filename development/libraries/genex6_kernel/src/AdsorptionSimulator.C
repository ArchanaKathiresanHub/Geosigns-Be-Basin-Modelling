#include "AdsorptionSimulator.h"

Genex6::AdsorptionSimulator::AdsorptionSimulator ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                   const bool isManaged ) :
   m_projectHandle ( projectHandle ) {

   m_adsorptionFunction = 0;
   m_adsorptionCalculator = 0;
   m_priority = DefaultPriority;
   m_isManaged = isManaged;
}

Genex6::AdsorptionSimulator::~AdsorptionSimulator () {

   if ( m_adsorptionFunction != 0 ) {
      delete m_adsorptionFunction;
      m_adsorptionFunction = 0;
   }

}

void Genex6::AdsorptionSimulator::setPriority ( const unsigned int newPriority ) {
   m_priority = newPriority;
}

void Genex6::AdsorptionSimulator::setAdsorptionFunction ( AdsorptionFunction* newAdsorptionFunction ) {
   m_adsorptionFunction = newAdsorptionFunction;
}

void Genex6::AdsorptionSimulator::setSimulatorStateAdsorption( SimulatorStateAdsorption * state ) {
   m_adsorptionCalculator = state;
}

double Genex6::AdsorptionSimulator::getOrganoPorosity() const {

   if( m_adsorptionCalculator != 0 ) {
      return m_adsorptionCalculator->getOrganoPorosity();
   } else {
      return -1.0;
   }
}
double Genex6::AdsorptionSimulator::getPessureLangmuir() const {

   if( m_adsorptionCalculator != 0 ) {
      return m_adsorptionCalculator->getPessureLangmuir();
   } else {
      return -1.0;
   }
}
double Genex6::AdsorptionSimulator::getPessurePrangmuir() const {

   if( m_adsorptionCalculator != 0 ) {
      return m_adsorptionCalculator->getPessurePrangmuir();
   } else {
      return -1.0;
   }
}
double Genex6::AdsorptionSimulator::getPessureJGS() const {

   if( m_adsorptionCalculator != 0 ) {
      return m_adsorptionCalculator->getPessureJGS();
   } else {
      return -1.0;
   }
}
double Genex6::AdsorptionSimulator::getAdsGas() const {

   if( m_adsorptionCalculator != 0 ) {
      return m_adsorptionCalculator->getAdsGas();
   } else {
      return -1.0;
   }
}
double Genex6::AdsorptionSimulator::getFreeGas() const {

   if( m_adsorptionCalculator != 0 ) {
      return m_adsorptionCalculator->getFreeGas();
   } else {
      return -1.0;
   }
}
double Genex6::AdsorptionSimulator::getNmaxAds() const {

   if( m_adsorptionCalculator != 0 ) {
      return m_adsorptionCalculator->getNmaxAds();
   } else {
      return -1.0;
   }
}
double Genex6::AdsorptionSimulator::getNmaxFree() const {

   if( m_adsorptionCalculator != 0 ) {
      return m_adsorptionCalculator->getNmaxFree();
   } else {
      return -1.0;
   }
}
double Genex6::AdsorptionSimulator::getMolesGas() const {

   if( m_adsorptionCalculator != 0 ) {
      return m_adsorptionCalculator->getMolesGas();
   } else {
      return -1.0;
   }
}
double Genex6::AdsorptionSimulator::getMolarGasVol() const {

   if( m_adsorptionCalculator != 0 ) {
      return m_adsorptionCalculator->getMolarGasVol();
   } else {
      return -1.0;
   }
}
