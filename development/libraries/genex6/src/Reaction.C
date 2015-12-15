#include "Reaction.h"
#include "SCmathlib.h" 
#include "Species.h"
#include "SpeciesManager.h"
#include "ReactionRatio.h"

namespace Genex6
{
Reaction::Reaction(Species *const in_theMother): m_theMother(in_theMother) 
{
   m_theProducts.reserve(Genex6::SpeciesManager::numberOfSpecies);
}
Reaction::~Reaction() 
{
   clearReactionRatios();
}
void Reaction::clearReactionRatios() 
{
   std::vector<ReactionRatio*>::iterator itEnd  = m_theReactionRatios.end();
   for(std::vector<ReactionRatio*>::iterator it =  m_theReactionRatios.begin(); it != itEnd; ++it) {
      delete (*it);
   } 
   m_theReactionRatios.clear();
}
void Reaction::ComputeProductRatios(const double preasphaltheneArom) 
{
   unsigned int i, row;
   const std::string::size_type equationsFromMother = m_theMother->GetNumberOfElements();
   const std::string::size_type equationsFromRatios = m_theReactionRatios.size();
   const std::string::size_type verifyFromProducts = m_theProducts.size();

   const vector<Species*>::size_type sz = m_theProducts.size();

   //solving only for these for the moment, the rest require further preprocessing steps, CompEarly,KineticsEarly
   if((equationsFromMother + equationsFromRatios) == verifyFromProducts) {
      //set up the temporal SpeciesName2Equation map
      std::map<int,int>::size_type solutionIndex=0;
      std::map<int,int> mapProductName2SolutionVector;

      for( i = 0; i < sz; ++ i ) {
         mapProductName2SolutionVector[m_theProducts[i]->GetId()] = solutionIndex;
         ++ solutionIndex;
      }
      //initiaze the matrix A and the vectors X and B to 0.0
      SCMatrix A(verifyFromProducts);
      SCVector B(verifyFromProducts);
      SCVector X(verifyFromProducts);

      //set up the system of linear equations
      row = solutionIndex = 0;

      int * motherCompositionCodeIds = m_theMother->GetCompositionCodeIds();
      unsigned int theCompositionCodeLength = m_theMother->GetCompositionCodeLength();

      std::map<int,int>::iterator iterIndex;
      //the Mother Element part
      for(row = 0; row < theCompositionCodeLength; ++ row) {
         double MotherElementCompositionFactor = m_theMother->GetCompositionByElement(motherCompositionCodeIds[row]);
         B(row) = MotherElementCompositionFactor;

         for( i = 0; i < sz; ++ i ) {
            double ProductElementCompositionFactor = 
               m_theProducts[i]->GetCompositionByElement(motherCompositionCodeIds[row]);
            iterIndex = mapProductName2SolutionVector.find(m_theProducts[i]->GetId()); 
            solutionIndex = iterIndex->second;
            A(row,solutionIndex) = ProductElementCompositionFactor;
         }
      }
      //the Reaction Ratios Part
      for(row = theCompositionCodeLength; row < verifyFromProducts; ++ row) {
         ReactionRatio *const currRatio = m_theReactionRatios[row-theCompositionCodeLength];
         Species *const Reactant1 = currRatio->GetReactant1();
         Species *const Reactant2 = currRatio->GetReactant2();

         //First Reactant
         iterIndex = mapProductName2SolutionVector.find(Reactant1->GetId());
         solutionIndex = iterIndex->second;
         A(row,solutionIndex) = 1.0;
         //Second Reactant
         iterIndex = mapProductName2SolutionVector.find(Reactant2->GetId());
         solutionIndex = iterIndex->second;
         A(row,solutionIndex) = currRatio->GetRatio(preasphaltheneArom);
      }
      //Solve the System
      //GMRES(verifyFromProducts,A,B,X);
      //BiCGStab (A,B,X);
      GEPP(A,B,X); 
 
      //Assign solution to reaction product ratio factor map: m_productRatioBySpeciesName
      iterIndex = mapProductName2SolutionVector.begin();
      while(iterIndex != mapProductName2SolutionVector.end()) {
         m_productRatioBySpeciesName[iterIndex->first] = X(iterIndex->second);
         ++ iterIndex;
      }
      mapProductName2SolutionVector.clear();
   }
}
void Reaction::ComputeMassProductRatios()  //changes the state of the mother object
{
   const double motherMolWeight = m_theMother->GetMolWeight();
   std::vector<Species*>::size_type sz = m_theProducts.size();

   for(unsigned int i = 0; i < sz; ++ i ) {
      const Species *const theProduct = m_theProducts[i];
      const double productMolWeight = theProduct->GetMolWeight();
      std::map<int,double>::const_iterator itRatios = m_productRatioBySpeciesName.find(theProduct->GetId());
      if(itRatios == m_productRatioBySpeciesName.end()) 
         continue;
      const double productRatio = itRatios->second;
      const double productMassFactor = (productRatio * productMolWeight) / motherMolWeight;
      m_theMother->UpdateMassFactorBySpeciesName(theProduct->GetId(), productMassFactor);
   }
   m_theMother->validate(); // check for negative coefficients 
  // OutputProductsOnScreen();
  // OutputProductRatiosOnScreen();
  // OutputReactionRatiosOnScreen();
}
void Reaction::AddProduct(Species *theProduct) 
{
   m_theProducts.push_back( theProduct );
}
void Reaction::OutputProductsOnScreen() 
{
   cout << m_theMother->GetName();
   std::vector<Species*>::size_type sz =m_theProducts.size();
   for(unsigned int i = 0; i < sz; ++ i ) {
      cout << "," << m_theProducts[i]->GetName();
   }
   cout << endl;
}

void Reaction::OutputProductRatiosOnScreen() 
{
   cout << m_theMother->GetName();
   cout << "," << 1;
   std::map<int,double>::iterator it = m_productRatioBySpeciesName.begin();
   while(it != m_productRatioBySpeciesName.end()) {
      cout << "," << it->second;
      ++ it;
   }
   cout << endl;
}
void Reaction::OutputProductsOnFile(ofstream &outfile) 
{
   outfile << m_theMother->GetName();
   std::vector<Species*>::size_type sz = m_theProducts.size();
   for(unsigned int i = 0; i < sz; ++ i ) {
      outfile << "," << m_theProducts[i]->GetName();
   }
   outfile << endl;
}
void Reaction::OutputProductRatiosOnFile(ofstream &outfile) 
{
   outfile << m_theMother->GetName();
   outfile << "," << 1;
   std::map<int,double>::iterator it = m_productRatioBySpeciesName.begin();
   while(it != m_productRatioBySpeciesName.end()) {
      outfile << "," << it->second;
      ++ it;
   }
   outfile << endl;
}
void Reaction::OutputReactionRatiosOnScreen() 
{
   std::vector<ReactionRatio*>::iterator itR = m_theReactionRatios.begin();
   while(itR != m_theReactionRatios.end()) {
      cout << m_theMother->GetName() << ",";
      (*itR)->OutputOnScreen();
      ++ itR;
   }  
}
void Reaction::OutputReactionRatiosOnFile(ofstream &outfile) 
{
   std::vector<ReactionRatio*>::iterator itR = m_theReactionRatios.begin();
   while(itR != m_theReactionRatios.end()) {
      outfile << m_theMother->GetName() << ",";
      (*itR)->OutputOnFile(outfile);
      ++ itR;
   }
}

Species *Reaction::GetProductById(const int in_id) 
{
   Species *ret = 0;
   std::map<int,Species*>::iterator it = m_mapId2Product.find(in_id);
   if(it != m_mapId2Product.end()) {
      ret = (it->second);  
   }
   return ret;
}

}
