#ifndef SPECIESRESULT_H
#define SPECIESRESULT_H


namespace Genex5
{
class SpeciesResult
{
public:
   SpeciesResult();
   ~SpeciesResult();

   SpeciesResult & operator=(const SpeciesResult &theResult);
   void SetConcentration(const double &in_conc);
   void SetFlux(const double &in_flux);
   void SetExpelledMass(const double &in_expelledMass);
   void SetGeneratedRate(const double &in_generatedRate);

   double GetConcentration() const;
   double GetFlux() const;
   double GetExpelledMass() const;
   double GetGeneratedRate() const;

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
   double m_flux;
   double m_expelledMass;
   double m_generatedRate;

   double m_adsorpedMol;
   double m_freeMol;
   double m_expelledMol;

};
inline SpeciesResult::SpeciesResult()
{
   m_concentration = m_flux = m_expelledMass = m_generatedRate = 0.0;
}
inline SpeciesResult::~SpeciesResult()
{

}
inline void SpeciesResult::SetGeneratedRate(const double &in_generatedRate)
{
   m_generatedRate = in_generatedRate;
}
inline double SpeciesResult::GetGeneratedRate() const
{
   return m_generatedRate;
}
inline void SpeciesResult::SetConcentration(const double &in_conc)
{
  m_concentration = in_conc; 
}
inline void SpeciesResult::SetFlux(const double &in_flux)
{
  m_flux = in_flux; 
}
inline void SpeciesResult::SetExpelledMass(const double &in_expelledMass)
{
  m_expelledMass = in_expelledMass; 
}

inline double SpeciesResult::GetConcentration() const
{
  return m_concentration; 
}
inline double SpeciesResult::GetFlux() const
{
  return m_flux; 
}
inline double SpeciesResult::GetExpelledMass() const
{
  return m_expelledMass; 
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
