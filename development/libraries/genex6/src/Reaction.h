#ifndef REACTION_H
#define REACTION_H

#include <vector>
#include <string>
#include <map>
#include<iostream>
#include <fstream>
#include<iomanip>

using namespace std;


namespace Genex6
{
class Species;
class ReactionRatio;

class Reaction
{
public:
   Reaction(Species *const in_theMother);
   virtual ~Reaction();
  
   void AddReactionRatio(ReactionRatio *theRatio);
   void AddProduct(Species *theProduct);
   void SetProductId(const int in_id, Species *theProduct);
   Species *GetProductById(const int in_id); // map used only in the C++ prototype
   Species *GetMother();
   int GetProductIdBySpeciesId( const int id );

   void ComputeProductRatios(const double preasphalteneAromaticity);
   void ComputeMassProductRatios(); // changes the state of the mother object

   //-------------------------------Output functionality--------------------------------------
   void OutputMassFractionsOnScreen();
   void OutputProductsOnScreen();
   void OutputReactionRatiosOnScreen();
   void OutputProductRatiosOnScreen();
   void OutputProductsOnFile(ofstream &outfile);
   void OutputReactionRatiosOnFile(ofstream &outfile);
   void OutputProductRatiosOnFile(ofstream &outfile);

   // Output in VBA sch format
   void OutputReactionRatiosToFile( ofstream &outfile );
   void OutputProductsToFile( ofstream &outfile );

private:
   Species *m_theMother;
   std::vector<Species*> m_theProducts;          //Species cracking product species by product name
   std::vector<ReactionRatio*> m_theReactionRatios; //Reaction ratios among several reactants. Necesssary
                                                    //constraints for the stoichiometric problem solution
   std::map<int,Species*> m_mapId2Product; //map used only in the C++ prototype to cope with original sch conventions
                                           //(e.g the 1 and 2 product have ratio 1.2, where 1 is the first id product
                                           //which might have on the stoichiometry id 5 and name "coke2"...
   std::map<int,double>	m_productRatioBySpeciesName;  //Production ratios after the cracking of 1 Mother Species
                                                      //Obtained as a result of the stoichiometry procedure

   void clearReactionRatios();
};

inline void Reaction::SetProductId(const int in_id, Species *theProduct) 
{
   m_mapId2Product[in_id] = theProduct;
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
