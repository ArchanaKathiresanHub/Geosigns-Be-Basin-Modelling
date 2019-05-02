#ifndef SPECIESPROPERTIES_H
#define SPECIESPROPERTIES_H

//activation parameters
//reaction  1:dUa 2:dUb   3:dS 4:dV  5:order
//diffusion 6:dUa 7:spare 8:dS 9:dV 10:jump length
            
#include<iostream>
#include <fstream>
#include<iomanip>
using namespace std;

namespace Genex6
{
class Species;  

class SpeciesProperties
{
public:
   ~SpeciesProperties(){}
   SpeciesProperties(Species *const in_Elem,
                      const double &in_activationEnergy1,
                      const double &in_activationEnergy2,
                      const double &in_entropy,
                      const double &in_volume,
                      const double &in_reactionOrder,
                      const double &in_diffusionEnergy1,
                      const double &in_diffusionEnergy2,
                      const double &in_jumpLength,
                      const double &in_density,
                      const double &in_B0,
                      const double &in_Aromaticity
                      );

   double GetActivationEnergy1() const;
   double GetActivationEnergy2() const;
   double GetEntropy() const;
   double GetVolume() const;
   double GetReactionOrder() const;
   double GetDiffusionEnergy1() const;
   double GetAromaticity() const;
   double GetB0()const;
   double GetB0radical()const;

   double GetDiffusionEnergy2() const;
   double GetJumpLength() const; 
   double GetDensity() const;
   double GetMolWeight() const;

   bool IsMobile();
   bool IsReactive();
   bool IsHC();
   bool IsOil();
   bool IsHCgas();

   void SetAromaticity(const double aromaticity );
   void SetB0(const double &in_B0);
   void SetB0radical(const double &in_B0radical);
   void SetActivationEnergy1(const double &in_activationEnergy1);
   void SetActivationEnergy2(const double &in_activationEnergy2);
   void SetReactionOrder(const double &in_reactionOrder);
   void SetMolWeight(const double &in_molWeight);
   void SetDiffusionEnergy1(const double &in_diffusionEnergy1);

   void OutputOnScreen() const;
   void OutputOnFile(ofstream &outfile) const;
   void OutputToSchFile(ofstream &outfile) const;

   void Update();

   void PrintBenchmarkProperties(ofstream &outfile) const;

private:
   Species *const m_theSpecies;
   double m_activationEnergy1;     //     UNITS
   double m_activationEnergy2;     //     UNITS
   double m_entropy;               //     UNITS
   double m_volume;                //     UNITS
   double m_reactionOrder;         //     UNITS
   double m_diffusionEnergy1;      //     UNITS
   double m_diffusionEnergy2;      //     UNITS  
   double m_jumpLength;            //     UNITS
   double m_density;               //     UNITS
   double m_molWeight;             //     UNITS, computed by  Species::ComputeMolWeight()
   double m_B0;
   double m_B0radical;
   double m_aromaticity;
   bool m_mobile;
   bool m_reactive;
   bool m_HC;
   bool m_Oil;
   bool m_HCgas;

   void CheckValueBounds();
};
inline double SpeciesProperties::GetActivationEnergy1() const
{
 return m_activationEnergy1;
}
inline double SpeciesProperties::GetActivationEnergy2() const
{
 return m_activationEnergy2;
}
inline double SpeciesProperties::GetEntropy()const
{
 return m_entropy;
}
inline double SpeciesProperties::GetVolume() const
{
 return m_volume;
}
inline double SpeciesProperties::GetReactionOrder()const
{
 return m_reactionOrder;
}
inline double SpeciesProperties::GetDiffusionEnergy1()const
{
 return m_diffusionEnergy1;
}
inline double SpeciesProperties::GetDiffusionEnergy2()const
{
 return m_diffusionEnergy2;
}
inline double SpeciesProperties::GetJumpLength()const
{
 return m_jumpLength;
}
inline double SpeciesProperties::GetDensity()const
{
 return m_density;
}
inline double SpeciesProperties::GetMolWeight() const
{
 return m_molWeight;
}
inline void SpeciesProperties::SetActivationEnergy1(const double &in_activationEnergy1)
{
 m_activationEnergy1 = in_activationEnergy1;
}
inline void SpeciesProperties::SetActivationEnergy2(const double &in_activationEnergy2)
{
 m_activationEnergy2 = in_activationEnergy2;
}
inline void SpeciesProperties::SetReactionOrder(const double &in_reactionOrder)
{
 m_reactionOrder = in_reactionOrder;
}
inline void SpeciesProperties::SetMolWeight(const double &in_molWeight)
{
   m_molWeight = in_molWeight;
}
inline bool SpeciesProperties::IsMobile()
{
   return m_mobile;
}
inline bool SpeciesProperties::IsReactive()
{
   return m_reactive;
}
inline bool SpeciesProperties::IsHC()
{
   return m_HC;
}
inline bool SpeciesProperties::IsOil()
{
   return m_Oil;
}
inline bool SpeciesProperties::IsHCgas()
{
   return m_HCgas;
}
inline double SpeciesProperties::GetAromaticity() const
{
  return m_aromaticity;
}
inline double SpeciesProperties::GetB0()const
{
   return m_B0;
}
inline double SpeciesProperties::GetB0radical()const
{
   return m_B0radical;
}
inline void SpeciesProperties::SetB0(const double &in_B0)
{
   m_B0 = in_B0;
}
inline void SpeciesProperties::SetB0radical(const double &in_B0radical)
{
   m_B0radical = in_B0radical;
}
inline void SpeciesProperties::SetDiffusionEnergy1(const double &in_diffusionEnergy1)
{
   m_diffusionEnergy1 = in_diffusionEnergy1;
}
inline void SpeciesProperties::SetAromaticity(const double aromaticity ) {
   m_aromaticity = aromaticity;
}

}

#endif//SPECIESPROPERTIES_H
