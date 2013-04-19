#ifndef REACTION_H
#define REACTION_H

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;


namespace Genex5
{
class Species;
class ReactionRatio;

//!Description of a chemical reaction.
/*!
* Reaction contains the full description of a particular chemical reaction containing the parent and child species and the corresponding reaction ratios. This  information is used to set up the stoichiometric equation whose solution contains the stochiometric factors that are necessary in order to perform the time integration of the nonlinear ODEs.
*/
class Reaction
{
public:
   Reaction(Species *const in_theMother);
   virtual ~Reaction();
  
   void AddProduct(Species *theProduct);
   void SetProductId(const int &in_id,Species *theProduct);
   Species *GetProductById(const int &in_id);////map used only in the C++ prototype
   void AddReactionRatio(ReactionRatio *theRatio);
   Species *GetMother();

   void ComputeProductRatios(const double &preasphalteneAromaticity);
   void ComputeMassProductRatios();//changes the state of the mother object

   //-------------------------------Output functionality--------------------------------------
   void OutputProductsOnScreen();
   void OutputReactionRatiosOnScreen();
   void OutputProductRatiosOnScreen();
   void OutputProductsOnFile(ofstream &outfile);
   void OutputReactionRatiosOnFile(ofstream &outfile);
   void OutputProductRatiosOnFile(ofstream &outfile);
protected:
   void clearReactionRatios();
private:
   Species *m_theMother;
   std::map<std::string,Species*>  m_theProducts;                 //Species cracking product species by product name
   std::vector<ReactionRatio*>     m_theReactionRatios;           //Reaction ratios among several reactants. Necesssary
                                                                  //constraints for the stoichiometric problem solution
   std::map<int,Species*>          m_mapId2Product;               //map used only in the C++ prototype to cope with original sch conventions
                                                                  //(e.g the 1 and 2 product have ratio 1.2, where 1 is the first id product
                                                                  //which might have on the stoichiometry id 5 and name "coke2"...
   std::map<std::string,double>	  m_productRatioBySpeciesName;   //Production ratios after the cracking of 1 Mother Species
                                                                  //Obtained as a result of the stoichiometry procedure
};
inline void Reaction::SetProductId(const int &in_id,Species *theProduct)
{
   m_mapId2Product[in_id]=theProduct;
}
inline void Reaction::AddReactionRatio(ReactionRatio *theRatio)
{
   m_theReactionRatios.push_back(theRatio);
}
inline Species *Reaction::GetMother() 
{
   return m_theMother;
}
}
#endif
