#ifndef ELEMENT_H
#define ELEMENT_H

#include <iostream>
#include <fstream>
#include<iomanip>
#include <string>

using std::cout;
using std::endl;

namespace Genex6
{
class Element
{
public:
   Element();
   virtual ~Element(){ }
   Element(std::string &in_Name): m_name(in_Name), m_AtomWeight(0) {  m_id = -1; } 

   void SetId( int in_id );
   void SetName(std::string &in_Name);
   std::string GetName();
   int GetId() const;
   void SetAtomWeight(const double in_AtomWeight);
   double GetAtomWeight()const ;
   void OutputOnScreen()const;
   void OutputOnFile(std::ofstream &outfile) const;

private:
   std::string m_name;
   int m_id;
   double m_AtomWeight;
};

inline std::string Element::GetName() { 
   return m_name; 
}
inline int Element::GetId() const { 
   return m_id; 
}
inline void Element::SetId(const int in_id) {
   m_id = in_id;
}
inline void Element::SetAtomWeight(const double in_AtomWeight)
{
    m_AtomWeight = in_AtomWeight;
}
inline double Element::GetAtomWeight()const
{
    return m_AtomWeight;
}
inline void Element::OutputOnScreen() const
{
   cout << m_name << "," << std::setprecision(7) << m_AtomWeight << endl;
}
inline void Element:: OutputOnFile(std::ofstream &outfile) const
{
   
   outfile << m_name << "," << std::setprecision(7) << m_AtomWeight << std::endl;
}

}
#endif
