#include "SpeciesState.h"

void Genex6::SpeciesState::SetExpelledMass(const double  in_expelledMass,
                                           const bool    updatePrevious ) 
{

   if ( updatePrevious ) {
      m_previousExpelledMass = m_expelledMass;
   }

   m_expelledMass=in_expelledMass;
}

void Genex6::SpeciesState::setMassExpelledFromSourceRock ( const double expelledMass ) {

   m_previousExpelledMassSR = expelledMass - m_expelledMassSR;
   m_expelledMassSR = expelledMass;   
}

void Genex6::SpeciesState::setMassExpelledTransientFromSourceRock ( const double expelledMassTransient ) {

   m_expelledMassTransientSR = expelledMassTransient;
}

void Genex6::SpeciesState::setExpelledMassTransient ( const double expelled ) {
   m_expelledMassTransient = expelled;
}

void Genex6::SpeciesState::setAdsorptionCapacity ( const double capacity ) {
   m_adsorptionCapacity = capacity;
}

void Genex6::SpeciesState::SetConcentration(const double in_concentration) 
{
   m_concentration[FIRST] = m_concentration[SECOND];
   m_concentration[SECOND] = m_concentration[THIRD];
   m_concentration[THIRD] = in_concentration;
}

void Genex6::SpeciesState::setAdsorpedMol ( const double newValue ) {
   m_adsorpedMol = newValue;
}

void Genex6::SpeciesState::setAdsorpedMass ( const double newValue ) {
   m_adsorpedMass = newValue;
}

void Genex6::SpeciesState::setTransientAdsorpedMass ( const double transientAdsorped ) {
   m_transientAdsorpedMass = transientAdsorped;

}

void Genex6::SpeciesState::setTransientDesorpedMass ( const double transientDesorped ) {
   m_transientDesorpedMass = transientDesorped;
}


void Genex6::SpeciesState::setDesorpedMol ( const double newValue ) {
   m_desorpedMol = newValue;
}

void Genex6::SpeciesState::setFreeMol ( const double newValue ) {
   m_freeMol = newValue;
}

void Genex6::SpeciesState::setExpelledMol ( const double newValue ) {
   m_expelledMol = newValue;
}

void Genex6::SpeciesState::setRetained ( const double retained ) {
   m_retained = retained;
}

void Genex6::SpeciesState::setGeneratedMass ( const double mass ) {
   m_generatedMass = mass;
}

