//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// std library
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <math.h>

#include "ChemicalModel.h"

#include "Element.h"
#include "Species.h"
#include "Reaction.h"
#include "ReactionRatio.h"
#include "SimulatorStateBase.h"
#include "SpeciesProperties.h"
#include "SpeciesState.h"

#include "Utilities.h"
#include "ComponentManager.h"
#include "GenexResultManager.h"
#include "GeneralParametersHandler.h"

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::KiloJouleToJoule;
#include "StringHandler.h"

namespace Genex6
{

int ChemicalModel::GetNumberOfElements() const
{
   return s_numberOfElements;
}
int ChemicalModel::GetNumberOfSpecies() const
{
   return s_numberOfSpecies;
}
int ChemicalModel::GetNumberOfReactions() const
{
   return s_numberOfReactions;
}
ChemicalModel::ChemicalModel(const int in_simulationType)
{
   m_simulationType = in_simulationType;
   m_SC = 0.0;
   m_HC = 0.0;

   for(int i = 0; i < m_speciesManager.getNumberOfSpecies (); ++ i) {
      m_theSpecies[i] = NULL;
   } 

   s_numberOfElements = 0;  
   s_numberOfSpecies = 0;  
   s_numberOfReactions = 0; 

}
ChemicalModel::~ChemicalModel()
{
   clearElements();
   clearSpecies();
   clearReactions();
}

void ChemicalModel::clearElements()
{
   std::vector<Element*>::iterator itEnd = m_theElements.end();

   for(std::vector<Element*>::iterator it = m_theElements.begin(); it != itEnd; ++it) {
      delete (*it);
   } 

   m_theElements.clear();
}
void ChemicalModel::clearSpecies()
{
   for(int i = 0; i < m_speciesManager.getNumberOfSpecies (); ++ i) {
      delete m_theSpecies[i];
   } 
}
void ChemicalModel::clearReactions()
{
   std::map<int,Reaction*>::iterator itEnd = m_theReactions.end();

   for(std::map<int,Reaction*>::iterator it = m_theReactions.begin(); it != itEnd; ++it) {
      delete (it->second);
   } 

   m_theReactions.clear();
}
//only for printing out results according to the VBA format
void ChemicalModel::AddPairInSpeciesId2Name(const int id, const std::string &name)
{
  s_mapSpeciesId2Name[name] = id;
}
//only for printing out results according to the VBA format
std::string ChemicalModel::GetSpeciesNameById(const int id) const
{
   std::string ret;
   std::map<std::string, int>::const_iterator itId;

   for(itId = s_mapSpeciesId2Name.begin();  itId != s_mapSpeciesId2Name.end(); ++ itId) {
      if(itId->second == id) {
         ret = itId->first;
         break;
      }
   } 
   return ret;
}
int ChemicalModel::GetSpeciesIdByName(const string &in_name) const
{
   int ret = 0;
   std::map<std::string,int >::const_iterator it = s_mapSpeciesId2Name.find(in_name);
   if(it != s_mapSpeciesId2Name.end()) {
      ret = it->second;
   }
   
   return ret;
}
const Species** ChemicalModel::GetSpecies() const
{
   return const_cast<const Species **>(m_theSpecies);
}
void ChemicalModel::AddElement(Element *theElement)
{
   int index = m_theElements.size();

   if(index > m_speciesManager.getNumberOfElements ()) {
      cout << "Warning!! Too big number of elements." << endl;
   }
   string elementName = theElement->GetName();

   m_speciesManager.setElementNum(elementName, index);

   theElement->SetId(index);
   m_theElements.push_back(theElement);   
}
Element *ChemicalModel::GetElementByName(const int in_name) const
{
   if(in_name < 0 || in_name > m_speciesManager.getNumberOfElements ()) return 0;
   return m_theElements[in_name];
}
Species *ChemicalModel::GetByNameSpecies(const std::string &in_name) const
{
   Species *functionReturn = 0;
   std::map<std::string,int >::const_iterator it = s_mapSpeciesId2Name.find(in_name);
   if(it != s_mapSpeciesId2Name.end()) {
      functionReturn = m_theSpecies[it->second-1]; 
   }
   return functionReturn;
}
Reaction *ChemicalModel::GetReactionByMotherName (const int theMotherId) const
{
   Reaction *functionReturn = 0;
   std::map<int,Reaction*>::const_iterator it = m_theReactions.find(theMotherId);
   if(it != m_theReactions.end()) {
      functionReturn = it->second;
   }
   return functionReturn;
}
void ChemicalModel::AddSpecies(Species *theSpecies)
{
   int speciesId = theSpecies->GetId();
 
   if(speciesId > m_speciesManager.getNumberOfSpecies () || speciesId <= 0) {
      cout << "Warning. Species id=" << speciesId << " is wrong (too big or negative)." << endl;
   }
   if(m_theSpecies[speciesId - 1] != NULL) {
      cout << "Warning. Species with id=" << speciesId <<" already exist." << endl;
   }
   m_theSpecies[speciesId - 1] = theSpecies; 
  
   m_speciesManager.setSpeciesNum(theSpecies->GetName(), speciesId);

   AddPairInSpeciesId2Name(speciesId, theSpecies->GetName());
}

void ChemicalModel::AddReaction(Reaction *theReaction)
{
   Species *mother = theReaction->GetMother();
   m_theReactions[mother->GetId()] = theReaction;
}

void ChemicalModel::UpdateSpeciesCompositionsByElementName(const int in_Species,
                                                           const int in_Element, 
                                                           const double in_CompositionFactor)  
{
   Element *theElem = GetElementByName(in_Element);
   Species *curSpecies = GetSpeciesById(in_Species);

   if(theElem && curSpecies) {
      curSpecies->UpdateCompositionByElement(theElem, in_CompositionFactor);
   }
}
void ChemicalModel::CompEarlySpecies() 
{
   Element *const HYDROGEN = GetElementByName(m_speciesManager.getHydrogenId ());
   Element *const OXYGEN   = GetElementByName(m_speciesManager.getOxygenId ());
   Element *const NITROGEN = GetElementByName(m_speciesManager.getNitrogenId ());
   Element *const SULPHUR  = GetElementByName(m_speciesManager.getSulphurId ());

   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();

   //bool s_c = (theHandler.GetParameterById(GeneralParametersHandler::SCratio) != 0.0 && SULPHUR != 0 ? true: false);
   bool s_c = (m_SC != 0.0 && SULPHUR != 0 ? true: false); 

   Species *curSpecies=0;
   //Kerogen initial start for getting value for AtomN
   //KerogenAtomN-->PreashalteneAtomN-->KerogenAtomO-->KerogenAtomN ...
   //preasphaltene O/C ratio of  as function of its H/C ratio

   curSpecies = GetSpeciesById(m_speciesManager.getPreasphaltId ());
   double PreashalteneAtomH = 0.0;
   double PreashalteneAtomO = 0.0;
   double PreashalteneAtomN = 0.0;
   double PreashalteneAtomS = 0.0;

   //HC value for preasphaltene were updated with GUI value in Stoichiometry::CheckInitialHCindex()
   if(curSpecies) {
      //Atom(IatomO, Lpreasphalt) = OCpreasphalt1 * Atom(IatomH, Lpreasphalt) ^ 2 +
      //OCpreasphalt2 * Atom(IatomH, Lpreasphalt) + OCpreasphalt3

      PreashalteneAtomH = curSpecies->GetCompositionByElement(m_speciesManager.getHydrogenId ());
      PreashalteneAtomO = 
         theHandler.GetParameterById(GeneralParametersHandler::OCpreasphalt1) * pow(PreashalteneAtomH, 2.0) +
         theHandler.GetParameterById(GeneralParametersHandler::OCpreasphalt2) * PreashalteneAtomH +
         theHandler.GetParameterById(GeneralParametersHandler::OCpreasphalt3);

      if(s_c) PreashalteneAtomS = m_SC; //theHandler.GetParameterById(GeneralParametersHandler::SCratio);
      curSpecies->UpdateCompositionByElement(OXYGEN, PreashalteneAtomO);
      if(s_c) curSpecies->UpdateCompositionByElement(SULPHUR, PreashalteneAtomS);
      //      cout << "PreashalteneAtomH = " << PreashalteneAtomO << endl;
   }
   //KEROGEN H/C & O/C as functions of those of preasphaltene
   //-------------Atom(IatomH, Lkerogen) = HCkerogen1 * Atom(IatomH, Lpreasphalt) + HCkerogen2
   double KerogenAtomH = theHandler.GetParameterById(GeneralParametersHandler::HCkerogen1) * 
      PreashalteneAtomH + theHandler.GetParameterById(GeneralParametersHandler::HCkerogen2);

   //-------------Atom(IatomO, Lkerogen) = OCkerogen1 * Atom(IatomO, Lpreasphalt) + OCkerogen2
   double KerogenAtomO = theHandler.GetParameterById(GeneralParametersHandler::OCkerogen1) * 
      PreashalteneAtomO + theHandler.GetParameterById(GeneralParametersHandler::OCkerogen2);

   //-----------------------Atom(IatomN, Lkerogen) = Atom(IatomO, Lkerogen) * Nkerogen
   double KerogenAtomN = KerogenAtomO * theHandler.GetParameterById(GeneralParametersHandler::Nkerogen);

   //-------------get Kerogen species and update
   curSpecies = GetSpeciesById(m_speciesManager.getKerogenId ());
   if(curSpecies) {
      curSpecies->UpdateCompositionByElement(HYDROGEN, KerogenAtomH);
      curSpecies->UpdateCompositionByElement(OXYGEN, KerogenAtomO);
      curSpecies->UpdateCompositionByElement(NITROGEN, KerogenAtomN);
      if(s_c) curSpecies->UpdateCompositionByElement(SULPHUR, PreashalteneAtomS);
   }
   //And now update the PreasphalteneAtomN
   //Atom(IatomN, Lpreasphalt) = Atom(IatomN, Lkerogen) * Npreasphalt
   curSpecies = GetSpeciesById(m_speciesManager.getPreasphaltId ());
   if(curSpecies) {
      PreashalteneAtomN = KerogenAtomN * theHandler.GetParameterById(GeneralParametersHandler::Npreasphalt);
      curSpecies->UpdateCompositionByElement(NITROGEN, PreashalteneAtomN);
   }
 
   //ASPHALTENE H/C and O/C
   //-----------------------Call AsphalComp
   //------------Atom(IatomH, Lasphaltene) = HCAsphOverPreasphalt * Atom(IatomH, Lpreasphalt)
   //double HCAsphOverPreasphalt = (isSim5() ?  
   //                               theHandler.GetParameterById(GeneralParametersHandler::HCAsphOverPreasphaltGX5) :
   //                               theHandler.GetParameterById(GeneralParametersHandler::HCAsphOverPreasphalt));
   
   double HCAsphOverPreasphalt =  theHandler.GetParameterById(GeneralParametersHandler::HCAsphOverPreasphaltGX5);
   if (!isSim5()) { HCAsphOverPreasphalt -= m_SC * 0.5;}
   double AsphalteneAtomH = HCAsphOverPreasphalt * PreashalteneAtomH;
   
   //------------Atom(IatomO, Lasphaltene) = OCAsphOverPreasphalt * Atom(IatomO, Lpreasphalt)
   //double OCAsphOverPreasphalt = (isSim5() ?  
   //                               theHandler.GetParameterById(GeneralParametersHandler::OCAsphOverPreasphaltGX5) :
   //                               theHandler.GetParameterById(GeneralParametersHandler::OCAsphOverPreasphalt));
   double OCAsphOverPreasphalt =  theHandler.GetParameterById(GeneralParametersHandler::OCAsphOverPreasphaltGX5);
   if(!isSim5()) OCAsphOverPreasphalt -= m_SC * 0.15;

   double AsphalteneAtomO = OCAsphOverPreasphalt * PreashalteneAtomO;
   if(!isSim5()) AsphalteneAtomO += 0.01; //0.01 Genex6
   
   //------------Atom(IatomN, Lasphaltene) = Atom(IatomN, Lpreasphalt) * Nasphaltene
   double  AsphalteneAtomN = PreashalteneAtomN * theHandler.GetParameterById(GeneralParametersHandler::Nasphaltene);

   if(AsphalteneAtomO < theHandler.GetParameterById(GeneralParametersHandler::OCasphMin)) {
      AsphalteneAtomO = theHandler.GetParameterById(GeneralParametersHandler::OCasphMin);
   }
   //------------Atom(IatomS, Lasphaltene) = Atom(IatomS, Lpreasphalt) * Sasphaltene
   double  AsphalteneAtomS = 0.0;
   if(s_c) AsphalteneAtomS = PreashalteneAtomS * theHandler.GetParameterById(GeneralParametersHandler::Sasphaltene);
   //----------------get asphaltene species and update
   curSpecies = GetSpeciesById(m_speciesManager.getAsphaltenesId ());
   if(curSpecies) {
      curSpecies->UpdateCompositionByElement(HYDROGEN, AsphalteneAtomH);
      curSpecies->UpdateCompositionByElement(OXYGEN, AsphalteneAtomO);
      curSpecies->UpdateCompositionByElement(NITROGEN, AsphalteneAtomN);
      if(s_c) curSpecies->UpdateCompositionByElement(SULPHUR, AsphalteneAtomS);
   }
   //RESIN H/C AND O/C make identical to those of asphaltene
   curSpecies = GetSpeciesById(m_speciesManager.getResinsId ());
   double ResinAtomH = AsphalteneAtomH;

   if(!isSim5()) ResinAtomH *= 1.1; //for clarity // 1.1 Genex6

   double ResinAtomO = AsphalteneAtomO; //for clarity

   //---------------Atom(IatomN, Lresin) = Atom(IatomN, Lasphaltene) * Nresin
   double ResinAtomN = AsphalteneAtomN * theHandler.GetParameterById(GeneralParametersHandler::Nresin);
   double ResinAtomS = 0.0;
   if(s_c) ResinAtomS = AsphalteneAtomS; //for clarity

   if(curSpecies) {
      curSpecies->UpdateCompositionByElement(HYDROGEN, ResinAtomH);
      curSpecies->UpdateCompositionByElement(OXYGEN, ResinAtomO);
      curSpecies->UpdateCompositionByElement(NITROGEN, ResinAtomN);
      if(s_c) curSpecies->UpdateCompositionByElement(SULPHUR, ResinAtomS);
   }
   //PRECOKE & COKE1 O/C as a function of that of preasphaltene
   //-----Call CokeComp
   //-----precoke and coke1 oxygen contents as a function of Preasphalt composition
   //-------Atom(IatomO, Lprecoke) = OCprecokeWhenHCpreasphaltZero - OCprecokePerPreasphalt *Atom(IatomH, Lpreasphalt)

   double OCprecokeWhenHCpreasphaltZero, OCprecokePerPreasphalt;

   if(isGX5()) {
      OCprecokeWhenHCpreasphaltZero = theHandler.GetParameterById(GeneralParametersHandler::OCprecokeWhenHCpreasphaltZeroGX5);
      OCprecokePerPreasphalt = theHandler.GetParameterById(GeneralParametersHandler::OCprecokePerPreasphaltGX5);   
   } else if(isOTGC5()) {
      OCprecokeWhenHCpreasphaltZero = theHandler.GetParameterById(GeneralParametersHandler::OCprecokeWhenHCpreasphaltZeroOTGC5);
      OCprecokePerPreasphalt = theHandler.GetParameterById(GeneralParametersHandler::OCprecokePerPreasphaltOTGC5);   
   } else {
//      OCprecokeWhenHCpreasphaltZero =  theHandler.GetParameterById(GeneralParametersHandler::OCprecokeWhenHCpreasphaltZero);
//      OCprecokePerPreasphalt = theHandler.GetParameterById(GeneralParametersHandler::OCprecokePerPreasphalt);
      OCprecokeWhenHCpreasphaltZero =  theHandler.GetParameterById(GeneralParametersHandler::OCprecokeWhenHCpreasphaltZeroGX5) - m_SC * 0.15;
      OCprecokePerPreasphalt = theHandler.GetParameterById(GeneralParametersHandler::OCprecokePerPreasphaltGX5) - m_SC * 0.015;
   }

   double PrecokeAtomO = OCprecokeWhenHCpreasphaltZero - OCprecokePerPreasphalt * PreashalteneAtomH;
   //---------------------Atom(IatomN, Lprecoke) = Atom(IatomN, Lresin) * Nprecoke
   double PrecokeAtomN = ResinAtomN * theHandler.GetParameterById(GeneralParametersHandler::Nprecoke);
   double PrecokeAtomS = 0.0;
   if(s_c) PrecokeAtomS = ResinAtomS * theHandler.GetParameterById(GeneralParametersHandler::Sprecoke);

   curSpecies= GetSpeciesById(m_speciesManager.getPrecokeId ());
   if(curSpecies) {
      curSpecies->UpdateCompositionByElement(OXYGEN, PrecokeAtomO);
      curSpecies->UpdateCompositionByElement(NITROGEN, PrecokeAtomN);
      if(s_c) curSpecies->UpdateCompositionByElement(SULPHUR, PrecokeAtomS);
  }
   //----------make O/C of Coke1 the same as Precoke
   double Coke1AtomO = PrecokeAtomO; //for clarity
   double Coke1AtomS = 0.0;
   if(s_c) Coke1AtomS = PrecokeAtomS * theHandler.GetParameterById(GeneralParametersHandler::Scoke1);
   curSpecies = GetSpeciesById(m_speciesManager.getCoke1Id ());
   if(curSpecies) {
      curSpecies->UpdateCompositionByElement(OXYGEN, Coke1AtomO);
      if(s_c) curSpecies->UpdateCompositionByElement(SULPHUR, Coke1AtomS);
   }
   //Hetero1
   //--------------------Atom(IatomN, Lhetero1) = Atom(IatomN, Lprecoke) * Nhetero1
   double Nhetero1 = (isSim5() ?
                      theHandler.GetParameterById(GeneralParametersHandler::Nhetero1GX5): 
                      theHandler.GetParameterById(GeneralParametersHandler::Nhetero1));
   double Hetero1AtomN = PrecokeAtomN * Nhetero1;
   curSpecies = GetSpeciesById(m_speciesManager.getHetero1Id ());
   if(curSpecies) {
      curSpecies->UpdateCompositionByElement(NITROGEN, Hetero1AtomN);
   }

//Hetero2
//--------------------Atom(IatomN, Lhetero2) = Nhetero2
//   Taken out in Genex6:
//    double Hetero2AtomN = theHandler.GetParameterById(GeneralParametersHandler::Nhetero2);
//    curSpecies = GetSpeciesByName("Hetero2");
//    if(curSpecies)
//    {
//       curSpecies->UpdateCompositionByElement(NITROGEN,Hetero2AtomN);
//    }
//C15+ Aro
//--------------------Atom(IatomN, Lc15plusAro) = NC15plusAro
   if(isSim5()) {
      double c15plusAroAtomN =  theHandler.GetParameterById(GeneralParametersHandler::NC15plusAroGX5);
      curSpecies = GetSpeciesById(m_speciesManager.getC15plusAroId ());
      if(curSpecies) {
         curSpecies->UpdateCompositionByElement(NITROGEN, c15plusAroAtomN);
      }
   }
}
//value of Emean in Joule
void ChemicalModel::KineticsEarlySpecies(const double Emean)
{
   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();

   Species *curSpecies = 0;
   //preasphaltene cracking activation energies as function of input mean activation energy
   //-----------------------Emean = Range("Input!Emean").Value
   //-----------------------Easph1 = 2 * Emean - EcrackingCC   
   
   double Easph1 = 2.0 * Emean - theHandler.GetParameterById(GeneralParametersHandler::EcrackingCC);
   //double EdropForS = theHandler.GetParameterById(GeneralParametersHandler::EdropPerS) * m_SC;
   //theHandler.GetParameterById(GeneralParametersHandler::SCratio);

   //Preasphaltene
   curSpecies = GetSpeciesById(m_speciesManager.getPreasphaltId ());
   double PreashalteneAtomH = curSpecies->GetCompositionByElement(m_speciesManager.getHydrogenId ());
   double EdropForS = 0.0;
   if( !isSim5() ) {
      EdropForS =  (m_SC * (341 * PreashalteneAtomH * PreashalteneAtomH - 439 * PreashalteneAtomH + 155)) * 1000;
   }

   if(curSpecies) {
      SpeciesProperties *const PreasphalteneProps = curSpecies->GetSpeciesProperties();

      // For compatibility with OTGC-5.
      double Ediff1 = (isSim5() ?
                       theHandler.GetParameterById(GeneralParametersHandler::Ediff1GX5) : 0.0);
      // double Ediff2 = (isGX5() ?
      //                  theHandler.GetParameterById(GeneralParametersHandler::Ediff2GX5) : 0.0);
      double Ediff2 = (isOTGC5() ?
                       0.0 : theHandler.GetParameterById(GeneralParametersHandler::Ediff2GX5));

      //-------------Act(1, Lpreasphalt) = Easph1 - Ediff1 - EdropForS
      double PreasphalteneactivationEnergy1 = Easph1 - Ediff1 - EdropForS;
      PreasphalteneProps->SetActivationEnergy1(PreasphalteneactivationEnergy1);

      //-------------Act(2, Lpreasphalt) = EcrackingCC - Ediff2
      double PreasphalteneactivationEnergy2 = theHandler.GetParameterById(GeneralParametersHandler::EcrackingCC) - Ediff2;
      PreasphalteneProps->SetActivationEnergy2(PreasphalteneactivationEnergy2);
   }
   //Asphaltene
   curSpecies = GetSpeciesById(m_speciesManager.getAsphaltenesId ());
   double AsphalteneactivationEnergy1 = 0.0;
   double AsphalteneactivationEnergy2 = 0.0;
   if(curSpecies) {
      SpeciesProperties *const AsphalteneProps = curSpecies->GetSpeciesProperties();
      //-------------Act(1, Lasphaltene) = Easph1
      AsphalteneactivationEnergy1 = Easph1;
      AsphalteneProps->SetActivationEnergy1(AsphalteneactivationEnergy1);
      //-------------Act(2, Lasphaltene) = EcrackingCC
      AsphalteneactivationEnergy2 = theHandler.GetParameterById(GeneralParametersHandler::EcrackingCC);
      AsphalteneProps->SetActivationEnergy2(AsphalteneactivationEnergy2);
   }
   //Resin
   curSpecies = GetSpeciesById(m_speciesManager.getResinsId ());
   if(curSpecies) {
      SpeciesProperties *const ResinProps = curSpecies->GetSpeciesProperties();
      //-------------Act(1, Lresin) = Act(1, Lasphaltene)
      ResinProps->SetActivationEnergy1(AsphalteneactivationEnergy1);
      //-------------Act(2, Lresin) = Act(2, Lasphaltene)
      //n.b. can introduce a small increase in E here (e.g. 2 kJ/mol), to slightly improve Type II data fit
      ResinProps->SetActivationEnergy2(AsphalteneactivationEnergy2);
   }
}
void ChemicalModel::ComputeStoichiometry()
{
   //need of  preasphalteneAromaticity in Reaction::ComputeProductRatios
   Species *const preasphaltene = GetSpeciesById(m_speciesManager.getPreasphaltId ());
   double preasphalteneAromaticity = preasphaltene->ComputeAromaticity();

   this->ComputeProductRatios(preasphalteneAromaticity);
   this->ComputeMassProductRatios();
}
void ChemicalModel::SetSpeciesReactionOrder()
{
   SpeciesProperties *curSpeciesProp; 

   if(isGX5()) {

      curSpeciesProp = GetSpeciesById(m_speciesManager.getKerogenId ())->GetSpeciesProperties();
      curSpeciesProp->SetReactionOrder(1.5);
      Species *specPreashalt = GetSpeciesById(m_speciesManager.getPreasphaltId ());
      curSpeciesProp = specPreashalt->GetSpeciesProperties();
      curSpeciesProp->SetReactionOrder(specPreashalt->ComputeReactionOrder());

   } else if(( m_simulationType & Genex6::Constants::SIMOTGC ) and ( ! isTSR () )) {

      for(int i = 0; i < m_speciesManager.getNumberOfSpecies (); ++i) {
         if(m_theSpecies[i] != NULL) {
            curSpeciesProp = m_theSpecies[i]->GetSpeciesProperties();
            if(curSpeciesProp->IsReactive()) {
               curSpeciesProp->SetReactionOrder(1.5);
            }
         }
      }
      curSpeciesProp = GetSpeciesById(m_speciesManager.getC2Id ())->GetSpeciesProperties();

      if(curSpeciesProp->IsReactive()) { 
         curSpeciesProp->SetReactionOrder(2.0);
      }

   } else if( isTSR() ) {
       for(int i = 0; i < m_speciesManager.getNumberOfSpecies (); ++i) {
         if(m_theSpecies[i] != NULL) {
            curSpeciesProp = m_theSpecies[i]->GetSpeciesProperties();
            if(curSpeciesProp->IsReactive()) {
               curSpeciesProp->SetReactionOrder(1.5);
            }
         }
      }
      curSpeciesProp = GetSpeciesById(m_speciesManager.getC1Id ())->GetSpeciesProperties();
 
      if(curSpeciesProp->IsReactive()) {
         curSpeciesProp->SetReactionOrder( 1.0 );
      }
     
   } else {
      double reactionOrderToSet;
      for(int i = 0, curSpecId = 1; i < m_speciesManager.getNumberOfSpecies (); ++i, ++ curSpecId) {
         Species *curSpec = m_theSpecies[i];
         if(curSpec == NULL) continue;

         curSpeciesProp = curSpec->GetSpeciesProperties();

         if(curSpeciesProp->IsReactive()) {
            if(curSpecId == m_speciesManager.getKerogenId () ||
               curSpecId == m_speciesManager.getC15plusAroId () ||
               curSpecId == m_speciesManager.getC15plusSatId () ||
               curSpecId == m_speciesManager.getC6to14AroId () ||
               curSpecId == m_speciesManager.getC6to14SatId () ||
               curSpecId == m_speciesManager.getPreasphaltId () ||
               curSpecId == m_speciesManager.getC5Id () ||
               curSpecId == m_speciesManager.getC4Id () ||
               curSpecId == m_speciesManager.getC3Id () ||
               curSpecId == m_speciesManager.getC2Id () ||
               
               curSpecId == m_speciesManager.getC15plusAroSId () ||
               curSpecId == m_speciesManager.getC15plusSatSId () ||
               curSpecId == m_speciesManager.getLSCId () ||
               curSpecId == m_speciesManager.getC15plusATId () ||
               curSpecId == m_speciesManager.getC6to14BTId () ||
               curSpecId == m_speciesManager.getC6to14DBTId () ||
               curSpecId == m_speciesManager.getC6to14BPId () ||
               curSpecId == m_speciesManager.getC6to14AroSId () ||
               curSpecId == m_speciesManager.getC6to14SatSId ()) {
              
               reactionOrderToSet = 1.5;
            } else {
               reactionOrderToSet = 1.0;
            }
            curSpeciesProp->SetReactionOrder(reactionOrderToSet);
         }
      }
   }
}

void ChemicalModel::ComputeSpeciesUltimateMasses(SimulatorStateBase *theState)
{
   double * ultMasses = theState->getSpeciesUltimateMasses();

   memset(ultMasses, 0, sizeof(double) * m_speciesManager.getNumberOfSpecies ());

   theState->initSpeciesUltimateMass ( m_speciesManager );

   for(int i = 0, id = 1; i < m_speciesManager.getNumberOfSpecies (); ++i, ++ id) {
      Species *currentSpecies = m_theSpecies[i];
      if(currentSpecies == NULL) continue;

      // There is no check for kerogen in OTGC-5.
      if(id != m_speciesManager.getKerogenId ()) { 
         double currentUltimateMass = ultMasses[i];
         
         if(currentSpecies->GetSpeciesProperties()->IsReactive()) {
            const double * SpeciesProducts = currentSpecies->GetProductMassFactors();
            
            for(int j = 0; j < m_speciesManager.getNumberOfSpecies (); ++ j) {
               if(SpeciesProducts[j] > 0.0) {
                  ultMasses[j] += SpeciesProducts[j] * currentUltimateMass;
               }
            }	
         }
      }
   }
}

void ChemicalModel::ComputeProductRatios(const double preasphalteneAromaticity)
{
   std::map<int,Reaction*>::iterator itR = m_theReactions.begin();
   while(itR != m_theReactions.end()) {
      itR->second->ComputeProductRatios(preasphalteneAromaticity);
      ++ itR;
   }
}
void ChemicalModel::ComputeMassProductRatios()
{
   std::map<int,Reaction*>::iterator itR = m_theReactions.begin();
   while(itR != m_theReactions.end()) {
      itR->second->ComputeMassProductRatios();
      ++ itR;
   }

}
void ChemicalModel::UpdateSpeciesProperties()
{
   for(int i = 0; i < m_speciesManager.getNumberOfSpecies (); ++ i) {
      if(m_theSpecies[i] != NULL) m_theSpecies[i]->UpdateProperties();
   }
}
double ChemicalModel::GetElemAtomWeightByName(const int ElemName) const
{
   Element *const Elem = GetElementByName(ElemName);
   double ret=0.0;
   if(Elem) {
      ret = Elem->GetAtomWeight();
   }
   return ret;
}

double ChemicalModel::GetSpeciesMolWeightByName(const int SpeciesName) const
{
   const Species *const theSpecies = GetSpeciesById(SpeciesName);
   double ret = 0.0;
   if(theSpecies) {
      ret = theSpecies->GetMolWeight();
   }
   return ret;
}

double ChemicalModel::GetSpeciesDensityByName(const int SpeciesName) const
{
   const Species *const theSpecies = GetSpeciesById(SpeciesName);
   double ret = 0.0;
   if(theSpecies) {
      ret = theSpecies->GetDensity();
   }
   return ret; 
}
double ChemicalModel::GetSpeciesB0ByName(const int SpeciesId) const
{
   Species *theSpecies = GetSpeciesById(SpeciesId);
   double ret = 0.0;
   if(theSpecies) {
      ret = theSpecies->GetSpeciesProperties()->GetB0();
   }
   return ret; 
}
void ChemicalModel::InitializeSpeciesTimeStepVariables()
{
   //Species::SetSpeciesTimeStepVariablesToZero();

   double initialTimeStepGenRate = 0.0;
   
   for(int i = 0; i < m_speciesManager.getNumberOfSpecies (); ++ i) {
      if(m_theSpecies[i] != NULL) {
         m_theSpecies[i]->SetPositiveGenRate(initialTimeStepGenRate);
      }
   }
}
bool ChemicalModel::Validate() const  
{
   bool status = true;
  
   using namespace CBMGenerics;
   ComponentManager & theManager = ComponentManager::getInstance();
   
   int i, speciesId; 

#if 0 // has been moved to Species::validate method
	 Species *theSpecies;
     if( !isGenex() && !isOTGC5() ) {
      theSpecies = GetSpeciesById(m_speciesManager.getResinsId());
      double negativeFractionPrecoke = theSpecies->GetMassFactorBySpecies(m_speciesManager.getPrecokeId());
      if( negativeFractionPrecoke < 0 ) {
         double fractionC15plusSat = theSpecies->GetMassFactorBySpecies(m_speciesManager.getC15plusSatId());
         theSpecies->setMassFactorBySpecies(m_speciesManager.getC15plusSatId(), fractionC15plusSat + negativeFractionPrecoke);
      }
   }
#endif
   
   for(i = 0, speciesId = 1; i < ComponentManager::NUMBER_OF_SPECIES; ++i, ++ speciesId) {
      Species *theSpecies = (GetByNameSpecies(theManager.getSpeciesName(i)));  

      // status = (theSpecies == 0) ? false : theSpecies->validate();
      if( theSpecies != 0 ) { // to support GX5 and GX6 configuration files (different species)
         status = theSpecies->validate();
         if(status == false) {
            break;
         } 
      }                         
   }
   return status;
}          
void ChemicalModel::DebugOutput()
{
   for(int i = 0, id = 1; i < m_speciesManager.getNumberOfSpecies (); ++ i, ++ id) {
      if(m_theSpecies[i] != NULL) {
         cout<< "Name(" << id << ") = " << m_theSpecies[i]->GetName() << "; CompCode = " << m_theSpecies[i]->GetCompositionCode() << endl;
      }
   }
}                      
void ChemicalModel::ComputeTimeStep(SimulatorStateBase &theSimulatorState,
                                    const double in_dT,
                                    const double s_Peff,
                                    const double s_TK,
                                    const double s_FrequencyFactor,
                                    const double s_kerogenTransformationRatio,
                                    const double s_precokeTransformationRatio,
                                    const double s_coke2TransformationRatio,
                                    const double s_DiffusionConcDependence,
                                    const double s_VogelFulcherTemperature,
                                    const bool in_OpenSourceRockConditions)

{

   //initialisation for this time step
   //       For L = 1 To Ln
   //             RateGenPos(L) = 0!
   //            Theta(L) = 0#
   //         Next L
   //initialise chemical model, ready for computation
   InitializeSpeciesTimeStepVariables();
   theSimulatorState.SetSpeciesTimeStepVariablesToZero();

   for(int id = 1; id <= m_speciesManager.getNumberOfSpecies (); ++id) {
      Species* theSpecies=GetSpeciesById(id);
      if(theSpecies != NULL) {
         theSpecies->ComputeTimeStep(theSimulatorState,
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

         theSimulatorState.PostProcessTimeStep(* theSpecies, in_dT);
      }
   }
}
double ChemicalModel::GetSpeciesCompositionByElement(const int SpeciesId, const int ElementId)
{
   double ret=0.0;
   Species *theSpecies = GetSpeciesById(SpeciesId);
   if(theSpecies) {
      ret = theSpecies->GetCompositionByElement(ElementId);
   }
   return ret;
}
void ChemicalModel::UpdateSpeciesDiffusionEnergy1(const int SpeciesName, const double diffEnergy)
{
   Species *theSpecies = GetSpeciesById(SpeciesName);
   if(theSpecies) {
      theSpecies->UpdateDiffusionEnergy1(diffEnergy);
   }
}
void ChemicalModel::SetTheOutputSpecies()
{
   using namespace CBMGenerics;
   ComponentManager & theManager = ComponentManager::getInstance();
   for(int i = 0; i < ComponentManager::NUMBER_OF_SPECIES; ++i) {
      Species *theSpecies = GetByNameSpecies(theManager.getSpeciesName(i));
      if(theSpecies) {
         theSpecies->OutputResults(true);
      }
   }
   
}
void ChemicalModel::PrintConfigurationFileEntities(ofstream &outfile)
{
   outfile << "Table:[Elements]" << endl;
   outfile << "ElementName,AtomWeight" << endl;

   std::vector<Element*>::size_type sz = m_theElements.size();

   for( unsigned int id = 0; id < sz; ++ id ) {
      m_theElements[id]->OutputOnFile(outfile);
   }
   outfile << "[EndOfTable]" << endl;
   //----------------------------------------------------------------------------
   outfile << endl;
   outfile << "Table:[Species]" << endl;
   outfile << "SpeciesId,SpeciesName" << endl;

   for(int id = 0; id < m_speciesManager.getNumberOfSpecies (); ++ id) {
      if(m_theSpecies[id] != NULL) {
         outfile << id + 1 << "," << m_theSpecies[id]->GetName() << endl;
      }
   }
   outfile << "[EndOfTable]" << endl;
   //-----------------------------------------------------------------------------
   outfile << endl;
   outfile << "Table:[SpeciesCompositionByName]" << endl;
   outfile << "SpeciesName,CompositionCode,CompositionFactorC, CompositionFactorH, CompositionFactorO, CompositionFactorN, CompositionFactorS" << endl;
   for(int id = 0; id < m_speciesManager.getNumberOfSpecies (); ++ id) {
      if(m_theSpecies[id] != NULL) {
         m_theSpecies[id]->OutputCompositionOnFile(outfile);
      }
   }   
   outfile << "[EndOfTable]" << endl;
   //-----------------------------------------------------------------------------
   outfile << endl;
   outfile << "Table:[SpeciesPropertiesByName]" << endl;
   outfile << "SpeciesName,Weight,Density,activationEnergy1,activationEnergy2,entropy,volume,reactionOrder,diffusionEnergy1,diffusionEnergy2,jumpLength, B0,aromaticity" << endl;
   for(int id = 0; id < m_speciesManager.getNumberOfSpecies (); ++ id) {
      if(m_theSpecies[id] != NULL) {
         m_theSpecies[id]->OutputPropertiesOnFile(outfile);
      }
   }   
   outfile << "[EndOfTable]" << endl;
   //-----------------------------------------------------------------------------
   outfile << endl;
   outfile << "Table:[ReactionsBySpeciesName]" << endl;
   outfile << "MotherName,Product1, Product2, ..." << endl;
   std::map<int,Reaction*>::iterator itR=m_theReactions.begin();
   while(itR != m_theReactions.end()) {
      itR->second->OutputProductsOnFile(outfile);
      itR++;
   }
   outfile << "[EndOfTable]" << endl;
   //-----------------------------------------------------------------------------
   outfile << endl;
   outfile << "Table:[ReactionRatiosBySpeciesName]" << endl;
   outfile << "MotherName,Rectant1,Reactant2,Ratio" << endl;
   itR = m_theReactions.begin();
   while(itR != m_theReactions.end()) {
      itR->second->OutputReactionRatiosOnFile(outfile);
      itR++;
   }
   outfile << "[EndOfTable]" << endl;
   outfile << endl;
   //-----------------------------------------------------------------------------
}
void ChemicalModel::PrintBenchmarkSpeciesPropertiesHeader(ofstream &outfile) const
{
   outfile << "[Table:SpeciesPropertiesC++]" << endl;

   outfile  << "spec. no." << ","
            << "name" << ","
            << "C" << ","
            << "H" << ","
            << "O" << ","
            << "N" << ","
            << "S" << ","
            << "Dens" << ","
            << "FW" << ","
            << "Arom" << ","
            << "Er1" << ","
            << "Er2" << ","
            << "n" << ","
            << "Ed1" << ","
            << "B0" << ","
            << "IsMobile" << ","
            << "IsReactive" << ","
            << "IsHC" << ","
            << "IsOil" << ","
            << "IsHCgas" << endl;
}
void ChemicalModel::PrintBenchmarkSpeciesProperties(ofstream &outfile) const
{
   PrintBenchmarkSpeciesPropertiesHeader(outfile);

   for(int i = 0; i < m_speciesManager.getNumberOfSpecies (); ++ i) {
      if(m_theSpecies[i] != NULL) {
         m_theSpecies[i]->PrintBenchmarkProperties(outfile);
      }
   }
   if(isSim5()) {
      for(int i = 0; i < 13; ++ i) {
         outfile << endl;
      }
   }
}
void ChemicalModel::PrintBenchmarkStoichiometryHeader(ofstream &outfile) const
{
   outfile << "[Table:StoichiometryC++]" << endl;	
   outfile << "MotherName,";
   for(int i = 0; i < m_speciesManager.getNumberOfSpecies (); ++ i) {
      if(m_theSpecies[i] != NULL) {
         outfile << m_theSpecies[i]->GetName() << ",";
      }
   }
   outfile << endl;
}
#if 0
void ChemicalModel::LoadBenchmarkStoichiometry(string & namefile)
{
   ifstream inputFile;
   inputFile.open(namefile.c_str());
   if(!inputFile.fail()) {

      std::map<int,Reaction*>::iterator itR = m_theReactions.begin();
      while(itR != m_theReactions.end()) {
         itR->second->GetMother()->LoadStoichiometry(inputFile);
         ++ itR;
      }   
      inputFile.close();
   }
}
#endif
void ChemicalModel::PrintBenchmarkStoichiometry(ofstream &outfile) const
{
   PrintBenchmarkStoichiometryHeader(outfile);
   for(int i = 0; i < m_speciesManager.getNumberOfSpecies (); ++ i) {
      if(m_theSpecies[i] != NULL) {
         m_theSpecies[i]->PrintBenchmarkStoichiometry(outfile);
      }
   }
   if(isSim5()) {
      for(int i = 0; i < 11; ++ i) {
         outfile << endl;
      }
   }
}
void ChemicalModel::LoadElements(ifstream &ConfigurationFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";

   std::getline (ConfigurationFile, line, '\n');

   for(;;) {

      std::getline (ConfigurationFile, line, '\n');
     
      if(line==Genex6::CFG::EndOfTable || line.size() == 0) {
         break;
      }
      
      StringHandler::parseLine(line, delim, theTokens);
      
      Element *theElement = new Genex6::Element(theTokens[0]);
      theElement->SetAtomWeight(atof(theTokens[1].c_str()));
      this->AddElement(theElement);
      
      theTokens.clear();
   }
   
   s_numberOfElements = (int)m_theElements.size();
}
void ChemicalModel::LoadSpecies(ifstream &ConfigurationFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";



   std::getline (ConfigurationFile, line, '\n');

   int numberOfSpeciesInConfigFile = 0; 
   for(;;) {
      std::getline (ConfigurationFile, line, '\n');

      if(line == Genex6::CFG::EndOfTable || line.size() == 0) {
         break;
      }
      
      StringHandler::parseLine(line, delim, theTokens);
      
      Species *theSpecies = new Species(theTokens[1], atoi(theTokens[0].c_str()), this);
      this->AddSpecies(theSpecies);
      
      ++ numberOfSpeciesInConfigFile;
      
      theTokens.clear();
   }
   s_numberOfSpecies = numberOfSpeciesInConfigFile;

}
void ChemicalModel::LoadSpeciesComposition(ifstream &ConfigurationFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
   
   std::getline (ConfigurationFile, line, '\n');
   StringHandler::parseLine(line, delim, theTokens);
   
   int i, j;
   int theElements[m_speciesManager.numberOfElements];

   int tokenSize = theTokens.size();
   for(i = 0; i < m_speciesManager.getNumberOfElements (); ++ i) {
      theElements[i] = -1;
   }
   if(tokenSize - 2 > m_speciesManager.getNumberOfElements ()) {
      cout << "Warning!! Wrong number of elements in Species composition." << endl;
   }
   //Process header
   for(i = 2, j = 0; i < tokenSize; ++ i, ++ j) {
      //CompositionFactorC -->extract "C" into Element
      char elementFromHeader = theTokens[i][theTokens[i].size()-1];
      std::string Element(1,elementFromHeader);

      theElements[j] = m_speciesManager.GetElementIdByName(Element);
      if(theElements[j] < 0) {
         //   cout << "Warning!! Wrong element " << Element << " in Species composition." << endl;
      }
   }
   theTokens.clear();
   //Process main body
   for(;;) {

      std::getline (ConfigurationFile, line, '\n');

      if(line == Genex6::CFG::EndOfTable || line.size() == 0) {
         break;
      }
      
      StringHandler::parseLine(line, delim, theTokens);
      
      if(tokenSize - 2 > m_speciesManager.getNumberOfElements ()) {
         cout << "Warning!! Wrong number of elements in Species composition." << endl;
      }
      
      Species *theSpecies = this->GetByNameSpecies(theTokens[0]);
      for(i = 2, j = 0; i < tokenSize; ++ i, ++ j) {
         double compositionCode = atof(theTokens[i].c_str());
         if(compositionCode > Genex6::Constants::Zero) {
            if(theElements[j] < 0) continue;
            Element *const theElem = this->GetElementByName(theElements[j]);
            theSpecies->UpdateCompositionByElement(theElem,compositionCode);
         }
      }
      theTokens.clear();
   }
}
void ChemicalModel::LoadSpeciesProperties(ifstream &ConfigurationFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";

   std::getline (ConfigurationFile, line, '\n');
   
   for(;;) {
      std::getline (ConfigurationFile, line, '\n');

      if(line == Genex6::CFG::EndOfTable || line.size() == 0)
      {
         break;
      }
      
      StringHandler::parseLine(line, delim, theTokens);
      
      //SpeciesName,Weight,Density,activationEnergy1,activationEnergy2,entropy,volume,reactionOrder,diffusionEnergy1,diffusionEnergy2,jumpLength, B0,aromaticity
      
      //SpeciesProperties
      double weight             = atof(theTokens[1].c_str());
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
      
      if(reactionOrder == 0) reactionOrder = 1.0;
      
      Species *theSpecies = this->GetByNameSpecies(theTokens[0]);
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
                                                               Aromaticity);
      theProperties->SetMolWeight( weight );
      theSpecies->SetProperties(theProperties);
      theTokens.clear();
   }
}
void ChemicalModel::LoadReactions(ifstream &ConfigurationFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
   
   std::getline (ConfigurationFile, line, '\n');

   for(;;) {
      std::getline (ConfigurationFile, line, '\n');
 
      if(line == Genex6::CFG::EndOfTable || line.size() == 0) {
         break;
      }
      
      StringHandler::parseLine(line, delim, theTokens);
      Reaction * currentReaction = 0;
      std::vector<std::string>::size_type i = 0;
      for( i = 0; i < theTokens.size(); ++ i) {
         if(i == 0) {
            Species * theMother = GetByNameSpecies(theTokens[i]);
            if(theMother) {
               currentReaction = new Reaction(theMother);
            } else {
               //throw
            }
         } else {
            Species * theProduct = GetByNameSpecies(theTokens[i]);
            if(theProduct) {
               currentReaction->AddProduct(theProduct);
            } else {
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
   std::string delim = ",";

   std::getline (ConfigurationFile, line, '\n');

   for(;;) {

      std::getline (ConfigurationFile, line, '\n');

      if(line == Genex6::CFG::EndOfTable || line.size() == 0) {
         break;
      }
      
      StringHandler::parseLine(line, delim, theTokens);
      
      Reaction *currentReaction = GetReactionByMotherName(s_mapSpeciesId2Name[theTokens[0]]);
      if(currentReaction && theTokens.size() == 4) {
         Species *Reactant1 = GetByNameSpecies(theTokens[1]);
         Species *Reactant2 = GetByNameSpecies(theTokens[2]);
         ReactionRatio *theRatio = new ReactionRatio(Reactant1,Reactant2,theTokens[3]);
         
         currentReaction->AddReactionRatio(theRatio);
      } else {
         //throw
      }
      theTokens.clear();
   } 
}
ChemicalModel::ChemicalModel(const std::string &in_FulPathConfigurationFileName, const int in_simulationType)
{
   m_simulationType = in_simulationType;
   for(int i = 0; i < m_speciesManager.getNumberOfSpecies (); ++ i) {
      m_theSpecies[i] = NULL;
   } 

   ifstream theReactFile;
   
   theReactFile.open(in_FulPathConfigurationFileName.c_str());
 
   if(!theReactFile) {
      cerr << "MeSsAgE ERROR Attempting to open file : " + in_FulPathConfigurationFileName + "\nNo cfg file available in the $GENEX5DIR directory... Aborting... " << endl;
      throw ("MeSsAgE ERROR Attempting to open file : " + in_FulPathConfigurationFileName + "\nNo cfg file available in the $GENEX5DIR directory... Aborting... ");
   }
   std::string line;
   //temp values just for the C++ prototype
   //not needed for the new file structure
   //------------------------------------------------------------------------------------------------
   for(int i = 0; i < 2; ++ i) {

      std::getline (theReactFile, line, '\n');

      cout << line << endl;
      if(i == 0) {
         m_name = line;
      }
   }
   std::vector<std::string> theTokens;
   std::string delim(",\n0");
   std::string::size_type startPos = 0;
   std::string::size_type endPos = 0;
   
   while(endPos != std::string::npos) {
      endPos = line.find_first_of(delim,startPos);
      std::string::size_type increment = endPos - startPos;
      std::string tempString = line.substr(startPos,increment);
      theTokens.push_back(tempString);
      startPos += increment + 1;
   }
   s_numberOfElements = atoi(theTokens[1].c_str());
   s_numberOfSpecies = atoi(theTokens[0].c_str());
   
   theTokens.clear();

   //DEBUG MAD   
   cout << "Number of Elements " << s_numberOfElements << endl;
   //DEBUG MAD  
   cout << "Number of Species  " << s_numberOfSpecies  << endl;
   //------------------------------------------------------------------------------------------------
   SetTheElements(theReactFile);
   SetTheSpecies(theReactFile);
   //------------------------------------------------------------------------------------------------
   
   std::getline (theReactFile, line, '\n');

   if(!line.empty()) {
      s_numberOfReactions = atoi(line.c_str());
   }
   //DEBUG  MAD 
   cout << "Number of Reactions " << s_numberOfReactions << endl;
   SetTheReactions(theReactFile);
   
   //DEBUG std::cout<<"Operation finished"<<std::endl;
   theReactFile.close();
}
   
void ChemicalModel::SetTheElements(ifstream &theStream)
{
   clearElements();
   
   std::string line;

   std::getline (theStream, line, '\n');

   cout << line << endl;
   
   std::vector<std::string> theTokens;
   std::string delim(",");
   std::string::size_type startPos = 0;
   std::string::size_type endPos = 0;
   
   while(endPos!=std::string::npos) {
      endPos = line.find_first_of(delim,startPos);
      std::string::size_type increment = endPos - startPos;
      std::string tempString = line.substr(startPos,increment);
      theTokens.push_back(tempString);
      startPos += increment + 1;
   }
   std::string name;
   for(int i = 0; i != s_numberOfElements; ++ i) {
      switch(i) {
      case 0:  name = "C"; break;
      case 1:  name = "H"; break;
      case 2:  name = "O"; break;
      case 3:  name = "N"; break;
      case 4:  name = "S"; break;
      default:;
      }
      Genex6::Element *theEl = new Genex6::Element(name);
      theEl->SetAtomWeight(atof(theTokens[i].c_str()));
      AddElement(theEl);
   }
   theTokens.clear();
}
//should not be used in the product code
//only in the prototype for direct comparison with VBA prototype
void ChemicalModel::SetTheSpecies(ifstream &theStream)
{
   clearSpecies();

   for(int SpeciesId = 1; SpeciesId <= s_numberOfSpecies; ++ SpeciesId) {
      std::string line;
      std::getline (theStream, line, '\n');

      cout << line << endl;
      std::vector<std::string> theTokens;
      std::string delim(",\n");
      std::string::size_type startPos = 0;
      std::string::size_type endPos = 0;

      //DEBUG MAD
      
      while(endPos != std::string::npos) {
         endPos = line.find_first_of(delim,startPos);
         std::string::size_type increment = endPos - startPos;
         std::string tempString = line.substr(startPos,increment);
         theTokens.push_back(tempString);
         startPos += increment + 1;
      }
      //species name
      theTokens[0].erase(0, 1);
      std::string speciesName = theTokens[0].erase(theTokens[0].size() - 1, 1);
      int counter = 0;
      if(s_numberOfSpecies > 23) counter = 7; else counter = 6;
      //SpeciesProperties
      double activationEnergy1 = KiloJouleToJoule * atof(theTokens[counter ++].c_str()); // 7
      double activationEnergy2 = KiloJouleToJoule * atof(theTokens[counter ++].c_str()); // 8
      double entropy           = atof(theTokens[counter ++].c_str()); // 9
      double volume            = atof(theTokens[counter ++].c_str()); // 10
      double reactionOrder     = atof(theTokens[counter ++].c_str()); // 11
      double diffusionEnergy1  = KiloJouleToJoule*atof(theTokens[counter ++].c_str()); //12
      double diffusionEnergy2  = 0.0;     //generally not defined in sch files, implicitly changed at later stage
      double jumpLength        = atof(theTokens[counter + 3].c_str()); // 16
      double density           = atof(theTokens[counter + 4].c_str()); // 17

      Species *theSpec=0;
      SpeciesProperties *theProp = 0;

      theSpec = new Species(speciesName, SpeciesId, this);
      theProp = new SpeciesProperties(theSpec,
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
                                      0.0);//aromaticity
      theSpec->SetProperties(theProp);


      //element composition factor, this has to change in the product implementation
      //so that arbitrary elements per species can be supported
      //use a "composition code" forward declaration in the cfg file instead
      int elementId;
      double factor;
      Element *elem2Add = 0;

      for(int j = 2; j < 7; ++ j) {
         if(theTokens[j].size()) {
            elementId = -1;

            factor = atof(theTokens[j].c_str());
            if(fabs(factor - 0.0001) > 0.001) {
               switch(j) {
               case 2:  elementId = m_speciesManager.getCarbonId (); break;
               case 3:  elementId = m_speciesManager.getHydrogenId (); break;
               case 4:  elementId = m_speciesManager.getOxygenId (); break;
               case 5:  elementId = m_speciesManager.getNitrogenId (); break;
               case 6:  elementId = m_speciesManager.getSulphurId (); break;
               default:;
               } 
            }
            if(elementId >= 0) {
               elem2Add = this->GetElementByName(elementId);
               if(elem2Add) {
                  theSpec->UpdateCompositionByElement(elem2Add, factor);
               }
            }
         }
      }
      this->AddSpecies(theSpec);
      //update the map used for retrieving the VBA prototype codes
      //won't be used in the production code
      theTokens.clear();
   }//end of for(int i=0;i<speciesNr;i++)
   //DEBUG std::cout<<"finished with the species"<<std::endl;
}
void ChemicalModel::SetTheReactions(ifstream &theStream)
{
   clearReactions();

   std::string line;
   std::string delim(",\n");
   std::string tempString;
   std::vector<std::string> Tokens;  //buffer containing all the reactants
   std::vector<std::string> tempReactants;  //buffer containing all the reactants
   std::vector<std::string> tempRatios;     //buffer containing all the reaction ratios

   for(int i = 1; i <= s_numberOfReactions; ++ i) {

      std::getline (theStream, line, '\n');

      //DEBUG std::cout<<"---------------Start of Reaction----------Number:"<<i<<std::endl;
      //DEBUG MAD
      cout << line << endl;
      
      StringHandler::parseLine(line, delim, Tokens);
      
      Tokens.pop_back();
      std::vector<std::string>::iterator endOfReactants = std::find(Tokens.begin(), Tokens.end(), "0");
      
      for(std::vector<std::string>::iterator reactantsIt = Tokens.begin(); 
          reactantsIt != endOfReactants; ++ reactantsIt) {
         cout<<(*reactantsIt) << endl;
         tempReactants.push_back((*reactantsIt));
      }
      
      if(endOfReactants != Tokens.end()) {    
         cout << "No end" << endl;  
         
         for(std::vector<std::string>::iterator ratiosIt = endOfReactants + 1; 
             ratiosIt != Tokens.end(); ++ ratiosIt) {  
            cout << (*ratiosIt) << endl;
            tempRatios.push_back(*ratiosIt);
         }
      }
      if(tempRatios.size() % 3 != 0) {
         //MAD DEBUG
         cout << "Problem with Reaction Ratios" << endl;
         //must throw an exception here!
      }
      //-------------------------Set up the Reaction-----------------------------------------------
      Reaction * currentReaction = 0;
      std::vector<std::string>::size_type index = 0;
      
      if(!tempReactants.empty()) {
         //last one is always the mother in VBA prototype cfg file
         //Species   *theMother=GetSpeciesById(atoi(tempReactants[tempReactants.size()-1].c_str()));
         int theId = atoi(tempReactants[tempReactants.size() - 1].c_str());
         const std::string MotherName=GetSpeciesNameById(theId);

         //DEBUG MAD
         cout << "Mother Name and id: "<< MotherName << " "<< theId << endl;
         
         Species * theMother = GetSpeciesById(theId);
         if(theMother) {
            currentReaction = new Reaction(theMother);
         }
         for(index = 0; index < tempReactants.size() - 1; ++ index) {
            theId = atoi(tempReactants[index].c_str());
            //Species *theProduct=GetSpeciesById(atoi(tempReactants[index].c_str()));
            const std::string productName = GetSpeciesNameById(theId);
            
            Species *theProduct = GetSpeciesById(theId);
            if(theProduct) {
               currentReaction->AddProduct(theProduct);
               currentReaction->SetProductId(index + 1, theProduct); //
               cout << "Product name and Reaction id: "<< productName << " " << index + 1 << endl;
            }
         }
      }
      //-------------------------Set up the Reaction Ratios----------------------------------------
      if(!tempRatios.empty()) {
         cout << "Size of ratios" << tempRatios.size() << endl;
         //for(index = 0; index < tempRatios.size();index++)
         index = 0;
         while(index < tempRatios.size()) {
            Species *const reactant1 = currentReaction->GetProductById(atoi(tempRatios[index++].c_str()));
            Species *const reactant2 = currentReaction->GetProductById(atoi(tempRatios[index++].c_str()));
            const std::string functionCode = tempRatios[index++];
            cout << reactant1->GetName() << " " << reactant2->GetName() << " " << functionCode << endl;
            ReactionRatio * theRatio = new ReactionRatio(reactant1,reactant2,functionCode);
            if(theRatio) {
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

void ChemicalModel::ComputeB0()
{
   for(int i = 0; i < m_speciesManager.getNumberOfSpecies (); ++ i) {
      if(m_theSpecies[i] != NULL) {
          m_theSpecies[i]->ComputeB0();
      }
   }
}
void ChemicalModel::ComputePseudoActEnergyRadical()
{
   // If Not OTGC
   const double C2FineTuningFactor = 1.5;

   for(int i = 0, curSpecId = 1; i < m_speciesManager.getNumberOfSpecies (); ++ i, ++ curSpecId) {
      Species *curSpec = m_theSpecies[i];
      if(curSpec != NULL) {
         SpeciesProperties * curSpecProps = curSpec->GetSpeciesProperties();

         if(curSpecProps->IsReactive()){
            if(curSpecId == m_speciesManager.getC15plusAroId () ||
               curSpecId == m_speciesManager.getC15plusAroSId ()) {
               curSpecProps->SetB0radical(GetSpeciesB0ByName(m_speciesManager.getC5Id ()));
            } else if(curSpecId == m_speciesManager.getC15plusSatId () || 
                      curSpecId == m_speciesManager.getC15plusSatSId () || 
                      curSpecId == m_speciesManager.getC6to14AroId () ||
                      curSpecId == m_speciesManager.getC6to14BTId () || 
                      curSpecId == m_speciesManager.getC6to14DBTId () || 
                      curSpecId == m_speciesManager.getC6to14BPId () ||
                      curSpecId == m_speciesManager.getLSCId ()) {
               curSpecProps->SetB0radical(GetSpeciesB0ByName(m_speciesManager.getC6to14SatId ()));
            } else if(curSpecId == m_speciesManager.getC6to14SatId () || 
                      curSpecId == m_speciesManager.getC5Id () || 
                      curSpecId == m_speciesManager.getC4Id () ||
                      curSpecId == m_speciesManager.getC15plusATId () ||
                      curSpecId == m_speciesManager.getC6to14AroSId () ||
                      curSpecId == m_speciesManager.getC6to14SatSId ()) {
               curSpecProps->SetB0radical(GetSpeciesB0ByName(m_speciesManager.getC3Id ()));
            } else if(curSpecId == m_speciesManager.getC3Id ()) {
               curSpecProps->SetB0radical(GetSpeciesB0ByName(m_speciesManager.getC2Id ()) * C2FineTuningFactor);
            } else if(curSpecId == m_speciesManager.getC2Id ()) {
               curSpecProps->SetB0radical(GetSpeciesB0ByName(m_speciesManager.getC2Id ()));
            } else {
               curSpecProps->SetB0radical(0.0);
            }
         }
      }
   }
}

}//end of namespace Genex6
