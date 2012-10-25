#include "ChemicalModel.h"

#include <string>
#include <vector>
#include <algorithm>
#include "ReactionRatio.h"
#include "SimulatorState.h"
#include "SpeciesProperties.h"
#include "Element.h"
#include "Species.h"
#include "Reaction.h"

#include "Constants.h"
#include <math.h>
#include "Utilities.h"
#include "ComponentManager.h"
#include "GenexResultManager.h"
#include "GeneralParametersHandler.h"
#include "SpeciesState.h"
namespace OTGC
{
std::map<int,std::string> ChemicalModel::s_mapSpeciesId2Name;     //map used only in the testing version to cope with original sch conventions
int ChemicalModel::s_numberOfElements = 0;   //no need for this in the production code,only for the C++ prototype
int ChemicalModel::s_numberOfSpecies = 0;    //no need for this in the production code,only for the C++ prototype
int ChemicalModel::s_numberOfReactions = 0;  //no need for this in the production code,only for the C++ prototype


int ChemicalModel::GetNumberOfElements()
{
   return s_numberOfElements;
}
int ChemicalModel::GetNumberOfSpecies()
{
   return s_numberOfSpecies;
}
int ChemicalModel::GetNumberOfReactions()
{
   return s_numberOfReactions;
}
ChemicalModel::ChemicalModel()
{
  
}
ChemicalModel::ChemicalModel(const std::string &in_FulPathConfigurationFileName)
{
   #ifdef sun
   static char buf[1<<14];
   #endif
   ifstream theReactFile;

   theReactFile.open(in_FulPathConfigurationFileName.c_str());
   cout<<"Attempting to open "<<in_FulPathConfigurationFileName<<endl;
 
   if(!theReactFile)
   {
      cout<<"No file available!!!"<<endl;
   }
   std::string line;
   //temp values just for the C++ prototype
   //not needed for the new file structure
   //------------------------------------------------------------------------------------------------
   for(int i=0;i<2;i++)
   {
      //std::getline(theReactFile,line,'\n');
      #ifdef sun
      theReactFile.getline (buf, 1<<14);
      line = buf;
      #else
	 std::getline (theReactFile, line, '\n');
      #endif
      cout<<line<<endl;
      if(i==0)
      {
         m_name = line;
      }
   }
   std::vector<std::string> theTokens;
   std::string delim(",\n0");
   std::string::size_type startPos=0;
   std::string::size_type endPos=0;

   while(endPos!=std::string::npos)
   {
      endPos = line.find_first_of(delim,startPos);
      std::string::size_type increment=endPos-startPos;
      std::string tempString = line.substr(startPos,increment);
      theTokens.push_back(tempString);
      startPos += increment+1;
   }
   s_numberOfElements=atoi(theTokens[1].c_str());
   s_numberOfSpecies=atoi(theTokens[0].c_str());

   theTokens.clear();

   //DEBUG MAD   
   cout<< "Number of Elements "<<s_numberOfElements <<endl;
   //DEBUG MAD  
   cout<< "Number of Species  "<<s_numberOfSpecies  <<endl;
//------------------------------------------------------------------------------------------------
   SetTheElements(theReactFile);
   SetTheSpecies(theReactFile);
//------------------------------------------------------------------------------------------------
   //std::getline(theReactFile,line,'\n');
   #ifdef sun
   theReactFile.getline (buf, 1<<14);
   line = buf;
   #else
      std::getline (theReactFile, line, '\n');
   #endif
   if(!line.empty())
   {
      s_numberOfReactions=atoi(line.c_str());
   }
   //DEBUG  MAD 
   cout<<"Number of Reactions "<< s_numberOfReactions<<endl;
   SetTheReactions(theReactFile);

   //DEBUG std::cout<<"Operation finished"<<std::endl;
   theReactFile.close();
}
void ChemicalModel::SetTheElements(ifstream &theStream)
{
   std::string line;
   //std::getline(theStream,line,'\n');
   #ifdef sun
      static char buf[1<<14];
      theStream.getline (buf, 1<<14);
      line = buf;
   #else
      std::getline (theStream, line, '\n');
   #endif
   cout<<line<<endl;

   std::vector<std::string> theTokens;
   std::string delim(",");
   std::string::size_type startPos=0;
   std::string::size_type endPos=0;

   while(endPos!=std::string::npos)
   {
      endPos=line.find_first_of(delim,startPos);
      std::string::size_type increment=endPos-startPos;
      std::string tempString=line.substr(startPos,increment);
      theTokens.push_back(tempString);
      startPos+=increment+1;
   }
   for(int i = 0;i != s_numberOfElements;i++)
   {
      switch(i)
      {
         case 0:
	 {
            std::string name("C");
            OTGC::Element *theEl=new OTGC::Element(name);
            theEl->SetAtomWeight(atof(theTokens[i].c_str()));
            AddElement(theEl);
	 }
         break;
         case 1:
         {
            std::string name("H");
            OTGC::Element *theEl=new OTGC::Element(name);
            theEl->SetAtomWeight(atof(theTokens[i].c_str()));
            AddElement(theEl);
         }
         break;
         case 2:
         {
            std::string name("O");
            OTGC::Element *theEl=new OTGC::Element(name);
            theEl->SetAtomWeight(atof(theTokens[i].c_str()));
            AddElement(theEl);
         }
         break;
         default:
	 {
            std::string name("N");
            OTGC::Element *theEl=new OTGC::Element(name);
            theEl->SetAtomWeight(atof(theTokens[i].c_str()));
            AddElement(theEl);
	 }
         break;
       }
  }
  theTokens.clear();
}
//should not be used in the product code
//only in the prototype for direct comparison with VBA prototype
void ChemicalModel::SetTheSpecies(ifstream &theStream)
{
   for(int SpeciesId = 1;SpeciesId <= s_numberOfSpecies; SpeciesId++)
   {
      std::string line;
      //std::getline(theStream,line,'\n');
      #ifdef sun
	 static char buf[1<<14];
	 theStream.getline (buf, 1<<14);
	 line = buf;
      #else
	 std::getline (theStream, line, '\n');
      #endif
      cout<<line<<endl;
      std::vector<std::string> theTokens;
      std::string delim(",\n");
      std::string::size_type startPos=0;
      std::string::size_type endPos=0;

      //DEBUG MAD
      
      while(endPos!=std::string::npos)
      {
         endPos=line.find_first_of(delim,startPos);
         std::string::size_type increment=endPos-startPos;
         std::string tempString=line.substr(startPos,increment);
         theTokens.push_back(tempString);
         startPos+=increment+1;
      }
      //species name
      theTokens[0].erase(0,1);
      std::string speciesName=theTokens[0].erase(theTokens[0].size()-1,1);

      //SpeciesProperties
      double activationEnergy1  = OTGC::Constants::convert2Joule*atof(theTokens[6].c_str());
      double activationEnergy2  = OTGC::Constants::convert2Joule*atof(theTokens[7].c_str());
      double entropy            = atof(theTokens[8].c_str());
      double volume             = atof(theTokens[9].c_str());
      double reactionOrder      = atof(theTokens[10].c_str());
      double diffusionEnergy1   = OTGC::Constants::convert2Joule*atof(theTokens[11].c_str());
      double diffusionEnergy2   = 0.0;                         //generally not defined in sch files, implicitly changed at later stage
      double jumpLength         = atof(theTokens[15].c_str());
      double density            = atof(theTokens[16].c_str());

      Species *theSpec=0;
      SpeciesProperties *theProp = 0;

      theSpec=new Species(speciesName,SpeciesId,this);
      theProp=new SpeciesProperties(theSpec,
                                    activationEnergy1,
                                    activationEnergy2,
                                    entropy,
                                    volume,
                                    reactionOrder,
                                    diffusionEnergy1,
                                    diffusionEnergy2,
                                    jumpLength,
                                    density,
                                    0.0,//B0
                                    0.0//aromaticity
                                   );
      theSpec->SetProperties(theProp);


      //element composition factor, this has to change in the product implementation
      //so that arbitrary elements per species can be supported
      //use a "composition code" forward declaration in the cfg file instead
      for(int j=2;j!=6;j++)
      {
         if(theTokens[j].size())
         {
            Element *elem2Add=0;
            std::string elementName;
            switch(j)
            {
              case 2:
              {
                  double carbonFactor = atof(theTokens[j].c_str());
                  if(fabs(carbonFactor-0.0001)>0.001)
	          {
                     elementName="C";
                  }
                  else
		  {
                     elementName="NO ELEMENT";
		  }
                  elem2Add=this->GetElementByName(elementName);
                  if(elem2Add)
		  {
                     //theSpec->AddElement(elem2Add);
                     //theSpec->UpdateCompositionByElement(elementName,carbonFactor);
                     theSpec->UpdateCompositionByElement(elem2Add,carbonFactor);
		   }
		}
               break;
               case 3:
               {
                  double hydrogenFactor = atof(theTokens[j].c_str());
                  if(fabs(hydrogenFactor-0.0001)>0.001)
                  {
                     elementName="H";
                  }
                  else
                  {
                     elementName="NO ELEMENT";
                  }
                  elem2Add=this->GetElementByName(elementName);
                  if(elem2Add)
                  {
                     //theSpec->AddElement(elem2Add);
                     //theSpec->UpdateCompositionByElement(elementName,hydrogenFactor);
                     theSpec->UpdateCompositionByElement(elem2Add,hydrogenFactor);
                  }
               }
               break;
               case 4:
               {
                  double oxygenFactor = atof(theTokens[j].c_str());
                  if(fabs(oxygenFactor-0.0001)>0.001)
                  {
                     elementName="O";
                  }
                  else
                  {
                     elementName="NO ELEMENT";
                  }
                  elem2Add=this->GetElementByName(elementName);
                  if(elem2Add)
                  {
                     // theSpec->AddElement(elem2Add);
                     // theSpec->UpdateCompositionByElement(elementName,oxygenFactor);
                     theSpec->UpdateCompositionByElement(elem2Add,oxygenFactor);
                  }
               }
               break;
               default:
               {
                  double nitrogenFactor = atof(theTokens[j].c_str());
                  if(fabs(nitrogenFactor-0.0001)>0.001)
                  {
                     elementName="N";
                  }
                  else
                  {
                     elementName="NO ELEMENT";
                  }
                  elem2Add=this->GetElementByName(elementName);
                  if(elem2Add)
                  {
                     //theSpec->AddElement(elem2Add);
                     //theSpec->UpdateCompositionByElement(elementName,nitrogenFactor);
                     theSpec->UpdateCompositionByElement(elem2Add,nitrogenFactor);
                  }
               }
               break;
            }
         }
      }
      this->AddSpecies(theSpec);
      //update the map used for retrieving the VBA prototype codes
      //won't be used in the production code
      //s_mapSpeciesId2Name[SpeciesId]=theSpec->GetName();
      AddPairInSpeciesId2Name(SpeciesId,theSpec->GetName());
      theTokens.clear();
  }//end of for(int i=0;i<speciesNr;i++)
  //DEBUG std::cout<<"finished with the species"<<std::endl;
}
void ChemicalModel::SetTheReactions(ifstream &theStream)
{
   std::string line;
   std::string delim(",\n");
   //std::string::size_type startPos=0;
   //std::string::size_type endPos=0;
   //std::string::size_type increment=0;
   std::string tempString;
    std::vector<std::string> Tokens;  //buffer containing all the reactants
   std::vector<std::string> tempReactants;  //buffer containing all the reactants
   std::vector<std::string> tempRatios;     //buffer containing all the reaction ratios

   for(int i = 1;i <= s_numberOfReactions;i++)
   {

      //std::getline(theStream,line,'\n');
      #ifdef sun
	 static char buf[1<<14];
	 theStream.getline (buf, 1<<14);
	 line = buf;
      #else
	 std::getline (theStream, line, '\n');
      #endif
      //DEBUG std::cout<<"---------------Start of Reaction----------Number:"<<i<<std::endl;
      //DEBUG MAD
      cout<<line<<endl;
  
      ParseLine(line, delim, Tokens);

      Tokens.pop_back();
      std::vector<std::string>::iterator endOfReactants = std::find(Tokens.begin(), Tokens.end(), "0");

      for(std::vector<std::string>::iterator reactantsIt = Tokens.begin(); reactantsIt != endOfReactants ; ++reactantsIt) 
      {
         cout<<(*reactantsIt)<<endl;
         tempReactants.push_back((*reactantsIt));
      }
     
      if(endOfReactants != Tokens.end())
      {    
         cout<<"No end"<<endl;  

	 for(std::vector<std::string>::iterator ratiosIt = endOfReactants + 1; ratiosIt != Tokens.end(); ++ratiosIt)
	 {  
            cout<<(*ratiosIt)<<endl;
	    tempRatios.push_back(*ratiosIt);
	 }
      }
      if(tempRatios.size()%3!=0)
      {
        //MAD DEBUG
        cout<<"Problem with Reaction Ratios"<<endl;
        //must throw an exception here!
      }
      //-------------------------Set up the Reaction-----------------------------------------------
      Reaction  *currentReaction=0;
      std::vector<std::string>::size_type index=0;

      if(!tempReactants.empty())
      {
         //last one is always the mother in VBA prototype cfg file
         //Species   *theMother=GetSpeciesById(atoi(tempReactants[tempReactants.size()-1].c_str()));
         
         const std::string MotherName=GetSpeciesNameById(atoi(tempReactants[tempReactants.size()-1].c_str()));

         //DEBUG MAD
         cout<<"Mother Name and id: "<<MotherName<<" "<<atoi(tempReactants[tempReactants.size()-1].c_str())<<endl;
         
         Species *theMother=GetSpeciesByName(MotherName);
         if(theMother)
         {
            currentReaction = new Reaction(theMother);
         }
         for(index = 0; index < tempReactants.size() - 1; index++)
         {
            //Species *theProduct=GetSpeciesById(atoi(tempReactants[index].c_str()));
            const std::string productName=GetSpeciesNameById(atoi(tempReactants[index].c_str()));
            
            Species *theProduct=GetSpeciesByName(productName);
            if(theProduct)
            {
               currentReaction->AddProduct(theProduct);
               currentReaction->SetProductId(index+1,theProduct); //
               cout<<"Product name and Reaction id: "<<productName<<" "<<index+1<<endl;
            }
         }
      }
      //-------------------------Set up the Reaction Ratios----------------------------------------
      if(!tempRatios.empty())
      {
         cout<<"Size of ratios"<<tempRatios.size()<<endl;
         //for(index = 0; index < tempRatios.size();index++)
         index = 0;
         while(index < tempRatios.size() )
         {
            Species *const reactant1 = currentReaction->GetProductById(atoi(tempRatios[index++].c_str()));
            Species *const reactant2 = currentReaction->GetProductById(atoi(tempRatios[index++].c_str()));
            const std::string functionCode = tempRatios[index++];
            cout<<reactant1->GetName()<<" "<<reactant2->GetName()<<" "<<functionCode<<endl;
            ReactionRatio *theRatio=new ReactionRatio(reactant1,reactant2,functionCode);
            if(theRatio)
            {
               currentReaction->AddReactionRatio(theRatio);
            }
         }
      }
      //startPos=endPos=increment=0;
      Tokens.clear();
      tempRatios.clear();
      tempReactants.clear();
      tempString.erase();
      line.erase();
      this->AddReaction(currentReaction);
   }//end of reaction loop
}
void ChemicalModel::clearElements()
{
   std::map<std::string,Element*>::iterator itEnd = m_theElements.end();

   for(std::map<std::string,Element*>::iterator it = m_theElements.begin(); it != itEnd; ++it)
   {
      delete (it->second);
   } 

   m_theElements.clear();
}
void ChemicalModel::clearSpecies()
{
   std::vector<Species*>::iterator itEnd = m_theSpecies.end();

   for(std::vector<Species*>::iterator it = m_theSpecies.begin(); it != itEnd; ++it) 
   {
      delete (*it);
   } 

   m_theSpecies.clear();
}
void ChemicalModel::clearReactions()
{
   std::map<std::string,Reaction*>::iterator itEnd = m_theReactions.end();

   for(std::map<std::string,Reaction*>::iterator it = m_theReactions.begin(); it != itEnd; ++it)     
   {
      delete (it->second);
   } 

   m_theReactions.clear();
}
ChemicalModel::~ChemicalModel()
{
   clearElements();
   clearSpecies();
   clearReactions();
}
void ChemicalModel::AddPairInSpeciesId2Name(const int &id, const std::string &name)//only for printing out results according to the VBA format
{
  s_mapSpeciesId2Name[id] = name;
}
std::string ChemicalModel::GetSpeciesNameById(const int &id) //only for printing out results according to the VBA format
{
   std::string ret;
   std::map<int, std::string>::const_iterator itId = s_mapSpeciesId2Name.find(id);
   if(itId!=s_mapSpeciesId2Name.end())
   {
      ret = itId->second;
   }
   return ret;
}
void ChemicalModel::AddElement(Element *theElement)
{
   std::map<std::string,Element*>::iterator it;
   it = m_theElements.find(theElement->GetName());
   if(it == m_theElements.end()) //if not existant
   {
      m_theElements[theElement->GetName()] = theElement;
   }
   else
   {
      //throw a warning in a log file
      //delete theElement;
      //theElement=0;
   }
}
Element *ChemicalModel::GetElementByName(const std::string &in_name) const
{
   Element *functionReturn = 0;
   std::map<std::string,Element*>::const_iterator it = m_theElements.find(in_name);
   if(it != m_theElements.end())
   {
      functionReturn = it->second;
   }
   return functionReturn;
}
Species *ChemicalModel::GetSpeciesByName(const std::string &in_name) const
{
   Species *functionReturn = 0;
   std::map<std::string,std::vector<Species*>::size_type >::const_iterator it = m_mapSpeciesName2Index.find(in_name);
   if(it != m_mapSpeciesName2Index.end())
   {
      functionReturn = m_theSpecies[it->second];
   }
   return functionReturn;
}
Species *ChemicalModel::GetSpeciesById(const int &in_Id) const
{
   Species *functionReturn = 0;
   std::map<int, std::vector<Species*>::size_type > ::const_iterator it = m_mapSpeciesId2Index.find(in_Id);
   if(it!=m_mapSpeciesId2Index.end())
   {
      functionReturn = m_theSpecies[it->second];
   }
   return functionReturn;
}
Reaction *ChemicalModel::GetReactionByMotherName (const std::string &theMotherName) const
{
   Reaction *functionReturn = 0;
   std::map<std::string,Reaction*>::const_iterator it = m_theReactions.find(theMotherName);
   if(it!=m_theReactions.end())
   {
      functionReturn = it->second;
   }
   return functionReturn;
}
void ChemicalModel::AddSpecies(Species *theSpecies)
{
   std::vector<Species*>::size_type index = m_theSpecies.size();

   m_mapSpeciesName2Index[theSpecies->GetName()] = index;               
   m_mapSpeciesId2Index[theSpecies->GetId()]     = index;

   AddPairInSpeciesId2Name(theSpecies->GetId(), theSpecies->GetName());

   m_theSpecies.push_back(theSpecies);   
}
//OTGC2
void ChemicalModel::ComputeSpeciesUltimateMasses(SimulatorState &theState, const std::map<std::string,double>  & initialSpeciesConcentrations)
{
   for(std::vector<Species*>::iterator it = m_theSpecies.begin(), itEnd = m_theSpecies.end(); it != itEnd; ++it)
	{
      Species *currentSpecies = (*it);

      const std::string & speciesName = currentSpecies->GetName();

      std::map<std::string,double>::const_iterator itInitialConcIt;
      double initConc = (itInitialConcIt = initialSpeciesConcentrations.find(speciesName) ) == initialSpeciesConcentrations.end() ? 0.0 : itInitialConcIt->second;

      m_UltimateMassesBySpeciesName[speciesName] = initConc;  
   }
	for(std::vector<Species*>::iterator it = m_theSpecies.begin(), itEnd = m_theSpecies.end(); it != itEnd; ++it)
	{ 
      Species *currentSpecies = (*it);
   	
      double currentUltimateMass = m_UltimateMassesBySpeciesName[currentSpecies->GetName()];

   	if(currentSpecies->GetSpeciesProperties()->IsReactive())
   	{
         const std::map<std::string, double> & SpeciesProducts = currentSpecies->GetProductMassFactors();

         for(std::map<std::string, double>::const_iterator itIn = SpeciesProducts.begin(), itInEnd = SpeciesProducts.end(); itIn != itInEnd; ++itIn)
         {
            double previousMass = m_UltimateMassesBySpeciesName[itIn->first];
            double deltaMass = itIn->second * currentUltimateMass;
            m_UltimateMassesBySpeciesName[itIn->first] = previousMass + deltaMass;          
         }	
   	}
	}
}
//OTGC3
void ChemicalModel::ComputeSpeciesUltimateMasses(SimulatorState &theState)
{
   std::map<std::string, double> tempUltimateMassesBySpeciesName;
   for(std::vector<Species*>::iterator it = m_theSpecies.begin(), itEnd = m_theSpecies.end(); it != itEnd; ++it)
	{
      Species *currentSpecies = (*it);

      const std::string & speciesName = currentSpecies->GetName();

      tempUltimateMassesBySpeciesName[speciesName] = theState.GetSpeciesConcentrationByName(speciesName);  
   }
	for(std::vector<Species*>::iterator it = m_theSpecies.begin(), itEnd = m_theSpecies.end(); it != itEnd; ++it)
	{ 
      Species *currentSpecies = (*it);
   	
      double currentUltimateMass = tempUltimateMassesBySpeciesName[currentSpecies->GetName()];

   	if(currentSpecies->GetSpeciesProperties()->IsReactive())
   	{
         const std::map<std::string, double> & SpeciesProducts = currentSpecies->GetProductMassFactors();

         for(std::map<std::string, double>::const_iterator itIn = SpeciesProducts.begin(), itInEnd = SpeciesProducts.end(); itIn != itInEnd; ++itIn)
         {
            double previousMass = tempUltimateMassesBySpeciesName[itIn->first];
            double deltaMass = itIn->second * currentUltimateMass;
            tempUltimateMassesBySpeciesName[itIn->first] = previousMass + deltaMass;          
         }	
   	}
	}

   theState.setSpeciesUltimateMassesByName(tempUltimateMassesBySpeciesName);
}
//OTGC
const vector<std::string> & ChemicalModel::getSpeciesNames() const
{
   static bool firstTimeHere = true;
   static  vector<std::string> returnContainer;

   if(firstTimeHere)
   {
      typedef std::vector<Species*>::const_iterator itSp;
      for(itSp itB = m_theSpecies.begin(), itEnd = m_theSpecies.end(); itB != itEnd; ++itB)
   	{
         returnContainer.push_back((*itB)->GetName());
   	}
      firstTimeHere = false;
   }
   return returnContainer;
}
//OTGC
void ChemicalModel::AddReaction(Reaction *theReaction)
{
   Species *mother = theReaction->GetMother();
   m_theReactions[mother->GetName()] = theReaction;
}
void ChemicalModel::UpdateSpeciesCompositionsByElementName(const std::string &in_Species,const std::string &in_Element, const double &in_CompositionFactor)  
{
   Element *theElem = 0;
   theElem = GetElementByName(in_Element);
   Species *curSpecies = 0;
   curSpecies = GetSpeciesByName(in_Species);

   if(theElem && curSpecies)
   {
      curSpecies->UpdateCompositionByElement(theElem,in_CompositionFactor);
   }
}
void ChemicalModel::CompEarlySpecies() 
{
   Element *const HYDROGEN = GetElementByName("H");
   Element *const OXYGEN   = GetElementByName("O");
   Element *const NITROGEN = GetElementByName("N");

   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();

   Species *curSpecies=0;

   //Kerogen initial start for getting value for AtomN
   //KerogenAtomN-->PreashalteneAtomN-->KerogenAtomO-->KerogenAtomN ...
   //preasphaltene O/C ratio of  as function of its H/C ratio

   curSpecies = GetSpeciesByName("preasphalt");
   double PreashalteneAtomH = 0.0;
   double PreashalteneAtomO = 0.0;
   double PreashalteneAtomN = 0.0;

   //HC value for preasphaltene were updated with GUI value in Stoichiometry::CheckInitialHCindex()
   if(curSpecies)
   {
      //Atom(IatomO, Lpreasphalt) = OCpreasphalt1 * Atom(IatomH, Lpreasphalt) ^ 2 +
      //OCpreasphalt2 * Atom(IatomH, Lpreasphalt) + OCpreasphalt3

      PreashalteneAtomH = curSpecies->GetCompositionByElement("H");
      PreashalteneAtomO = theHandler.GetParameterById(GeneralParametersHandler::OCpreasphalt1) * pow(PreashalteneAtomH, 2.0) +
                          theHandler.GetParameterById(GeneralParametersHandler::OCpreasphalt2) * PreashalteneAtomH +
                          theHandler.GetParameterById(GeneralParametersHandler::OCpreasphalt3);

      curSpecies->UpdateCompositionByElement(OXYGEN,PreashalteneAtomO);
   }
   //KEROGEN H/C & O/C as functions of those of preasphaltene
   //-------------Atom(IatomH, Lkerogen) = HCkerogen1 * Atom(IatomH, Lpreasphalt) + HCkerogen2
   double KerogenAtomH = theHandler.GetParameterById(GeneralParametersHandler::HCkerogen1) * PreashalteneAtomH + 
                         theHandler.GetParameterById(GeneralParametersHandler::HCkerogen2);

   //-------------Atom(IatomO, Lkerogen) = OCkerogen1 * Atom(IatomO, Lpreasphalt) + OCkerogen2
   double KerogenAtomO = theHandler.GetParameterById(GeneralParametersHandler::OCkerogen1) * PreashalteneAtomO + 
                         theHandler.GetParameterById(GeneralParametersHandler::OCkerogen2);

   //-----------------------Atom(IatomN, Lkerogen) = Atom(IatomO, Lkerogen) * Nkerogen
   double KerogenAtomN = KerogenAtomO *theHandler.GetParameterById(GeneralParametersHandler::Nkerogen);

   //-------------get Kerogen species and update
   curSpecies = GetSpeciesByName("kerogen");
   if(curSpecies)
   {
      curSpecies->UpdateCompositionByElement(HYDROGEN,KerogenAtomH);
      curSpecies->UpdateCompositionByElement(OXYGEN,KerogenAtomO);
      curSpecies->UpdateCompositionByElement(NITROGEN,KerogenAtomN);
   }
   //And now update the PreasphalteneAtomN
   //Atom(IatomN, Lpreasphalt) = Atom(IatomN, Lkerogen) * Npreasphalt
   curSpecies = GetSpeciesByName("preasphalt");
   if(curSpecies)
   {
      PreashalteneAtomN = KerogenAtomN * theHandler.GetParameterById(GeneralParametersHandler::Npreasphalt);
      curSpecies->UpdateCompositionByElement(NITROGEN,PreashalteneAtomN);
   }

   //ASPHALTENE H/C and O/C
   //-----------------------Call AsphalComp

   //------------Atom(IatomH, Lasphaltene) = HCAsphOverPreasphalt * Atom(IatomH, Lpreasphalt)
   double AsphalteneAtomH = theHandler.GetParameterById(GeneralParametersHandler::HCAsphOverPreasphalt) * PreashalteneAtomH;
   //------------Atom(IatomO, Lasphaltene) = OCAsphOverPreasphalt * Atom(IatomO, Lpreasphalt)
   double AsphalteneAtomO = theHandler.GetParameterById(GeneralParametersHandler::OCAsphOverPreasphalt) * PreashalteneAtomO;
   //------------Atom(IatomN, Lasphaltene) = Atom(IatomN, Lpreasphalt) * Nasphaltene
   double  AsphalteneAtomN = PreashalteneAtomN * theHandler.GetParameterById(GeneralParametersHandler::Nasphaltene);

   if(AsphalteneAtomO < theHandler.GetParameterById(GeneralParametersHandler::OCasphMin) )
   {
      AsphalteneAtomO = theHandler.GetParameterById(GeneralParametersHandler::OCasphMin);
   }
   //----------------get asphaltene species and update
   curSpecies = GetSpeciesByName("asphaltenes");
   if(curSpecies)
   {
      curSpecies->UpdateCompositionByElement(HYDROGEN,AsphalteneAtomH);
      curSpecies->UpdateCompositionByElement(OXYGEN,AsphalteneAtomO);
      curSpecies->UpdateCompositionByElement(NITROGEN,AsphalteneAtomN);
   }
   //RESIN H/C AND O/C make identical to those of asphaltene
   curSpecies = GetSpeciesByName("resins");
   double   ResinAtomH = AsphalteneAtomH; //for clarity
   double   ResinAtomO = AsphalteneAtomO; //for clarity

   //---------------Atom(IatomN, Lresin) = Atom(IatomN, Lasphaltene) * Nresin
   double  ResinAtomN = AsphalteneAtomN * theHandler.GetParameterById(GeneralParametersHandler::Nresin);
   if(curSpecies)
   {
      curSpecies->UpdateCompositionByElement(HYDROGEN,ResinAtomH);
      curSpecies->UpdateCompositionByElement(OXYGEN,ResinAtomO);
      curSpecies->UpdateCompositionByElement(NITROGEN,ResinAtomN);
   }
   //PRECOKE & COKE1 O/C as a function of that of preasphaltene
   //--------------------Call CokeComp
   //---------------precoke and coke1 oxygen contents as a function of Preasphalt composition
   //---------------Atom(IatomO, Lprecoke) = OCprecokeWhenHCpreasphaltZero - OCprecokePerPreasphalt *Atom(IatomH, Lpreasphalt)

   double PrecokeAtomO = theHandler.GetParameterById(GeneralParametersHandler::OCprecokeWhenHCpreasphaltZero) - 
                         theHandler.GetParameterById(GeneralParametersHandler::OCprecokePerPreasphalt) * PreashalteneAtomH;
   //---------------------Atom(IatomN, Lprecoke) = Atom(IatomN, Lresin) * Nprecoke
   double PrecokeAtomN = ResinAtomN *  theHandler.GetParameterById(GeneralParametersHandler::Nprecoke);
   curSpecies= GetSpeciesByName("precoke");
   if(curSpecies)
   {
      curSpecies->UpdateCompositionByElement(OXYGEN,PrecokeAtomO);
      curSpecies->UpdateCompositionByElement(NITROGEN,PrecokeAtomN);
   }
   //----------make O/C of Coke1 the same as Precoke
   double Coke1AtomO = PrecokeAtomO;//for clarity
   curSpecies = GetSpeciesByName("coke1");
   if(curSpecies)
   {
      curSpecies->UpdateCompositionByElement(OXYGEN,Coke1AtomO);
   }
   //Hetero1
   //--------------------Atom(IatomN, Lhetero1) = Atom(IatomN, Lprecoke) * Nhetero1
   double Hetero1AtomN =  PrecokeAtomN * theHandler.GetParameterById(GeneralParametersHandler::Nhetero1);
   curSpecies = GetSpeciesByName("Hetero1");
   if(curSpecies)
   {
      curSpecies->UpdateCompositionByElement(NITROGEN,Hetero1AtomN);
   }

   //Hetero2
   //--------------------Atom(IatomN, Lhetero2) = Nhetero2
   double Hetero2AtomN = theHandler.GetParameterById(GeneralParametersHandler::Nhetero2);
   curSpecies = GetSpeciesByName("Hetero2");
   if(curSpecies)
   {
      curSpecies->UpdateCompositionByElement(NITROGEN,Hetero2AtomN);
   }
   //C15+ Aro
   //Atom(IatomN, Lc15plusAro) = NC15plusAro
   double c15plusAroAtomN =  theHandler.GetParameterById(GeneralParametersHandler::NC15plusAro);
   curSpecies = GetSpeciesByName("C15+Aro");
   if(curSpecies)
   {
      curSpecies->UpdateCompositionByElement(NITROGEN,c15plusAroAtomN);
   }
}
//OTGC
void ChemicalModel::SetSpeciesReactionOrder()
{
   //OTGC
   for(std::vector<Species*>::iterator itB = m_theSpecies.begin(), itEnd = m_theSpecies.end();
       itB != itEnd; ++itB)
   {
		SpeciesProperties *curSpeciesProp = (*itB)->GetSpeciesProperties();

		if(curSpeciesProp->IsReactive())
		{
   		curSpeciesProp->SetReactionOrder(1.5);
		}
		else
		{
		   curSpeciesProp->SetReactionOrder(1.0);
		}
   }
   GetSpeciesByName("C2")->GetSpeciesProperties()->SetReactionOrder(2.0);
   //OTGC


   //OTGC - DeltaGENEX  
/*
   for(std::vector<Species*>::iterator itB = m_theSpecies.begin(), itEnd = m_theSpecies.end();
       itB != itEnd; ++itB)
   {
     Species *curSpec = (*itB);
     if(curSpec->GeName() != "kerogen" && 
        curSpec->GeName() != "C15+Aro" &&
        curSpec->GeName() != "C15+Sat" &&
        curSpec->GeName() != "C6-14Aro" &&
        curSpec->GeName() != "C6-14Sat" &&
        curSpec->GeName() != "C5" &&
        curSpec->GeName() != "C4" &&
        curSpec->GeName() != "C3" &&
        )
     (*itB)->GetSpeciesProperties()->SetReactionOrder(1.0)
   }
*/
    //OTGC
}

//value of Emean in Joule
void ChemicalModel::KineticsEarlySpecies(const double &Emean)
{
   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();

   Species *curSpecies=0;
   //preasphaltene cracking activation energies as function of input mean activation energy
   //-----------------------Emean = Range("Input!Emean").Value
   //-----------------------Easph1 = 2 * Emean - EcrackingCC   
   
   double Easph1 = 2.0 * Emean - theHandler.GetParameterById(GeneralParametersHandler::EcrackingCC);

   //--------reaction orders of kerogen & preasphaltene decomposition as function of their H/C ratios
   //Kerogen
   curSpecies = GetSpeciesByName("kerogen");
   if(curSpecies)
   {
      SpeciesProperties *const KerogenProps = curSpecies->GetSpeciesProperties();
      //Order(Lkerogen) = 1.5 'or use FunOrder2(Atom(IatomH, Lpreasphalt)); 1.5 probably better
      //OTGC - order is not treated here for OTGC, it is treated in ChemicalModel::SetSpeciesReactionOrder
      //double KerogenOrder = 1.5;
      //KerogenProps->SetReactionOrder(KerogenOrder);
      //OTGC
   }
   //Preasphaltene
   curSpecies = GetSpeciesByName("preasphalt");
   if(curSpecies)
   {
      SpeciesProperties *const PreasphalteneProps = curSpecies->GetSpeciesProperties();
      //-------------Act(1, Lpreasphalt) = Easph1 - Ediff1 - EdropForS
      double PreasphalteneactivationEnergy1 = Easph1 - theHandler.GetParameterById(GeneralParametersHandler::Ediff1) 
                                                     - theHandler.GetParameterById(GeneralParametersHandler::EdropForS);

      PreasphalteneProps->SetActivationEnergy1(PreasphalteneactivationEnergy1);
      //-------------Act(2, Lpreasphalt) = EcrackingCC - Ediff2
      //OTGC
      //double PreasphalteneactivationEnergy2 = theHandler.GetParameterById(GeneralParametersHandler::EcrackingCC) - 
      //                                        theHandler.GetParameterById(GeneralParametersHandler::Ediff2);
      double PreasphalteneactivationEnergy2 = theHandler.GetParameterById(GeneralParametersHandler::EcrackingCC);                                        
      //OTGC
      PreasphalteneProps->SetActivationEnergy2(PreasphalteneactivationEnergy2);

       //OTGC - order is not treated here for OTGC, it is treated in ChemicalModel::SetSpeciesReactionOrder.
      //Change the order as well
      //------------Order(Lpreasphalt) = FunOrder2(Atom(IatomH, Lpreasphalt))
      //double PreasphalteneOrder = curSpecies->ComputeReactionOrder();
      //PreasphalteneProps->SetReactionOrder(PreasphalteneOrder);
      //OTGC
   }
   //Asphaltene
   curSpecies = GetSpeciesByName("asphaltenes");
   double AsphalteneactivationEnergy1 = 0.0;
   double AsphalteneactivationEnergy2 = 0.0;
   if(curSpecies)
   {
      SpeciesProperties *const AsphalteneProps = curSpecies->GetSpeciesProperties();
      //-------------Act(1, Lasphaltene) = Easph1
      AsphalteneactivationEnergy1 = Easph1;
      AsphalteneProps->SetActivationEnergy1(AsphalteneactivationEnergy1);
      //-------------Act(2, Lasphaltene) = EcrackingCC
      AsphalteneactivationEnergy2 = theHandler.GetParameterById(GeneralParametersHandler::EcrackingCC);
      AsphalteneProps->SetActivationEnergy2(AsphalteneactivationEnergy2);
   }
   //Resin
   curSpecies = GetSpeciesByName("resins");
   if(curSpecies)
   {
      SpeciesProperties *const ResinProps = curSpecies->GetSpeciesProperties();
      //-------------Act(1, Lresin) = Act(1, Lasphaltene)
      double ResinactivationEnergy1 = AsphalteneactivationEnergy1;
      ResinProps->SetActivationEnergy1(ResinactivationEnergy1);
      //-------------Act(2, Lresin) = Act(2, Lasphaltene)//n.b. can introduce a small increase in E here (e.g. 2 kJ/mol), to slightly improve Type II data fit
      double ResinactivationEnergy2 = AsphalteneactivationEnergy2;
      ResinProps->SetActivationEnergy2(ResinactivationEnergy2);
   }
}
//OTGC
//void ChemicalModel::ComputeFactors()
void ChemicalModel::ComputeStoichiometry()
{
   //need of  preasphalteneAromaticity in Reaction::ComputeProductRatios
   Species *const preasphaltene = GetSpeciesByName("preasphalt");
   double preasphalteneAromaticity = preasphaltene->ComputeAromaticity();
   this->ComputeProductRatios(preasphalteneAromaticity);
   this->ComputeMassProductRatios();
}
void ChemicalModel::ComputeProductRatios(const double &preasphalteneAromaticity)
{
   std::map<std::string,Reaction*>::iterator itR = m_theReactions.begin();
   while(itR != m_theReactions.end())
   {
      itR->second->ComputeProductRatios(preasphalteneAromaticity);
      itR++;
   }
}
void ChemicalModel::ComputeMassProductRatios()
{
   std::map<std::string,Reaction*>::iterator itR = m_theReactions.begin();
   while(itR != m_theReactions.end())
   {
      itR->second->ComputeMassProductRatios();
      itR++;
   }
}
void ChemicalModel::UpdateSpeciesProperties()
{
   std::vector<Species*>::iterator itB;
   for(itB = m_theSpecies.begin();itB != m_theSpecies.end(); ++itB)
   {
      (*itB)->UpdateProperties();
   }
}
double ChemicalModel::GetElemAtomWeightByName(const std::string &ElemName) const
{
   Element *const Elem = GetElementByName(ElemName);
   double ret=0.0;
   if(Elem)
   {
      ret = Elem->GetAtomWeight();
   }
   return ret;
}
double ChemicalModel::GetSpeciesMolWeightByName(const std::string &SpeciesName) const
{
   const Species *const theSpecies = GetSpeciesByName(SpeciesName);
   double ret = 0.0;
   if(theSpecies)
   {
      ret = theSpecies->GetMolWeight();
   }
   return ret;
}
void ChemicalModel::UpdateSpeciesPositiveGenRateByName(const std::string &SpeciesName, const double &in_GenRateToSubract)//adds to the existant
{
     Species *const theSpecies = GetSpeciesByName(SpeciesName);

     if(theSpecies)
     {
        double  positiveGenRate = theSpecies->GetPositiveGenRate();
        positiveGenRate -= in_GenRateToSubract;
        theSpecies->SetPositiveGenRate(positiveGenRate);
     }
}
double ChemicalModel::GetSpeciesDensityByName(const std::string &SpeciesName) const
{
   const Species *const theSpecies = GetSpeciesByName(SpeciesName);
   double ret = 0.0;
   if(theSpecies)
   {
      ret = theSpecies->GetDensity();
   }
   return ret; 
}

void ChemicalModel::InitializeSpeciesTimeStepVariables()
{
   //Species::SetSpeciesTimeStepVariablesToZero();

   double initialTimeStepGenRate = 0.0;
   std::vector<Species*>::iterator itS;
   for(itS = m_theSpecies.begin();itS != m_theSpecies.end(); ++itS)
   {
      (*itS)->SetPositiveGenRate(initialTimeStepGenRate);
   }
}
bool ChemicalModel::Validate() const  
{
   bool status = true;
  
   using namespace CBMGenerics;
   ComponentManager & theManager = ComponentManager::getInstance();
   int i;
   for (i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i)
   {
      Species *theSpecies = ( GetSpeciesByName(theManager.GetSpeciesName( i ) ) );  
      if(theSpecies == 0)
      {
          status = false;
          break;
      }                          
   }
   return status;
}                                 
//OTGC2
void ChemicalModel::ComputeTimeStep(SimulatorState &theSimulatorState,                                   
                                    const double &in_dT,
                                    const double &s_Peff,
                                    const double &s_TK,
                                    const double &s_FrequencyFactor,
                                    const double &s_kerogenTransformationRatio,
                                    const double &s_precokeTransformationRatio,
                                    const double &s_coke2TransformationRatio,
                                    const double &s_DiffusionConcDependence,
                                    const double &s_VogelFulcherTemperature,
                                    const bool &in_OpenSourceRockConditions )

{

   //initialisation for this time step
   //       For L = 1 To Ln
   //             RateGenPos(L) = 0!
   //            Theta(L) = 0#
   //         Next L
   //initialise chemical model, ready for computation
   InitializeSpeciesTimeStepVariables();

   for(int id = 1;id <= s_numberOfSpecies;id++)
   {
      Species* theSpecies=GetSpeciesById(id);
      theSpecies->ComputeTimeStep( theSimulatorState,                
                                    in_dT,
                                    s_Peff,
                                    s_TK,
                                    s_FrequencyFactor,
                                    s_kerogenTransformationRatio,
                                    s_precokeTransformationRatio,
                                    s_coke2TransformationRatio,
                                    s_DiffusionConcDependence,
                                    s_VogelFulcherTemperature,
                                    in_OpenSourceRockConditions);
   }
}
//OTGC
void ChemicalModel::PrintConfigurationFileEntities(ofstream &outfile)
{
   outfile<<"Table:[Elements]" <<endl;
   outfile<<"ElementName,AtomWeight" <<endl;
   std::map<std::string,Element*>::iterator itE = m_theElements.begin();
   while(itE != m_theElements.end())
   {
      itE->second->OutputOnFile(outfile);
      itE++;
   }
   outfile<<"[EndOfTable]" <<endl;
   //----------------------------------------------------------------------------
   int id;
   outfile<<endl;
   outfile<<"Table:[Species]" <<endl;
   outfile<<"SpeciesId,SpeciesName" <<endl;
   for(id = 1;id <= (int)m_theSpecies.size();id++)
   {
      Species* theSpecies = GetSpeciesById(id);
      outfile<<id<<","<<theSpecies->GetName()<<endl;
   }
   outfile<<"[EndOfTable]" <<endl;
   //-----------------------------------------------------------------------------
   outfile<<endl;
   outfile<<"Table:[SpeciesCompositionByName]" <<endl;
   outfile<<"SpeciesName,CompositionCode,CompositionFactorC, CompositionFactorH, CompositionFactorO, CompositionFactorN, CompositionFactorS" <<endl;
   for(id = 1;id <= (int)m_theSpecies.size();id++)
   {
      Species* theSpecies = GetSpeciesById(id);
      theSpecies->OutputCompositionOnFile(outfile);
   }   
   outfile<<"[EndOfTable]" <<endl;
   //-----------------------------------------------------------------------------
   outfile<<endl;
   outfile<<"Table:[SpeciesPropertiesByName]" <<endl;
   outfile<<"SpeciesName,Weight,Density,activationEnergy1,activationEnergy2,entropy,volume,reactionOrder,diffusionEnergy1,diffusionEnergy2,jumpLength, B0,aromaticity" <<endl;
   for(id = 1;id <= (int)m_theSpecies.size();id++)
   {
      Species* theSpecies = GetSpeciesById(id);
      theSpecies->OutputPropertiesOnFile(outfile);
   }   
  outfile<<"[EndOfTable]" <<endl;
   //-----------------------------------------------------------------------------
   outfile<<endl;
   outfile<<"Table:[ReactionsBySpeciesName]" <<endl;
   outfile<<"MotherName,Product1, Product2, ..." <<endl;
   std::map<std::string,Reaction*>::iterator itR=m_theReactions.begin();
   while(itR != m_theReactions.end())
   {
      itR->second->OutputProductsOnFile(outfile);
      itR++;
   }
   outfile<<"[EndOfTable]" <<endl;
   //-----------------------------------------------------------------------------
   outfile<<endl;
   outfile<<"Table:[ReactionRatiosBySpeciesName]" <<endl;
   outfile<<"MotherName,Rectant1,Reactant2,Ratio" <<endl;
   itR = m_theReactions.begin();
   while(itR != m_theReactions.end())
   {
      itR->second->OutputReactionRatiosOnFile(outfile);
      itR++;
   }
   outfile<<"[EndOfTable]" <<endl;
   outfile<<endl;
   //-----------------------------------------------------------------------------
}

void ChemicalModel::PrintBenchmarkSpeciesPropertiesHeader(ofstream &outfile) const
{
outfile<<"[Table:SpeciesPropertiesC++]"<<endl;

outfile  <<"spec. no."<<","
         <<"name"<<","
         <<"C"<<","
         <<"H"<<","
         <<"O"<<","
         <<"N"<<","
         <<"Dens"<<","
         <<"FW"<<","
         <<"Arom"<<","
         <<"Er1"<<","
         <<"Er2"<<","
         <<"n"<<","
         <<"Ed1"<<","
         <<"B0"<<","
         <<"IsMobile"<<","
         <<"IsReactive"<<","
         <<"IsHC"<<","
         <<"IsOil"<<","
         <<"IsHCgas"<<endl;
}

void ChemicalModel::PrintBenchmarkSpeciesProperties(ofstream &outfile) const
{
   PrintBenchmarkSpeciesPropertiesHeader(outfile);
   int id=0;
   for(id=1;id<=(int)m_theSpecies.size();id++)
   {
      Species* theSpecies=GetSpeciesById(id);
      theSpecies->PrintBenchmarkProperties(outfile,id);
   }
}
void ChemicalModel::PrintBenchmarkStoichiometryHeader(ofstream &outfile) const
{
   outfile<<"[Table:StoichiometryC++]"<<endl;	
  outfile<<"MotherName,";
  int id=0;
   for(id=1;id<=(int)m_theSpecies.size();id++)
   {
      Species* theSpecies=GetSpeciesById(id);
      outfile<< theSpecies->GetName()<<",";
   }
   outfile<<endl;
}
void ChemicalModel::PrintBenchmarkStoichiometry(ofstream &outfile) const
{
   PrintBenchmarkStoichiometryHeader(outfile);
   int id=0;
   for(id=1;id<=(int)m_theSpecies.size();id++)
   {
      Species* theSpecies=GetSpeciesById(id);
      theSpecies->PrintBenchmarkStoichiometry(outfile);
   }
}
void ChemicalModel::LoadElements(ifstream &ConfigurationFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim=",";

   //getline(ConfigurationFile,line,'\n');//header
   #ifdef sun
      static char buf[1<<14];
      ConfigurationFile.getline (buf, 1<<14);
      line = buf;
   #else
      std::getline (ConfigurationFile, line, '\n');
   #endif
   for(;;)
   {
        //getline(ConfigurationFile,line,'\n');
        #ifdef sun
           //static char buf[1<<14];
           ConfigurationFile.getline (buf, 1<<14);
           line = buf;
        #else
            std::getline (ConfigurationFile, line, '\n');
        #endif
        
        if(line==OTGC::CFG::EndOfTable || line.size()==0 )
        {
          break;
        }

        ParseLine(line, delim, theTokens);

        Element *theElement=new OTGC::Element(theTokens[0]);
        theElement->SetAtomWeight(atof(theTokens[1].c_str()));
        this->AddElement(theElement);

        theTokens.clear();
   }

   s_numberOfElements=(int)m_theElements.size();
}
void ChemicalModel::LoadSpecies(ifstream &ConfigurationFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim=",";

   //std::getline(ConfigurationFile,line,'\n');//header
   #ifdef sun
     static char buf[1<<14];
     ConfigurationFile.getline (buf, 1<<14);
     line = buf;
   #else
       std::getline (ConfigurationFile, line, '\n');
   #endif
   for(;;)
   {
       //std::getline(ConfigurationFile,line,'\n');
       #ifdef sun
          //static char buf[1<<14];
          ConfigurationFile.getline (buf, 1<<14);
          line = buf;
      #else
           std::getline (ConfigurationFile, line, '\n');
      #endif
       if(line==OTGC::CFG::EndOfTable || line.size()==0 )
       {
         break;
       }
   
       ParseLine(line, delim, theTokens);

       //Species(const std::string &in_Name, const int &in_id, ChemicalModel *const in_theModel);
       Species *theSpecies = new Species(theTokens[1], atoi(theTokens[0].c_str()), this);
       this->AddSpecies(theSpecies);

       //for testing the prototype has to go in the product version
       this->AddPairInSpeciesId2Name( atoi(theTokens[0].c_str()), theTokens[1]);

        theTokens.clear();
   }
   s_numberOfSpecies = (int)m_theSpecies.size();
}
void ChemicalModel::LoadSpeciesComposition(ifstream &ConfigurationFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim=",";

   //std::getline(ConfigurationFile,line,'\n');//header
   #ifdef sun
     static char buf[1<<14];
     ConfigurationFile.getline (buf, 1<<14);
     line = buf;
   #else
       std::getline (ConfigurationFile, line, '\n');
   #endif
   ParseLine(line, delim, theTokens);

   std::vector<std::string>::size_type i=0;
   std::map<int,std::string> TokenIndex2MapElementName;

   //Process header
   for(i=2;i<theTokens.size();i++)
   {
         //CompositionFactorC -->extract "C" into Element
         char elementFromHeader = theTokens[i][theTokens[i].size()-1];
         std::string Element(1,elementFromHeader);
         TokenIndex2MapElementName[i] = Element;
   }
   theTokens.clear();
   //Process main body
   for(;;)
   {
       //std::getline(ConfigurationFile,line,'\n');
       #ifdef sun
          //static char buf[1<<14];
          ConfigurationFile.getline (buf, 1<<14);
          line = buf;
       #else
           std::getline (ConfigurationFile, line, '\n');
       #endif
       if(line==OTGC::CFG::EndOfTable || line.size()==0 )
       {
         break;
       }

       ParseLine(line, delim, theTokens);

       Species *theSpecies=this->GetSpeciesByName(theTokens[0]);
       i = 0;
       for(i = 2;i < theTokens.size(); i++)
       {
             double compositionCode=atof(theTokens[i].c_str());
             if(compositionCode > OTGC::Constants::ZERO)
             {
               std::map<int,std::string>::const_iterator it = TokenIndex2MapElementName.find(i);
               if( it != TokenIndex2MapElementName.end())
               {
                  Element *const theElem = this->GetElementByName(it->second);
                  theSpecies->UpdateCompositionByElement(theElem,compositionCode);
               } 
             }
       }
       theTokens.clear();
   }
   TokenIndex2MapElementName.clear();
}
void ChemicalModel::LoadSpeciesProperties(ifstream &ConfigurationFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim=",";

   //std::getline(ConfigurationFile,line,'\n');//header
   #ifdef sun
     static char buf[1<<14];
     ConfigurationFile.getline (buf, 1<<14);
     line = buf;
   #else
       std::getline (ConfigurationFile, line, '\n');
   #endif
 
   for(;;)
   {
       //std::getline(ConfigurationFile,line,'\n');
       #ifdef sun
         //static char buf[1<<14];
         ConfigurationFile.getline (buf, 1<<14);
         line = buf;
       #else
          std::getline (ConfigurationFile, line, '\n');
       #endif
       if(line == OTGC::CFG::EndOfTable || line.size() == 0 )
       {
         break;
       }

       ParseLine(line, delim, theTokens);

       //SpeciesName,Weight,Density,activationEnergy1,activationEnergy2,entropy,volume,reactionOrder,diffusionEnergy1,diffusionEnergy2,jumpLength, B0,aromaticity
      
       //SpeciesProperties
       double density            = atof(theTokens[2].c_str());
       double activationEnergy1  = atof(theTokens[3].c_str());
       double activationEnergy2  = atof(theTokens[4].c_str());
       double entropy            = atof(theTokens[5].c_str());
       double volume             = atof(theTokens[6].c_str());
       double reactionOrder      = atof(theTokens[7].c_str());     
       double diffusionEnergy1   = atof(theTokens[8].c_str());           
       double diffusionEnergy2   = atof(theTokens[9].c_str());
       double jumpLength         = atof(theTokens[10].c_str());
       double B0                 = atof(theTokens[11].c_str());
       double Aromaticity        = atof(theTokens[12].c_str());
      

      Species *theSpecies = this->GetSpeciesByName(theTokens[0]);
      SpeciesProperties *theProperties = new SpeciesProperties(theSpecies,
                                    activationEnergy1,
                                    activationEnergy2,
                                    entropy,
                                    volume,
                                    reactionOrder,
                                    diffusionEnergy1,
                                    diffusionEnergy2,
                                    jumpLength,
                                    density,
                                    B0,
                                    Aromaticity
                                   );

      theSpecies->SetProperties(theProperties);
      theTokens.clear();
   }
}
void ChemicalModel::LoadReactions(ifstream &ConfigurationFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim=",";

   //std::getline(ConfigurationFile,line,'\n');//header
   #ifdef sun
     static char buf[1<<14];
     ConfigurationFile.getline (buf, 1<<14);
     line = buf;
   #else
       std::getline (ConfigurationFile, line, '\n');
   #endif

   for(;;)
   {
       //std::getline(ConfigurationFile,line,'\n');
       #ifdef sun
          //static char buf[1<<14];
          ConfigurationFile.getline (buf, 1<<14);
          line = buf;
       #else
          std::getline (ConfigurationFile, line, '\n');
       #endif

       if(line==OTGC::CFG::EndOfTable || line.size()==0 )
       {
         break;
       }

       ParseLine(line, delim, theTokens);
       Reaction  *currentReaction=0;
       std::vector<std::string>::size_type i=0;
       for(i=0;i<theTokens.size();i++)
       {
          if(i==0)
          {
             Species *theMother=GetSpeciesByName(theTokens[i]);
             if(theMother)
             {
                currentReaction=new Reaction(theMother);
             }
             else
             {
                //throw
             }
          }
          else
          {
             Species *theProduct=GetSpeciesByName(theTokens[i]);
             if(theProduct)
             {
                currentReaction->AddProduct(theProduct);
             }
             else
             {
              //throw
             }
          }
       }
       this->AddReaction(currentReaction);
       theTokens.clear();
   }
   s_numberOfReactions = (int)m_theReactions.size();
}
void ChemicalModel::LoadReactionRatios(ifstream &ConfigurationFile)
{
  std::string line;
   std::vector<std::string> theTokens;
   std::string delim=",";

   //std::getline(ConfigurationFile,line,'\n');//header
   #ifdef sun
     static char buf[1<<14];
     ConfigurationFile.getline (buf, 1<<14);
     line = buf;
   #else
       std::getline (ConfigurationFile, line, '\n');
   #endif

   for(;;)
   {
       //std::getline(ConfigurationFile,line,'\n');
       #ifdef sun
          static char buf[1<<14];
          ConfigurationFile.getline (buf, 1<<14);
          line = buf;
       #else
           std::getline (ConfigurationFile, line, '\n');
       #endif
       if(line==OTGC::CFG::EndOfTable || line.size()==0 )
       {
         break;
       }
       
       ParseLine(line, delim, theTokens);
       
       Reaction  *currentReaction=GetReactionByMotherName(theTokens[0]);
       if(currentReaction && theTokens.size()==4)
       {
          Species *Reactant1=GetSpeciesByName(theTokens[1]);
          Species *Reactant2=GetSpeciesByName(theTokens[2]);
          ReactionRatio *theRatio=new ReactionRatio(Reactant1,Reactant2,theTokens[3]);
          
          currentReaction->AddReactionRatio(theRatio);
       }
       else
       {
         //throw
       }
       theTokens.clear();
   } 
}
double ChemicalModel::GetSpeciesCompositionByElement(const std::string &SpeciesName, const std::string &ElementName)
{
   double ret=0.0;
   Species *theSpecies = GetSpeciesByName(SpeciesName);
   if(theSpecies)
   {
      ret = theSpecies->GetCompositionByElement(ElementName);
   }
   return ret;
}
void ChemicalModel::UpdateSpeciesDiffusionEnergy1(const std::string &SpeciesName,const double &diffEnergy)
{
   Species *theSpecies = GetSpeciesByName(SpeciesName);
   if(theSpecies)
   {
      theSpecies->UpdateDiffusionEnergy1(diffEnergy);
   }
}
void ChemicalModel::SetTheOutputSpecies()
{
   using namespace CBMGenerics;
   ComponentManager & theManager = ComponentManager::getInstance();
   int i;
   for(i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i)
   {
      Species *theSpecies = GetSpeciesByName(theManager.GetSpeciesName( i ));
      if(theSpecies)
      {
         theSpecies->OutputResults(true);
      }
   }
   
}

}//end of namespace OTGC
