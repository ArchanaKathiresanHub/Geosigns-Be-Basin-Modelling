#include "Reaction.h"
#include "SCmathlib.h" 
#include "Species.h"
#include "ReactionRatio.h"

namespace OTGC
{
Reaction::~Reaction()
{
   clearReactionRatios();
}
void Reaction::clearReactionRatios()
{
   std::vector<ReactionRatio*>::iterator itEnd  = m_theReactionRatios.end();
   for(std::vector<ReactionRatio*>::iterator it =  m_theReactionRatios.begin(); it != itEnd; ++it) 
   {
      delete (*it);
   } 
   m_theReactionRatios.clear();
}
void Reaction::ComputeProductRatios(const double &preasphaltheneArom)
{
   const std::string::size_type equationsFromMother=m_theMother->GetNumberOfElements();
   const std::string::size_type equationsFromRatios=m_theReactionRatios.size();
   const std::string::size_type verifyFromProducts=m_theProducts.size();

   //solving only for these for the moment, the rest require further preprocessing steps, CompEarly,KineticsEarly
   if((equationsFromMother+equationsFromRatios)==verifyFromProducts)
   {
      //set up the temporal SpeciesName2Equation map
      std::map<std::string,int>::size_type solutionIndex=0;
      std::map<std::string,int> mapProductName2SolutionVector;
      std::map<std::string,Species*>::iterator it=m_theProducts.begin();
      while(it!=m_theProducts.end())
      {
         mapProductName2SolutionVector[it->first]=solutionIndex;
         it++;
         solutionIndex++;
      }
      //initiaze the matrix A and the vectors X and B to 0.0
      SCMatrix A(verifyFromProducts);
      SCVector B(verifyFromProducts);
      SCVector X(verifyFromProducts);

      //set up the system of linear equations
      std::string::size_type row;
      row=solutionIndex=0;
      std::string motherCompositionCode=m_theMother->GetCompositionCode();

      //the Mother Element part
      for(row=0;row<motherCompositionCode.size();row++)
      {
         std::string Element;
         Element=motherCompositionCode[row];
         double MotherElementCompositionFactor=m_theMother->GetCompositionByElement(Element);
         B(row)=MotherElementCompositionFactor;

         it=m_theProducts.begin();
         while(it!=m_theProducts.end())
         {
            Species* curSpecies=it->second;
            double ProductElementCompositionFactor=curSpecies->GetCompositionByElement(Element);
            std::map<std::string,int>::iterator iterIndex=mapProductName2SolutionVector.find(it->first); 
            solutionIndex=iterIndex->second;
            A(row,solutionIndex)=ProductElementCompositionFactor;
            it++;
         }
      }
      //the Reaction Ratios Part
      for(row=motherCompositionCode.size();row<verifyFromProducts;row++)
      {
         ReactionRatio *const currRatio=m_theReactionRatios[row-motherCompositionCode.size()];
         Species *const Reactant1=currRatio->GetReactant1();
         Species *const Reactant2=currRatio->GetReactant2();
         std::map<std::string,int>::iterator iterIndex;
         //First Reactant
         iterIndex=mapProductName2SolutionVector.find(Reactant1->GetName());
         solutionIndex=iterIndex->second;
         A(row,solutionIndex)=1.0;
         //Second Reactant
         iterIndex=mapProductName2SolutionVector.find(Reactant2->GetName());
         solutionIndex=iterIndex->second;
         A(row,solutionIndex)=currRatio->GetRatio(preasphaltheneArom);
      }
      //Solve the System
      //GMRES(verifyFromProducts,A,B,X);
      GEPP(A,B,X);

      //Assign solution to reaction product ratio factor map: m_productRatioBySpeciesName
      std::map<std::string,int>::iterator iterIndex=mapProductName2SolutionVector.begin();
      while(iterIndex!=mapProductName2SolutionVector.end())
      {
         m_productRatioBySpeciesName[iterIndex->first]=X(iterIndex->second);
         iterIndex++;
      }
      mapProductName2SolutionVector.clear();
   }
}
void Reaction::ComputeMassProductRatios()//changes the state of the mother object
{
   const double motherMolWeight = m_theMother->GetMolWeight();
   std::map<std::string,Species*>::iterator it;
   for(it=m_theProducts.begin(); it!=m_theProducts.end(); ++it)
   {
      const Species *const theProduct=it->second;
      const double productMolWeight=theProduct->GetMolWeight();
      std::map<std::string,double>::const_iterator itRatios=m_productRatioBySpeciesName.find(it->first);
      const double productRatio=itRatios->second;
      const double productMassFactor=(productRatio*productMolWeight)/motherMolWeight;
      m_theMother->UpdateMassFactorBySpeciesName(it->first, productMassFactor);
   }
}
void Reaction::AddProduct(Species *theProduct)
{
   std::map<std::string,Species*>::iterator it;
   it=m_theProducts.find(theProduct->GetName());
   if(it==m_theProducts.end()) //if not existant
   {
      m_theProducts[theProduct->GetName()]= theProduct;
   }
}
void Reaction::OutputProductsOnScreen()
{
   cout<<m_theMother->GetName();
   std::map<std::string,Species*>::iterator it=m_theProducts.begin();
   while(it!=m_theProducts.end())
   {
      cout<<","<<it->second->GetName();
      it++;
   }
   cout<<endl;
}
void Reaction::OutputProductRatiosOnScreen()
{
   cout<<m_theMother->GetName();
   cout<<","<<1;
   std::map<std::string,double>::iterator it=m_productRatioBySpeciesName.begin();
   while(it!=m_productRatioBySpeciesName.end())
   {
      cout<<","<<it->second;
      it++;
   }
   cout<<endl;
}
void Reaction::OutputProductsOnFile(ofstream &outfile)
{
   outfile<<m_theMother->GetName();
   std::map<std::string,Species*>::iterator it=m_theProducts.begin();
   while(it!=m_theProducts.end())
   {
      outfile<<","<<it->second->GetName();
      it++;
   }
   outfile<<endl;
}
void Reaction::OutputProductRatiosOnFile(ofstream &outfile)
{
   outfile<<m_theMother->GetName();
   outfile<<","<<1;
   std::map<std::string,double>::iterator it=m_productRatioBySpeciesName.begin();
   while(it!=m_productRatioBySpeciesName.end())
   {
      outfile<<","<<it->second;
      it++;
   }
   outfile<<endl;
}
void Reaction::OutputReactionRatiosOnScreen()
{
   std::vector<ReactionRatio*>::iterator itR=m_theReactionRatios.begin();
   while(itR!=m_theReactionRatios.end())
   {
      cout<<m_theMother->GetName()<<",";
      (*itR)->OutputOnScreen();
      itR++;
   }  
}
void Reaction::OutputReactionRatiosOnFile(ofstream &outfile)
{
   std::vector<ReactionRatio*>::iterator itR=m_theReactionRatios.begin();
   while(itR!=m_theReactionRatios.end())
   {
      outfile<<m_theMother->GetName()<<",";
      (*itR)->OutputOnFile(outfile);
      itR++;
   }
}

Species *Reaction:: GetProductById(const int &in_id)
{
   Species *ret = 0;
   std::map<int,Species*>::iterator it=m_mapId2Product.find(in_id);
   if(it!=m_mapId2Product.end())
   {
      ret = (it->second);  
   }
   return ret;
}

}
