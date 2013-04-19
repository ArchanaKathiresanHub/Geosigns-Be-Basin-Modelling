#include "Species.h"
#include "ChemicalModel.h"
#include "Constants.h"
#include "SpeciesResult.h"
#include "SimulatorState.h"
#include "SpeciesProperties.h"
#include "SpeciesState.h"

#include <math.h>

#include "GenexResultManager.h"
#include "GeneralParametersHandler.h"

namespace Genex5
{
double Species::s_ExmTot = 0.0;
double Species::s_OilExpelledMassInst = 0.0;
double Species::s_OilExpelledVolumeInst = 0.0;
double Species::s_HcGasExpelledVolumeInst = 0.0;
double Species::s_WetGasExpelledVolumeInst = 0.0;
double Species::s_C614SatPlusAromExpVolInst = 0.0;
double Species::s_AromaticsExpelledVolumeInst = 0.0;
double Species::s_SaturatesExpelledVolumeInst = 0.0;
std::map<int, double> Species::s_GroupResults;

double Species::GetExmTot()
{
   return s_ExmTot;
}
double Species::GetOilExpelledMassInst()
{
   return s_OilExpelledMassInst;
}
double Species::GetOilExpelledVolumeInst()
{
   return s_OilExpelledVolumeInst;
}
double Species::GetHcGasExpelledVolumeInst()
{
   return s_HcGasExpelledVolumeInst;
}
double Species::GetWetGasExpelledVolumeInst()
{
   return s_WetGasExpelledVolumeInst;
}
double Species::GetC614SatPlusAromExpVolInst()
{
   return s_C614SatPlusAromExpVolInst;
}
double Species::GetAromaticsExpelledVolumeInst()
{
   return s_AromaticsExpelledVolumeInst;
}
double Species::GetSaturatesExpelledVolumeInst()
{
   return s_SaturatesExpelledVolumeInst;
}
void Species::SetSpeciesTimeStepVariablesToZero()
{
   s_ExmTot                        = 0.0;
   s_OilExpelledMassInst           = 0.0;
   s_OilExpelledVolumeInst         = 0.0;
   s_HcGasExpelledVolumeInst       = 0.0;
   s_WetGasExpelledVolumeInst      = 0.0;
   s_C614SatPlusAromExpVolInst     = 0.0;
   s_AromaticsExpelledVolumeInst   = 0.0;
   s_SaturatesExpelledVolumeInst   = 0.0;

   if(!s_GroupResults.empty())
   {
      std::map<int, double>::iterator it;
      for(it = s_GroupResults.begin(); it != s_GroupResults.end(); ++it)
      {
         it->second = 0.0;
      }
   }
   else
   {
      using namespace CBMGenerics;
      GenexResultManager & theResultManager = GenexResultManager::getInstance();
      
      int i;
      for(i = GenexResultManager::OilGeneratedCum; i < GenexResultManager::NumberOfResults; ++i )
      {
         if(theResultManager.IsResultRequired( i ))
         {
            s_GroupResults[i] = 0.0;
         }
      }
   }
}
Species::~Species()
{
   delete m_theProps;

   if(!s_GroupResults.empty())
   {
      s_GroupResults.clear();
   }
}
Species::Species(const std::string &in_Name, const int &in_id, ChemicalModel *const in_theModel): m_theChemicalModel(in_theModel)
{
   m_name = in_Name;
   m_id = in_id;
   m_positiveGenRate = 0.0;
   m_outputResults = true; 
}

double Species::GetMolWeight() const
{
   return (m_theProps->GetMolWeight());
}
double Species::GetDensity() const
{
   return (m_theProps->GetDensity());
}
double Species::GetAromaticity() const
{
   return (m_theProps->GetAromaticity());
}
double Species::GetMassFactorBySpecies(const std::string &productName) const
{
   double ret = 0.0;
   std::map<std::string,double>::const_iterator it = m_massFactorsBySpecies.find(productName);
   if(it != m_massFactorsBySpecies.end())
   {
      ret = it->second;
   }
   return ret;
}
void Species::UpdateCompositionByElement(Element *const theElem,const double &inComposition)
{
   std::string elemName = theElem->GetName();

   std::map<std::string,double>::iterator it = m_compositionByElement.find(elemName);
   if(it == m_compositionByElement.end())//if does not exist add
   {
      m_theElements[elemName] = theElem;
      m_compositionByElement[elemName] = inComposition;
      m_compositionCode += elemName;
   }
   else//it exists and the value is updated and the composition code is regenerated
   {
      it->second = inComposition;
   }
}
void Species::UpdateMassFactorBySpeciesName(const std::string &SpeciesName, const double &Factor)
{
   std::map<std::string,double>::iterator it = m_massFactorsBySpecies.find(SpeciesName);
   if(it==m_massFactorsBySpecies.end())
   {
      m_massFactorsBySpecies[SpeciesName] = Factor;
   }
   else//it exists and the value is updated 
   {
      it->second = Factor;
   }
}
void Species::OutputMassFactorsOnScreen() const
{
   if(!m_massFactorsBySpecies.empty())
   {
      std::map<std::string,double>::const_iterator it;
      cout<<m_name;
      for(it = m_massFactorsBySpecies.begin();it != m_massFactorsBySpecies.end();++it)
      {
         cout<<","<<it->first;
      }
      cout<<endl;
      cout<<1.0;
      for(it = m_massFactorsBySpecies.begin(); it != m_massFactorsBySpecies.end(); ++it)
      {
         cout<<","<<it->second;
      }
      cout<<endl;
   }
}
double Species::ComputeMolWeight() const//need to opt
{
   double MolWeight = 0.0;
   std::map<std::string,Element*>::const_iterator it = m_theElements.begin();
   while(it != m_theElements.end())
   {
      Element *const theElem = it->second;
      const double elemCompositionFactor = GetCompositionByElement(theElem->GetName());
      MolWeight += elemCompositionFactor * theElem->GetAtomWeight();
      ++it;
   }
   return MolWeight;
}
double Species::ComputeAromaticity() const//need to opt
{
   //H/C roughly corrected for O-groups, Van Krevelen, Table XVI,4, p.321
   double theAromaticity=0.0;    
   const double x = 6.9;
   const double A = 5.2;

   double B=0;
   double C=0;
   //aromaticity of species

   //a mobile non-hydrocarbon species
   //If IsMobile(L) And Not (IsHC(L)) Then
   //    FunArom = 0!

   if(m_theProps->IsMobile() && !m_theProps->IsHC())
   {
      theAromaticity=0.0;
   }
   //h/c roughly corrected for o-groups, Van Krevelen, Table XVI,4, p.321
   else
   {
      const double HC_Corr = this->ComputeHCCorrector();
      //quadratic for fa given HCCORR
      //my correl of Van Krevelen's model params for coal, Table XVI,6, p.323
      B = -(x + A * (2.0 - HC_Corr));
      C = x * (2.0 - HC_Corr) - 1.0;
      theAromaticity = (-B - sqrt(pow(B,2)- 4.0 * A * C)) / (2.0 * A);
      //theAromaticity=0.8;
   }
   if(theAromaticity > 1.0)
   {
      theAromaticity = 1.0;
   }
   if (theAromaticity < 0.0)
   {
      theAromaticity = 0.0;
   }
   if( m_name == "preasphalt")
   {
      GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
      static double PreasphalteneAromMin = theHandler.GetParameterById(GeneralParametersHandler::PreasphalteneAromMin);
      static double PreasphalteneAromMax = theHandler.GetParameterById(GeneralParametersHandler::PreasphalteneAromMax);

      if( theAromaticity < PreasphalteneAromMin)
      {
          theAromaticity = PreasphalteneAromMin;
      }
      if( theAromaticity > PreasphalteneAromMax)
      {
          theAromaticity =  PreasphalteneAromMax;
      }
   }
   return theAromaticity;
}
double Species::ComputeHCCorrector() const
{
   double AtomH = GetCompositionByElement("H");
   double AtomO = GetCompositionByElement("O");
   double HC_Corr = AtomH + AtomO * Genex5::Constants::VAN_KREVELEN_HC_CORRECTOR;

   return  HC_Corr;
}
double Species::ComputeReactionOrder() const
{
   //double newReactionOrder=(OrderPerHoverC * HC + Order0);
   //Update the prop Before returning the value...
   //m_theProps->SetReactionOrder(newReactionOrder);

   const double HC = GetCompositionByElement("H");

   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
   static double OrderPerHoverC = theHandler.GetParameterById(GeneralParametersHandler::OrderPerHoverC);
   static double Order0         = theHandler.GetParameterById(GeneralParametersHandler::Order0);

   return (OrderPerHoverC * HC +Order0);
}
void Species::UpdateProperties()
{
   m_theProps->Update();
}
double Species::ComputeB0() const
{
   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
   static double Tlab          = theHandler.GetParameterById(GeneralParametersHandler::Tlab);
   static double Uj            = theHandler.GetParameterById(GeneralParametersHandler::Uj);
   static double T0torbanite   = theHandler.GetParameterById(GeneralParametersHandler::T0torbanite);

   double ret=0.0;
   const double diffusionEnergy1 = m_theProps->GetDiffusionEnergy1();
   //FunB0 = (EdiffApparent -                    R *    Tlab - Uj) *     (1   - T0torbanite / Tlab) ^ 2
   ret    =  (diffusionEnergy1 - Genex5::Constants::R * Tlab - Uj) * pow((1.0 - T0torbanite / Tlab),2.0);
   if (ret < 0.0)
   {
      ret = 0.0;
   }
   return ret;
}
double Species::ComputeDensity() const
{
   double retDensity = 0.0;
   //H/C roughly corrected for O-groups, Van Krevelen, Table XVI,4, p.321
   //HCcorr = FunHCcorr(L)
   double HCcorr = this->ComputeHCCorrector();
   //ring condensation index given fa and HCcorr, Van Krevelen, eqn, XVI,10, p.322
   //    Rdash = 2! - Arom(L) - HCcorr
   //    If (Rdash < 0!) Then
   //        Rdash = 0!
   //    End If
   double Rdash = (2.0 - this->ComputeAromaticity() - HCcorr);       //rearrange computation in order to avoid this, use Get, Set instead
   if(Rdash < 0.0)
   {
      Rdash = 0.0;
   }
   //approx R/C, Van Krevelen, p.322
   //Rc = Rdash / 2!
   double Rc = Rdash * 0.5;

  
   //volume of rings, Van Krevelen and Chermin '54, VK eqn. XVI,4, p.317
   //VolRing = VolRing1 + VolRing2 * Atom(IatomH, L)
   double AtomH = GetCompositionByElement("H");
   double VolRing = Genex5::Constants::VolRing1+
                    Genex5::Constants::VolRing2 * AtomH;
   
   //FormVol = FormVol1 + FormVol2 * Atom(IatomH, L) + FormVol3 * Atom(IatomO, L) - VolRing * Rc
   double AtomO = GetCompositionByElement("O");
   double FormVol = Genex5::Constants::FormVol1         +
                    Genex5::Constants::FormVol2 * AtomH +
                    Genex5::Constants::FormVol3 * AtomO - VolRing * Rc;

   //density
   //FunDensImmobOM = FW(L) / FormVol * 1000!
   retDensity = this->ComputeMolWeight()/ FormVol * 1000.0;
   return retDensity;
}
double Species::GetCompositionByElement(const std::string &elemName)  const
{
   double ret = 0;
   std::map<std::string,double>::const_iterator it = m_compositionByElement.find(elemName);
   if(it != m_compositionByElement.end())
   {
      ret = it->second;
   }
   return ret;  
}
void Species::OutputCompositionOnScreen() const
{
   //name, composition,composition factors,properties
   cout<<m_name<<","<<m_compositionCode;
   std::string::size_type i=0;
   while(i != m_compositionCode.size())
   {
      std::string key(1,m_compositionCode[i]);
      std::map<std::string,double>::const_iterator it = m_compositionByElement.find(key);
      cout<<","<<it->second;
      i++;
   }
   cout<<endl;
}
void Species::OutputCompositionOnFile(ofstream &outfile) const
{
    std::string::size_type i=0;

    std::string newCompositionCode;
    std::string::size_type pos;

    std::string key_element;
    while(i!=m_compositionCode.size())
    {
      switch(i)
      {
        case 0:
          key_element="C";
        break;
        case 1:
          key_element="H";
        break;
        case 2:
          key_element="O";
        break;
        case 3:
          key_element="N";
        break;
        case 4:
          key_element="S";
        break;
        default:
          key_element="C";
        break;
      }
      pos = m_compositionCode.find(key_element);
      if(pos!=std::string::npos)
      {
         newCompositionCode += key_element;
      }
      i++;
    }
   //name, composition,composition factors,properties
   //outfile<<m_name<<","<<m_compositionCode;
   outfile<<m_name<<","<<newCompositionCode;

    i=0;
    while(i!=5)
    {
	switch(i)
	{
          case 0:
            key_element="C";
          break;
          case 1:
            key_element="H";
          break;
          case 2:
            key_element="O";
          break;
          case 3:
            key_element="N";
          break;
          case 4:
            key_element="S";
          break;
          default:
            key_element="C";
          break;
	}
	double zero = 0.0;
	pos = m_compositionCode.find(key_element);
	std::map<std::string,double>::const_iterator it;
	it = m_compositionByElement.find(key_element);
	if(it != m_compositionByElement.end())
	{
           outfile<<","<<it->second;
	}
	else
	{
           outfile<<","<<zero;
	}
	i++;
    }
   outfile<<endl;
}
void Species::OutputMassFactorsOnFile(ofstream &outfile) const
{
   if(!m_massFactorsBySpecies.empty())
   {
      std::map<std::string,double>::const_iterator it;
      outfile<<m_name;
      for(it = m_massFactorsBySpecies.begin();it!=m_massFactorsBySpecies.end();++it)
      {
         outfile<<","<<it->first;
      }
      outfile<<endl;
      outfile<<1.0;
      for(it = m_massFactorsBySpecies.begin();it!=m_massFactorsBySpecies.end();++it)
      {
         outfile<<","<<it->second;
      }
      outfile<<endl;
   }
}
void Species::OutputPropertiesOnScreen() const
{
   //name,properties
   cout<<m_name;
   m_theProps->OutputOnScreen();
}
void Species::OutputPropertiesOnFile(ofstream &outfile) const
{
   //name,properties
   outfile<<m_name;
   m_theProps->OutputOnFile(outfile);
}

//Sub ArrhReact2(ByVal K As Integer, ByVal FreqF As Single, ByVal Pl As Single, _
//                  ByVal TK As Single, ByVal Tr As Single, ByRef ArrhK As Single)
double Species::ComputeArrheniusReactionRate(const SimulatorState &theSimulatorState, const double &s_FrequencyFactor, const double &s_Peff, const double &s_TK,   const double &s_kerogenTransformationRatio)
{
   //new changes of activation entropy with coking, seems to much improve fit to lab data
   //Const dSperCoke As Single = -150

   //Dim ActU As Single
   //'activation energy as a function of kerogen conversion
   //ActU = Act(1, K) * (1 - Tr) + Act(2, K) * Tr
   double activationEnergy1 = m_theProps->GetActivationEnergy1();
   double activationEnergy2 = m_theProps->GetActivationEnergy2();
   double ActU = activationEnergy1*(1.0 - s_kerogenTransformationRatio) + activationEnergy2 * s_kerogenTransformationRatio;

   double entropy = m_theProps->GetEntropy();  //Act(3, K)
   double volume = m_theProps->GetVolume();  //Act(4, K)
   double Coke2Concentration = theSimulatorState.GetSpeciesConcentrationByName("coke2");

   double ArrheniusReactionRate=0.0;

   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
   static double dSperCoke          = theHandler.GetParameterById(GeneralParametersHandler::dSperCoke);
 
   //modify activation entropy
   // If K = Lasphaltene Or K = Lresin Or K = Lc15plusAro Or K = Lc6to14Aro Then
   if(m_name == "asphaltenes" || m_name == "resins" || m_name == "C15+Aro" || m_name == "C6-14Aro")
   {
        //ArrhK =                FreqF             * Exp( (- ( ActU + Pl     * Act(4, K) ) / TK   + Act(3, K) + dSperCoke                      * ( Conc(Lcoke2)       )) / R                   )
        ArrheniusReactionRate = s_FrequencyFactor   * exp( (- ( ActU + s_Peff * volume    ) / s_TK + entropy   + dSperCoke * ( Coke2Concentration )) / Genex5::Constants::R);
   }
   else
   {
        //ArrhK = FreqF *                          Exp( (-(ActU + Pl     * Act(4, K) ) / TK   + Act(3, K)) / R)
        ArrheniusReactionRate = s_FrequencyFactor * exp( (-(ActU + s_Peff * volume    ) / s_TK + entropy  ) / Genex5::Constants::R);
   }
   return ArrheniusReactionRate;
 }
//Function FunHybrid4(ByVal L As Integer, ByVal FreqF As Single, ByVal Pl As Single, B0 As Single, _
//                ByVal TK As Single, ByVal T0 As Single, ByVal Beta As Single) As Single
double Species::Hybrid4(const double &s_FrequencyFactor, const double &s_Peff, const double &s_TK,  const double &s_VogelFulcherTemperature)
{
    
   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
   static double BetaOverAlpha = theHandler.GetParameterById(GeneralParametersHandler::BetaOverAlpha);
   static double Uj            = theHandler.GetParameterById(GeneralParametersHandler::Uj);

   double B0 = m_theProps->GetB0();
   
   //T1 = T0 + Pl * BetaOverAlpha
   double T1 = s_VogelFulcherTemperature + s_Peff * BetaOverAlpha;
   double jumplength = m_theProps->GetJumpLength(); 

   //Const BetaOverAlpha As Single = 0.00000056  'Dlubek et al 2004
   //implies beta about half old number with AlphaWLF unchanged
   //old beta/alphaWLF  = 5e-10/4.8e-4 = 1.041667e-6, whereas new beta/alpha is 5e-7
   //New value of this ratio from recent polymer literature, e.g. Dlubek et al 2004
   //activation entropy and volume for diffusive jump do not seem to be essential in hybrid model, hence are omitted
   //instead, thermal expansion and expansivity effect the free volume
   //Act(10, L) is the jump distance for species L (typically 6 Angstrom)

   

   double returnValue = 0.0;
   if (T1 < s_TK)
   {
      //Hybrid4   = FreqF             *     Act(10, L) ^ 2   * Exp(-(Uj / TK   + B0 / (TK   - T1)) / R)
      returnValue = s_FrequencyFactor * pow(jumplength, 2.0) * exp(-(Uj / s_TK + B0 / (s_TK - T1)) / Genex5::Constants::R);
   }
   else
   {
        //FunHybrid4 = FreqF             *     Act(10, L) ^ 2   * Exp(-(Uj / (R                    * TK  )))
        returnValue  = s_FrequencyFactor * pow(jumplength, 2.0) * exp(-(Uj / (Genex5::Constants::R * s_TK)));
    }
    return returnValue;
}
void Species::ComputeTimeStep(SimulatorState &theSimulatorState,
                              SpeciesResult &theSpeciesResult,
                              const double &thickness, 
                              const double &ConcKi,
                              const double &in_dT,
                              const double &s_Peff,
                              const double &s_TK,
                              const double &s_FrequencyFactor,
                              const double &s_kerogenTransformationRatio,
                              const double &s_DiffusionConcDependence,
                              const double &s_VogelFulcherTemperature,
                              const bool &in_OpenSourceRockConditions)
{
   double concentration = 0.0;
   double expelledMass  = 0.0;
   
   SpeciesState *currentSpeciesState = 0;
   currentSpeciesState = theSimulatorState.GetSpeciesStateByName(m_name);
   
   if(currentSpeciesState)
   {
      concentration = currentSpeciesState->GetConcentration();
      expelledMass = currentSpeciesState->GetExpelledMass();
   }
   
   double theta = 0.0;
   if(m_theProps->IsMobile())
   {
      if(in_OpenSourceRockConditions)
      {
         GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
         static double BiotOverL2 = theHandler.GetParameterById(GeneralParametersHandler::BiotOverL2);
	 
         //effective diffusion coeff
         //Deff = DiffusionConcDependence * FunHybrid4(L, FreqF, Peff(J), B0(L), TK, T0, Beta)        'hybrid FV-transition state
         double Hybrid = Hybrid4(s_FrequencyFactor, s_Peff, s_TK,  s_VogelFulcherTemperature);
         double effectiveDiffusionCoeff = s_DiffusionConcDependence *  Hybrid ;
	 
         //Theta(L) = 4! * BiotOverL2 * Deff
         theta = 4.0 * BiotOverL2 * effectiveDiffusionCoeff;
     }
   }
   if(m_theProps->IsReactive())
   {
      //Arrhenius rate "constant" of reaction/reactant L
      double ArrheniusReactionRate = ComputeArrheniusReactionRate(theSimulatorState, s_FrequencyFactor, s_Peff, s_TK,  s_kerogenTransformationRatio);
      
      //semi-implicit concentrations (original algorithm)
      //Conc(L) = (Conc(L) + RateGenPos(L) * dT) / (1 + (Theta(L) + ArrhK(L) * Conc(L) ^ (Order(L) - 1)) * dT)
      double reactionOrder = m_theProps->GetReactionOrder();

      //m_concentration = (m_concentration + m_positiveGenRate * in_dT) / (1.0 + (m_theta+ ArrheniusReactionRate * Conc(L) ^ (Order(L) - 1)) * dT) 
      concentration = (concentration + m_positiveGenRate * in_dT)/(1.0 + (theta + ArrheniusReactionRate *pow( concentration, (reactionOrder - 1.0)) ) * in_dT);

      //-ve generation rate, i.e. rate of cracking, of reactant L
      //array for RateGenNeg is only necessary for plotting etc; but the array is clearer (because it emphasises different indices of RateGenPos & RateGenNeg)
      //there is no apparent speed overhead in using an array
      //RateGenNeg(L) = -ArrhK(L) * Conc(L) ^ Order(L)
      double NegativeGenerationRate = - ArrheniusReactionRate * pow(concentration, reactionOrder);
      
      this->UpdatePositiveGenerationRatesOfDaughters(NegativeGenerationRate);
   }
   else
   {
      //semi-implicit concentrations (original algorithm)
      //Conc(L) = (Conc(L) + RateGenPos(L) * dT) / (1 + Theta(L) * dT)
      concentration = (concentration + m_positiveGenRate * in_dT) / (1.0 + theta * in_dT);
   }
   //Compute flux
   //Flx(J, L) = Theta(L) * Conc(L) * SRthicki * ConcKi
   
   double generatedRate = m_positiveGenRate * thickness * ConcKi;
   double flux = theta * concentration * thickness * ConcKi;
   double MassExpelledInst = (flux * in_dT);
   double VolumeExpelledInst = MassExpelledInst / m_theProps->GetDensity();
   
   //Compute cummulative mass
   //-----cumulative expelled masses until current time
   //-----ExM(J, L) = ExM(J - 1, L) + Flx(J, L) * dT
   expelledMass += MassExpelledInst;

   //Update the Species State
   currentSpeciesState->SetConcentration(concentration);
   currentSpeciesState->SetExpelledMass(expelledMass);
   currentSpeciesState->setExpelledMassTransient ( MassExpelledInst );


   double Oil = 0.0;
   double AromaticOM = 0.0;//initialization, final value only for "preasphalt"
   if(m_theProps->IsOil())
   {
      Oil = concentration;
   }
   else if(m_name == "kerogen")
   {
      AromaticOM = concentration * m_theProps->GetAromaticity();
   }
   else if(m_name == "preasphalt")
   {
      AromaticOM = 0.0;
   }
   else
   {
     AromaticOM = concentration;
   }
   theSimulatorState.UpdateLumpedConcentrations(concentration, Oil, AromaticOM);

   //Set Species Result
   theSpeciesResult.SetConcentration(concentration);
   theSpeciesResult.SetExpelledMass(expelledMass);
   theSpeciesResult.SetFlux(flux);
   theSpeciesResult.SetGeneratedRate(m_positiveGenRate);

   //Compute the static variables that will be used for the computation of further SourceRockNodeOutput quantities
   //'''''total expelled masses and effective diffusivities
   //ExmTot = ExmTot + ExM(J, L)

   s_ExmTot += expelledMass;

   using namespace CBMGenerics;
   if(
      m_theProps->IsOil() 
     //2007.093 addition for Volker/Ozkan
     // && (m_name == "C15+Aro" || m_name == "C15+Sat" || m_name == "C6-14Aro"|| m_name == "C6-14Sat")
     //2007.091 addition for Volker/Ozkan))
     )
      
   {
      s_OilExpelledMassInst   += MassExpelledInst;
      s_OilExpelledVolumeInst += VolumeExpelledInst;

      s_GroupResults[GenexResultManager::OilGeneratedCum]     += ( (concentration * thickness * ConcKi) + expelledMass) ;
      s_GroupResults[GenexResultManager::OilGeneratedRate]    += generatedRate;
      s_GroupResults[GenexResultManager::OilExpelledCum]      += expelledMass;
      s_GroupResults[GenexResultManager::OilExpelledRate]     += flux;

      //should use a criterio here, tbd IsC614SaturatesAndAromatics, IsAromatics, IsSaturates
      if(m_name == "C6-14Sat" || m_name == "C6-14Aro")
      {
	      s_C614SatPlusAromExpVolInst += VolumeExpelledInst;
      }
      if(m_name == "C6-14Aro" || m_name == "C15+Aro")
      {
	      s_AromaticsExpelledVolumeInst += VolumeExpelledInst;
      }
      if(m_name == "C6-14Sat" || m_name == "C15+Sat")
      {
	      s_SaturatesExpelledVolumeInst += VolumeExpelledInst;
      }
   }
    //should use a criterio here, tbd IsWetGas, IsC614SaturatesAndAromatics, IsAromatics, IsSaturates
   if(m_theProps->IsHCgas())
   {
      s_HcGasExpelledVolumeInst += VolumeExpelledInst;

      s_GroupResults[GenexResultManager::HcGasGeneratedCum]     += ((concentration * thickness * ConcKi) + expelledMass);
      s_GroupResults[GenexResultManager::HcGasGeneratedRate]    += generatedRate;
      s_GroupResults[GenexResultManager::HcGasExpelledCum]      += expelledMass;
      s_GroupResults[GenexResultManager::HcGasExpelledRate]     += flux;

      //wet gas
      if( m_name == "C5" ||  m_name == "C4" || m_name == "C3" || m_name == "C2")
      {
         s_WetGasExpelledVolumeInst += VolumeExpelledInst; 

         s_GroupResults[GenexResultManager::WetGasGeneratedCum]     += ((concentration * thickness * ConcKi) + expelledMass);
         s_GroupResults[GenexResultManager::WetGasGeneratedRate]    += generatedRate;
         s_GroupResults[GenexResultManager::WetGasExpelledCum]      += expelledMass;
         s_GroupResults[GenexResultManager::WetGasExpelledRate]     += flux;
      } 
      //dry gas
      else
      {
         s_GroupResults[GenexResultManager::DryGasGeneratedCum]     += ((concentration * thickness * ConcKi) + expelledMass);
         s_GroupResults[GenexResultManager::DryGasGeneratedRate]    += generatedRate;
         s_GroupResults[GenexResultManager::DryGasExpelledCum]      += expelledMass;
         s_GroupResults[GenexResultManager::DryGasExpelledRate]     += flux;
      }
   } 
}
double Species::GetGroupResult(const int &theId)
{
   std::map<int,double>::const_iterator resIt =  s_GroupResults.find(theId);
   double Value = Genex5::Constants::s_undefinedValue;
   if(resIt != s_GroupResults.end() )
   {
      Value = resIt->second;
   }
   return Value;
}
bool Species::validate() const
{
     bool status = true;
      
     for(std::map<std::string,double>::const_iterator it = m_massFactorsBySpecies.begin(), itEnd =  m_massFactorsBySpecies.end();
      it != itEnd; ++it)
     {
        if(it->second < 0.0)
        {
           status = false;
           break;
        }
     } 
    return status;
}
void Species::UpdatePositiveGenerationRatesOfDaughters(const double &NegativeGenerationRate)
{
   //+ve generation rate of each daughter (product) species Lp from parent reactant L
   //n.b. *Start loop from 1 instead of L + 1 if any Lp < L,  i.e For Lp = 1 To Ln*
   //The only merit of starting from L + 1 is speed, and that has some limited merit!
   //For Lp = L + 1 To Ln
   //   RateGenPos(Lp) = RateGenPos(Lp) - SFmass(Lp, L) * RateGenNeg(L)
   //Next Lp

   if(m_massFactorsBySpecies.empty())
   {
      return;
   }
   
   int numberOfSpecies = m_theChemicalModel->GetNumberOfSpecies();
   int id=0;
   for(id = m_id+1; id <= numberOfSpecies; id++)
   {
      std::map<std::string,double>::const_iterator it = m_massFactorsBySpecies.find(m_theChemicalModel->GetSpeciesNameById(id));
      if(it != m_massFactorsBySpecies.end())
      {
         double daughterGenerationRateToAdd = ((it->second) *  NegativeGenerationRate);
         m_theChemicalModel->UpdateSpeciesPositiveGenRateByName(it->first, daughterGenerationRateToAdd);
      }
   }
}
void Species::PrintBenchmarkProperties(ofstream &outfile,const int &id) const
{
/*
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
         <<"Ed1"<<","
         <<"B0"<<","
         <<"IsMobile"<<","
         <<"IsReactive"<<","
         <<"IsHC"<<","
         <<"IsOil"<<","
         <<"IsHCgas"<<std::endl;
*/
   outfile  <<id<<","
            <<m_name<<","
            <<GetCompositionByElement("C")<<","
            <<GetCompositionByElement("H")<<","
            <<GetCompositionByElement("O")<<","
            <<GetCompositionByElement("N")<<",";

  m_theProps->PrintBenchmarkProperties(outfile);
}
void Species::PrintBenchmarkStoichiometry(ofstream &outfile) const
{
   int numberOfSpecies=m_theChemicalModel->GetNumberOfSpecies();
   if(!m_massFactorsBySpecies.empty())
   {
      outfile<<m_name<<",";
      for(int id=1;id<=numberOfSpecies;id++)
      {
         Species *Daughter=m_theChemicalModel->GetSpeciesById(id);
         if(Daughter)
         {
            const std::string productName=Daughter->GetName();
            if(productName.empty())
            {
               outfile<< ",";
            }
            else if(productName==m_name)
            {   
               outfile<< "1.0 ,";
            }
            else
            {
               double Factor=GetMassFactorBySpecies(productName);
               outfile<< Factor<<",";
            }
         }
      }
      outfile<<endl;
   } 
}
void Species::UpdateDiffusionEnergy1(const double &in_diffEnergy)
{
   m_theProps->SetDiffusionEnergy1(in_diffEnergy);
}

}//end of Genex5 namespace
