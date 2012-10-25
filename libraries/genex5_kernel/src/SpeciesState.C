#include "SpeciesState.h"

namespace Genex5
{
/*
SpeciesState &SpeciesState::operator=(const SpeciesState &in_state)
{
   m_concentration=in_state.m_concentration;
   m_expelledMass=in_state.m_expelledMass;
   return (*this);
}
*/

void SpeciesState::SetExpelledMass(const double &in_expelledMass,
                                   const bool    updatePrevious ) 
{

   if ( updatePrevious ) {
      m_previousExpelledMass = m_expelledMass;
   }

   m_expelledMass=in_expelledMass;
}

void SpeciesState::setMassExpelledFromSourceRock ( const double expelledMass ) {

   m_previousExpelledMassSR = expelledMass - m_expelledMassSR;
   m_expelledMassSR = expelledMass;   
}

void SpeciesState::setExpelledMassTransient ( const double expelled ) {
   m_expelledMassTransient = expelled;
}

void SpeciesState::setAdsorptionCapacity ( const double capacity ) {
   m_adsorptionCapacity = capacity;
}

void SpeciesState::SetConcentration(const double &in_concentration) 
{
   m_concentration=in_concentration;
}

void SpeciesState::setAdsorpedMol ( const double newValue ) {
   m_adsorpedMol = newValue;
}

void SpeciesState::setDesorpedMol ( const double newValue ) {
   m_desorpedMol = newValue;
}

void SpeciesState::setFreeMol ( const double newValue ) {
   m_freeMol = newValue;
}

void SpeciesState::setExpelledMol ( const double newValue ) {
   m_expelledMol = newValue;
}

void SpeciesState::setRetained ( const double retained ) {
   m_retained = retained;
}




}
