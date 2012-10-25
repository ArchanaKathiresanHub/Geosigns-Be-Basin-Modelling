#ifndef _OTGC_SPECIESSTATE_H
#define _OTGC_SPECIESSTATE_H

#include <iostream>

namespace OTGC
{
class SpeciesState
{
public:
   SpeciesState();
   ~SpeciesState();
   
   SpeciesState(const double &in_concentration);
   
   double GetConcentration() const;
   double GetConcentrationApproximation() const;   
  
   void SetConcentration(const double &in_concentration) ;
   

private:
   enum BUFFER_INDEX{FIRST = 0, SECOND = 1, THIRD = 2, NUMBER_OF_ENTRIES = 3};
   double m_concentration[NUMBER_OF_ENTRIES];
  
   SpeciesState(const SpeciesState &);
   SpeciesState & operator=(const SpeciesState &);
};


inline double SpeciesState::GetConcentrationApproximation() const
{
   return   ( m_concentration[ FIRST ] > 0.0 ? ( 3.0 * ( m_concentration[ THIRD ] - m_concentration[ SECOND ] ) + m_concentration[ FIRST ] ) : m_concentration[ THIRD ] );

}
inline SpeciesState::SpeciesState(const double &in_concentration)
{
   m_concentration[ THIRD ]  = in_concentration;
   m_concentration[ SECOND ] = 0.0;
   m_concentration[ FIRST ]  = 0.0;

}
inline SpeciesState::SpeciesState()
{
   for(int i = 0; i < NUMBER_OF_ENTRIES; i++)
   {
       m_concentration[ i ] = 0.0;
   }
}
inline SpeciesState::~SpeciesState()
{

}
inline double SpeciesState::GetConcentration() const
{
   return m_concentration[ THIRD ];
}
inline void SpeciesState::SetConcentration(const double &in_concentration) 
{
   m_concentration[ FIRST ] = m_concentration[ SECOND ];
   m_concentration[ SECOND] = m_concentration[ THIRD ];
   m_concentration[ THIRD ] = in_concentration;
}


}
#endif
