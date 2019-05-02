#ifndef SPECIESRESULT_H
#define SPECIESRESULT_H


namespace Genex6
{
class SpeciesResult
{
public:
   SpeciesResult();
   SpeciesResult( const SpeciesResult &theResult );

   ~SpeciesResult();

   SpeciesResult & operator=(const SpeciesResult &theResult);
   void clean();

   void SetConcentration(const double in_conc);
   void SetConcentrationPrev(const double in_conc);
   void SetFlux(const double in_flux);
   void SetExpelledMass(const double in_expelledMass);
   void SetExpelledMassPrev(const double in_expelledMass);
   void setGeneratedMass ( const double generatedMass );
   void SetGeneratedRate(const double in_generatedRate);
   void SetGeneratedCum(const double in_generatedCum);
   void SetExpelledMassTransient(const double in_mass);
   
   double GetConcentration() const;
   double GetConcentrationPrev() const;
   double GetFlux() const;
   double GetExpelledMass() const;
   double GetExpelledMassPrev() const;
   double getGeneratedMass () const;
   double GetGeneratedRate() const;
   double GetGeneratedCum() const;
   double GetExpelledMassTransient() const;

   /// Set a new adsorped value for the species.
   ///
   /// Units are in moles.
   void setAdsorpedMol ( const double newValue );

   /// Get the current adsorped value for the species.
   ///
   /// Units are in moles.
   double getAdsorpedMol () const;

   /// Set a new free value for the species.
   ///
   /// Units are in moles.
   void setFreeMol ( const double newValue );

   /// Get the current free value for the species.
   ///
   /// Units are in moles.
   double getFreeMol () const;

   /// Set a new expelled-mol value for the species.
   ///
   /// Units are in moles.
   void setExpelledMol ( const double newValue );

   /// Get the current expelled-mol value for the species.
   ///
   /// Units are in moles.
   double getExpelledMol () const;

private:
   double m_concentration;             
   double m_concentrationPrev;             
   double m_flux;
   double m_expelledMass;
   double m_expelledMassPrev;
   double m_generatedMass;
   double m_generatedRate;
   double m_generatedCum;
   double m_expelledMassTransient;

   double m_adsorpedMol;
   double m_freeMol;
   double m_expelledMol;

};
inline void SpeciesResult::clean()
{
   m_concentration = 0.0;
   m_concentrationPrev = 0.0;
   m_flux = 0.0;
   m_expelledMass = 0.0;
   m_generatedMass = 0.0;
   m_generatedRate = 0.0;
   m_adsorpedMol = 0.0;
   m_freeMol = 0.0;
   m_expelledMol = 0.0;
   m_generatedCum = 0.0;
   m_expelledMassTransient = 0.0;
   m_expelledMassPrev = 0.0;
}
inline SpeciesResult::SpeciesResult()
{
   clean();
}
inline SpeciesResult::~SpeciesResult()
{

}

inline void SpeciesResult::SetGeneratedRate(const double in_generatedRate)
{
   m_generatedRate = in_generatedRate;
}
inline double SpeciesResult::GetGeneratedRate() const
{
   return m_generatedRate;
}
inline void SpeciesResult::SetGeneratedCum(const double in_generatedCum)
{
   m_generatedCum = in_generatedCum;
}
inline double SpeciesResult::GetGeneratedCum() const
{
   return m_generatedCum;
}
inline void SpeciesResult::SetExpelledMassTransient( const double in_mass )
{
   m_expelledMassTransient = in_mass;
}
inline double SpeciesResult::GetExpelledMassTransient() const
{
   return m_expelledMassTransient;
}
inline void SpeciesResult::SetConcentration(const double in_conc)
{
  m_concentration = in_conc; 
}
inline void SpeciesResult::SetConcentrationPrev(const double in_conc)
{
  m_concentrationPrev = in_conc; 
}
inline void SpeciesResult::SetFlux(const double in_flux)
{
  m_flux = in_flux; 
}
inline void SpeciesResult::SetExpelledMass(const double in_expelledMass)
{
  m_expelledMass = in_expelledMass; 
}
inline void SpeciesResult::SetExpelledMassPrev(const double in_expelledMass)
{
  m_expelledMassPrev = in_expelledMass; 
}
inline void SpeciesResult::setGeneratedMass(const double generatedMass)
{
  m_generatedMass = generatedMass; 
}
inline double SpeciesResult::GetConcentration() const
{
  return m_concentration; 
}
inline double SpeciesResult::GetConcentrationPrev() const
{
  return m_concentrationPrev; 
}
inline double SpeciesResult::GetFlux() const
{
  return m_flux; 
}
inline double SpeciesResult::GetExpelledMass() const
{
  return m_expelledMass; 
}
inline double SpeciesResult::GetExpelledMassPrev() const
{
  return m_expelledMassPrev; 
}

inline double SpeciesResult::getGeneratedMass () const
{
  return m_generatedMass; 
}

inline void SpeciesResult::setAdsorpedMol ( const double newValue ) {
   m_adsorpedMol = newValue;
}

inline double SpeciesResult::getAdsorpedMol () const {
   return m_adsorpedMol;
}

inline void SpeciesResult::setFreeMol ( const double newValue ) {
   m_freeMol = newValue;
}

inline double SpeciesResult::getFreeMol () const {
   return m_freeMol;
}

inline void SpeciesResult::setExpelledMol ( const double newValue ) {
   m_expelledMol = newValue;
}

inline double SpeciesResult::getExpelledMol () const {
   return m_expelledMol;
}

}
#endif
