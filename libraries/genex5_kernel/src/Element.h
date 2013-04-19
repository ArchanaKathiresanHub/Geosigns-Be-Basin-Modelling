#ifndef ELEMENT_H
#define ELEMENT_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

using std::cout;
using std::endl;

namespace Genex5
{
//!A chemical element (C, H etc) that exists in chemical compounds.

class Element
{
public:
   Element();
   virtual ~Element(){ }
   Element(std::string &in_Name) : m_name(in_Name),m_AtomWeight(0) 
   {
   }

   void SetName(std::string &in_Name);
   std::string GetName(){return m_name;}
   void SetAtomWeight(const double &in_AtomWeight);
   double GetAtomWeight()const ;
   void OutputOnScreen()const;
   void OutputOnFile(std::ofstream &outfile) const;

private:
   std::string m_name;
   double m_AtomWeight;
};

inline void Element::SetAtomWeight(const double &in_AtomWeight)
{
   m_AtomWeight = in_AtomWeight;
}
inline double Element::GetAtomWeight()const
{
   return m_AtomWeight;
}
inline void Element::OutputOnScreen() const
{
   cout<<m_name<<","<<m_AtomWeight<<endl;
}
inline void Element::OutputOnFile(std::ofstream &outfile) const
{
   outfile<<m_name<<","<<m_AtomWeight<<endl;
}

}
#endif
