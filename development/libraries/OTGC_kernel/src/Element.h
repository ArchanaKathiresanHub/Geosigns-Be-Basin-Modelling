#ifndef ELEMENT_H
#define ELEMENT_H

#include <string>
#include<iostream>
#include <fstream>
#include<iomanip>



namespace OTGC
{
class Element
{
public:
  Element();
  ~Element(){}
  Element(std::string &in_Name):m_name(in_Name){ }

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
   std::cout<<m_name<<","<<m_AtomWeight<<std::endl;
}
inline void Element:: OutputOnFile(std::ofstream &outfile) const
{
   outfile<<m_name<<","<<m_AtomWeight<<std::endl;
}

}
#endif
