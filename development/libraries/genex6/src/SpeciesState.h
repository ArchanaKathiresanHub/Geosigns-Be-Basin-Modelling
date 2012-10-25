#ifndef _GENEX6__SPECIESSTATE_H_
#define _GENEX6__SPECIESSTATE_H_

namespace Genex6
{
class Species;
class SpeciesState
{
public:
  
   SpeciesState(const Species* species,const double &in_concentration = 0, const double &in_expelledMass = 0);

   virtual ~SpeciesState(){};

   /// \brief Get species for which this is the state.
   const Species* getSpecies () const;

   /// \brief Get the concentration of the species, units=.
   double GetConcentration() const;

   /// \brief Get the expelled mass of the species, units=.
   double GetExpelledMass() const;

   /// \brief Get the expelled mass of the species at the previous time-step, units=.
   double getPreviousExpelledMass () const;

   /// \brief Get the approximation of the concentration of the species, units=.
   double GetConcentrationApproximation() const;   
   
   void SetExpelledMass(const double in_expelledMass,
                        const bool    updatePrevious = true );

   /// \brief Get the generated mass of the species, units=.
   double getGeneratedMass() const;

   /// \brief Get the generated mass of the species, units=.
   void setGeneratedMass ( const double mass );



   void SetConcentration(const double in_concentration);

   void UpdateConcentration(const double in_concentration);

   void setMassExpelledFromSourceRock ( const double expelledMass );

   double getMassExpelledFromSourceRock () const;

   /// Set the mass expelled during the last time-step.
   void setExpelledMassTransient ( const double expelled );

   void setAdsorptionCapacity ( const double capacity );

   double getAdsorptionCapacity () const;


   /// Set a new adsorped value for the species.
   ///
   /// Units are in moles.
   void setAdsorpedMol ( const double newValue );

   /// Get the current adsorped value for the species.
   ///
   /// Units are in moles/m^3.
   double getAdsorpedMol () const;

   /// Set a new desorped value for the species.
   ///
   /// Units are in moles/m^3
   void setDesorpedMol ( const double newValue );

   /// Get the current desorped value for the species.
   ///
   /// Units are in moles/m^3.
   double getDesorpedMol () const;

   /// Set a new free value for the species.
   ///
   /// Units are in moles/m^3.
   void setFreeMol ( const double newValue );

   /// Get the current free value for the species.
   ///
   /// Units are in moles/m^3.
   double getFreeMol () const;

   /// Set a new expelled-mol value for the species.
   ///
   /// Units are in moles/m^3.
   void setExpelledMol ( const double newValue );

   /// Get the current expelled-mol value for the species.
   ///
   /// Units are in moles/m^3.
   double getExpelledMol () const;

   /// Get the mass expelled during the last time-step.
   double getExpelledMassTransient () const;


   /// Set how much of the species has been retained.
   void setRetained ( const double retained );

   /// Get how much of the species has been retained.
   double getRetained () const;



private:

   const Species* m_species;

   enum BUFFER_INDEX{FIRST = 0, SECOND = 1, THIRD = 2, NUMBER_OF_ENTRIES = 3};
   double m_concentration[NUMBER_OF_ENTRIES];
  
   double m_expelledMass; //cumulative expelled masses until current time
   SpeciesState(const SpeciesState &);
   SpeciesState & operator=(const SpeciesState &);

   double m_generatedMass;

   double m_previousExpelledMass;
   double m_expelledMassSR;
   double m_previousExpelledMassSR;

   double m_adsorpedMol;
   double m_desorpedMol;
   double m_freeMol;
   double m_expelledMol;
   double m_expelledMassTransient;
   double m_retained;

   double m_adsorptionCapacity;


};


inline double SpeciesState::GetConcentrationApproximation() const
{

   if ( m_concentration[FIRST] > 0.0 ) {
      // For compatibility with OTGC-5.
      return (3.0 * (m_concentration[THIRD] - m_concentration[SECOND]) + m_concentration[FIRST]); 
   } else {
      return m_concentration[THIRD]; 
   }

}

inline SpeciesState::SpeciesState(const Species* species,const double &in_concentration, const double &in_expelledMass) :
   m_species ( species )

{
   m_concentration[THIRD]  = in_concentration;
   m_concentration[SECOND] = 0.0;
   m_concentration[FIRST]  = 0.0;
   m_expelledMass = in_expelledMass;

   m_generatedMass = 0.0;

   m_adsorpedMol = 0.0;
   m_desorpedMol = 0.0;
   m_freeMol = 0.0;
   m_expelledMol = 0.0;
   m_expelledMassTransient = 0.0;
   m_previousExpelledMass = 0.0;
   m_retained = 0.0;
   m_adsorptionCapacity = 0.0;
   m_expelledMassSR = 0.0;
   m_previousExpelledMassSR = 0.0;

}

inline double SpeciesState::getGeneratedMass () const {
   return m_generatedMass;
}

inline double SpeciesState::GetConcentration() const
{
   return m_concentration[THIRD];
}

inline void SpeciesState::UpdateConcentration(const double in_concentration) 
{
   m_concentration[THIRD] = in_concentration;
}

inline double SpeciesState::GetExpelledMass() const
{
   return m_expelledMass;
}

inline double SpeciesState::getPreviousExpelledMass() const
{
   return m_previousExpelledMass;
}

inline double SpeciesState::getMassExpelledFromSourceRock () const {
   return m_expelledMassSR;
}

inline double SpeciesState::getAdsorpedMol () const {
   return m_adsorpedMol;
}

inline double SpeciesState::getDesorpedMol () const {
   return m_desorpedMol;
}

inline double SpeciesState::getFreeMol () const {
   return m_freeMol;
}

inline double SpeciesState::getExpelledMol () const {
   return m_expelledMol;
}

inline double SpeciesState::getExpelledMassTransient () const {
   return m_expelledMassTransient;
}

inline double SpeciesState::getRetained () const {
   return m_retained;
}

inline double SpeciesState::getAdsorptionCapacity () const {
   return m_adsorptionCapacity;
}

inline const Species* SpeciesState::getSpecies () const {
   return m_species;
}


}
#endif // _GENEX6__SPECIESSTATE_H_
