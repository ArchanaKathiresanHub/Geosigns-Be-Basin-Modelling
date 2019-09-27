#ifndef REACTIONRATIO_H
#define REACTIONRATIO_H

#include <string>
#include<iostream>
#include <fstream>
#include<iomanip>
using namespace std;

namespace Genex6
{
class Species;
class ReactionRatio //class to declare the ratio : m_reactant1/m_reactant2=ReactionRatioFunction
{
public:
   ReactionRatio(Species *const in_react1, Species *const in_react2, const std::string &functionCode);
   ~ReactionRatio(){}
   Species *GetReactant1();
   Species *GetReactant2();
   
   double GetRatio(const double preasphaltheneArom) const;

   void OutputOnScreen();
   void OutputOnFile(ofstream &outfile);

private:
   Species *m_reactant1;
   Species *m_reactant2;
   std::string  m_functionCode;   
};

inline Species *ReactionRatio::GetReactant1() 
{
   return m_reactant1;
}
inline Species *ReactionRatio::GetReactant2() 
{
   return m_reactant2;
}

}
#endif
