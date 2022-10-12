//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include <stdlib.h>

#include "ReactionRatio.h"
#include "ConstantsGenex.h"
#include "GeneralParametersHandler.h"
#include "Species.h"

using namespace std;

namespace Genex6
{
ReactionRatio::ReactionRatio(Species *const in_react1, Species *const in_react2, 
                             const std::string &functionCode):
m_reactant1(in_react1), m_reactant2(in_react2), m_functionCode(functionCode) 
{

}
void ReactionRatio::OutputOnScreen() 
{
   cout << m_reactant1->GetName() << "," << m_reactant2->GetName() << "," << m_functionCode << endl;
}
void ReactionRatio::OutputOnFile(ofstream &outfile) 
{
   outfile << m_reactant1->GetName() << "," << m_reactant2->GetName() << "," << m_functionCode << endl;
}
double ReactionRatio::GetRatio(const double preasphaltheneArom) const 
{
   double theRatio = 0.0;
   
   if(m_functionCode != "*") { //most likely
      theRatio = atof(m_functionCode.c_str());
   } else {
      GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
      //must use * in config file only for ratio related to aromaticity
      //Rat   = Arom(Lpreasphalt)  / (1 - Arom(Lpreasphalt) ) * AsphalteneRatio
      theRatio = preasphaltheneArom / (1.0 - preasphaltheneArom ) * 
         theHandler.GetParameterById(GeneralParametersHandler::asphalteneRatio);
   }
    //minus( - ) explanation: A/B=RATIO-->A-B*RATIO=0
    theRatio *= -1.0;
    
    return theRatio;
}

};
