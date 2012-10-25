#ifndef _GENEX5_SPECIESSTATE_H
#define _GENEX5_SPECIESSTATE_H

namespace Genex5
{
class SpeciesState
{
public:
   SpeciesState();
   virtual ~SpeciesState();
    
   SpeciesState(const double &in_concentration, const double &in_expelledMass);
   
   double GetConcentration() const;
   double GetExpelledMass() const;

   double getPreviousExpelledMass () const;
   
   void SetExpelledMass(const double &in_expelledMass,
                        const bool    updatePrevious = true );

   void setMassExpelledFromSourceRock ( const double expelledMass );

   double getMassExpelledFromSourceRock () const;

   void SetConcentration(const double &in_concentration) ;

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
   double m_concentration;
   double m_expelledMass; //cumulative expelled masses until current time
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

   SpeciesState(const SpeciesState &);
   SpeciesState &operator=(const SpeciesState &);
};


inline SpeciesState::SpeciesState(const double &in_concentration, const double &in_expelledMass)
{
   m_concentration = in_concentration;
   m_expelledMass  = in_expelledMass;

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
inline SpeciesState::SpeciesState()
{
   m_concentration = 0.0;
   m_expelledMass = 0.0;
   m_previousExpelledMass = 0.0;
   m_adsorpedMol = 0.0;
   m_desorpedMol = 0.0;
   m_freeMol = 0.0;
   m_expelledMol = 0.0;
   m_expelledMassTransient = 0.0;
   m_retained = 0.0;
   m_adsorptionCapacity = 0.0;
   m_expelledMassSR = 0.0;
   m_previousExpelledMassSR = 0.0;
}
inline SpeciesState::~SpeciesState()
{

}
inline double SpeciesState::GetConcentration() const
{
   return m_concentration;
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


inline SpeciesState::SpeciesState(const SpeciesState &in_state)
{
   m_concentration          = in_state.m_concentration;
   m_expelledMass           = in_state.m_expelledMass;
   m_adsorpedMol            = in_state.m_adsorpedMol;
   m_desorpedMol            = in_state.m_desorpedMol;
   m_freeMol                = in_state.m_freeMol;
   m_expelledMol            = in_state.m_expelledMol;
   m_expelledMassTransient  = in_state.m_expelledMassTransient;
   m_retained               = in_state.m_retained;
   m_expelledMassSR         = in_state.m_expelledMassSR;
   m_previousExpelledMassSR = in_state.m_previousExpelledMassSR;
   m_adsorptionCapacity     = in_state.m_adsorptionCapacity;
}

}
#endif
