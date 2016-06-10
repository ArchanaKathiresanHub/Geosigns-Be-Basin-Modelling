#include "Species.h"
#include "ChemicalModel.h"
#include "Constants.h"
#include "SimulatorStateBase.h"
#include "SpeciesState.h"
#include "SpeciesProperties.h"
#include "Element.h"

#include <math.h>
#include <iomanip>

#include "GenexResultManager.h" 
#include "GeneralParametersHandler.h"

namespace Genex6
{

Species::~Species()
{
   delete m_theProps;
}
Species::Species(const std::string &in_Name, const int in_id, ChemicalModel *const in_theModel):
   m_theChemicalModel(in_theModel)
{
   m_name = in_Name;
   m_id = in_id;
   m_positiveGenRate = 0.0;
   m_outputResults = true; 
   m_theta = 0;
   m_approximate = true;

   m_compositionCodeLength = 0;
   for(int i = 0; i < m_theChemicalModel->getSpeciesManager ().getNumberOfElements (); ++ i) {
      m_compositionCodeIds[i] = -1;
      m_compositionByElement[i] = -1;
      m_theElements[i] = NULL;
   }
   for(int i = 0; i < m_theChemicalModel->getSpeciesManager ().getNumberOfSpecies (); ++ i) {
      m_massFactorsBySpecies[i] = 0.0;
   }
}
bool Species::isGX5() const
{
   return m_theChemicalModel->isGX5();
}

bool Species::isTSR() const
{
   return m_theChemicalModel->isTSR();
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
double Species::GetMassFactorBySpecies(const int productId) const
{
   return m_massFactorsBySpecies[productId - 1];
}

void Species::setMassFactorBySpecies(const int productId, const double in_factor)
{
   m_massFactorsBySpecies[productId - 1] = in_factor;
}

void Species::UpdateCompositionByElement(Element *const theElem, const double inComposition)
{
   int elemId = theElem->GetId();
   if(elemId < 0) return;
   if(m_compositionByElement[elemId] < 0) {
      m_compositionCodeIds[m_compositionCodeLength ++] = elemId;

      m_theElements[elemId] = theElem;
      m_compositionCode += theElem->GetName();  
   }
   m_compositionByElement[elemId] = inComposition;
}

void Species::UpdateMassFactorBySpeciesName(const int SpeciesId, const double Factor)
{
   m_massFactorsBySpecies[SpeciesId-1] = Factor;
}
void Species::OutputMassFactorsOnScreen() const
{
   int i, j;
   cout << m_name;
   
   for(i = 0, j = 1; i < m_theChemicalModel->getSpeciesManager ().getNumberOfSpecies (); ++ i, ++ j) {
      if(m_massFactorsBySpecies[i] > 0.0) {
         cout << ","<< m_theChemicalModel->GetSpeciesNameById(j);
      }
   }
   cout << endl;

   cout << 1.0;
   for(i = 0; i < m_theChemicalModel->getSpeciesManager ().getNumberOfSpecies (); ++ i) {
      if(m_massFactorsBySpecies[i] > 0.0) {
         cout << "," << m_massFactorsBySpecies[i];
      }
   }
   cout << endl;

}
double Species::ComputeMolWeight() const//need to opt
{
   double MolWeight = 0.0;
   for(int i = 0; i < m_compositionCodeLength; ++ i) {
      Element *const theElem = m_theElements[m_compositionCodeIds[i]];
      const double elemCompositionFactor = GetCompositionByElement(theElem->GetId());
      MolWeight += elemCompositionFactor * theElem->GetAtomWeight();
   }
   return MolWeight;
}
double Species::ComputeAromaticity() const//need to opt
{
   //H/C roughly corrected for O-groups, Van Krevelen, Table XVI,4, p.321
   double theAromaticity=0.0;    
   const double x = 6.9;
   const double A = 5.2;

   double B = 0;
   double C = 0;
   //aromaticity of species

   //a mobile non-hydrocarbon species
   //If IsMobile(L) And Not (IsHC(L)) Then
   //    FunArom = 0!

   if(m_theProps->IsMobile() && !m_theProps->IsHC()) {
      theAromaticity = 0.0;
   } else {
      //h/c roughly corrected for o-groups, Van Krevelen, Table XVI,4, p.321
  
      const double HC_Corr = this->ComputeHCCorrector();
      //quadratic for fa given HCCORR
      //my correl of Van Krevelen's model params for coal, Table XVI,6, p.323
      B = -(x + A * (2.0 - HC_Corr));
      C = x * (2.0 - HC_Corr) - 1.0;
      theAromaticity = (-B - sqrt(pow(B, 2)- 4.0 * A * C)) / (2.0 * A);
      //theAromaticity=0.8;
   }
   if(theAromaticity > 1.0) {
      theAromaticity = 1.0;
   }
   if (theAromaticity < 0.0) {
      theAromaticity = 0.0;
   }
   if(m_id == m_theChemicalModel->getSpeciesManager ().getPreasphaltId ()) {
      GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
      static double PreasphalteneAromMin = theHandler.GetParameterById(GeneralParametersHandler::PreasphalteneAromMin);
      static double PreasphalteneAromMax = theHandler.GetParameterById(GeneralParametersHandler::PreasphalteneAromMax);

      if(theAromaticity < PreasphalteneAromMin) {
          theAromaticity = PreasphalteneAromMin;
      }
      if(theAromaticity > PreasphalteneAromMax) {
          theAromaticity =  PreasphalteneAromMax;
      }
   }
   return theAromaticity;
}
double Species::ComputeHCCorrector() const
{
   double AtomH = GetCompositionByElement(m_theChemicalModel->getSpeciesManager ().getHydrogenId ());
   double AtomO = GetCompositionByElement(m_theChemicalModel->getSpeciesManager ().getOxygenId ());
   double HC_Corr = AtomH + AtomO * Genex6::Constants::VAN_KREVELEN_HC_CORRECTOR;

   return  HC_Corr;
}
double Species::ComputeReactionOrder() const
{
   //double newReactionOrder=(OrderPerHoverC * HC + Order0);
   //Update the prop Before returning the value...
   //m_theProps->SetReactionOrder(newReactionOrder);

   const double HC = GetCompositionByElement(m_theChemicalModel->getSpeciesManager ().getHydrogenId ());

   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
   static double OrderPerHoverC = theHandler.GetParameterById(GeneralParametersHandler::OrderPerHoverC);
   static double Order0         = theHandler.GetParameterById(GeneralParametersHandler::Order0);

   return (OrderPerHoverC * HC + Order0);
}
void Species::UpdateProperties()
{
   m_theProps->Update();

   // For compatibility with OTGC-5.
   if( m_theChemicalModel->isOTGC5 () and m_id == m_theChemicalModel->getSpeciesManager ().getKerogenId ())
   {
      m_theProps->SetAromaticity(m_theChemicalModel->GetSpeciesById( m_theChemicalModel->getSpeciesManager ().getPreasphaltId ())->ComputeAromaticity());
   }


}
//In OTGC version of the code, the ComputeB0 has changed. 
///On top of it the B0radical has been introduced for the generation part. 
//The B0 is still in use but only in the Diffusion computation: 
//Deff = DiffusionConcDependence * FunDiffusivityHybrid(L, FreqF, Peff(J), B0(L), TK, T0)
void Species::ComputeB0() const
{
   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
   static double Tlab        = theHandler.GetParameterById(GeneralParametersHandler::Tlab);
   static double Uj          = theHandler.GetParameterById(GeneralParametersHandler::Uj);
   static double T0torbanite = theHandler.GetParameterById(GeneralParametersHandler::T0torbanite);

   const double diffusionEnergy1 = m_theProps->GetDiffusionEnergy1();
   //FunB0 = (EdiffApparent - R * Tlab - Uj) * (1 - T0torbanite / Tlab) ^ 2
   double ret = (diffusionEnergy1 - Genex6::Constants::R * Tlab - Uj) * pow((1.0 - T0torbanite / Tlab), 2.0);
   if (ret < 0.0) {
      ret = 0.0;
   }
   m_theProps->SetB0(ret);
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
   double Rdash = (2.0 - this->ComputeAromaticity() - HCcorr); //rearrange computation in order to avoid this, use Get, Set instead
   if(Rdash < 0.0) {
      Rdash = 0.0;
   }
   //approx R/C, Van Krevelen, p.322
   //Rc = Rdash / 2!
   double Rc = Rdash * 0.5;

   //volume of rings, Van Krevelen and Chermin '54, VK eqn. XVI,4, p.317
   //VolRing = VolRing1 + VolRing2 * Atom(IatomH, L)
   double AtomH = GetCompositionByElement(m_theChemicalModel->getSpeciesManager ().getHydrogenId ());
   double VolRing = Genex6::Constants::VolRing1 + Genex6::Constants::VolRing2 * AtomH;
   
   //FormVol = FormVol1 + FormVol2 * Atom(IatomH, L) + FormVol3 * Atom(IatomO, L) - VolRing * Rc
   double AtomO = GetCompositionByElement(m_theChemicalModel->getSpeciesManager ().getOxygenId ());
   double FormVol = Genex6::Constants::FormVol1         +
                    Genex6::Constants::FormVol2 * AtomH +
                    Genex6::Constants::FormVol3 * AtomO - VolRing * Rc;

   //density
   //FunDensImmobOM = FW(L) / FormVol * 1000!
   retDensity = this->ComputeMolWeight()/ FormVol * 1000.0;

   return retDensity;
}
double Species::GetCompositionByElement(const int elemName)  const
{
   if(elemName < 0 || m_compositionByElement[elemName] < 0) return 0.0;
   return m_compositionByElement[elemName];  
}
void Species::OutputCompositionOnScreen()
{
   //name, composition,composition factors,properties
   cout << m_name << "," << m_compositionCode;

   for(int i = 0; i < m_compositionCodeLength; ++ i) {
      cout << "," <<  m_compositionByElement[m_compositionCodeIds[i]];
   }
   cout << endl;
}
void Species::OutputCompositionOnFile(ofstream &outfile)
{
   //name, composition,composition factors,properties

   outfile << m_name << "," << m_compositionCode;

   const int elements_ids[] = { m_theChemicalModel->getSpeciesManager ().getCarbonId (),
                                m_theChemicalModel->getSpeciesManager ().getHydrogenId (),
                                m_theChemicalModel->getSpeciesManager ().getOxygenId (),
                                m_theChemicalModel->getSpeciesManager ().getNitrogenId (),
                                m_theChemicalModel->getSpeciesManager ().getSulphurId ()};

   double zero = 0.0;
   for(int i = 0; i < m_theChemicalModel->getSpeciesManager ().getNumberOfElements (); ++ i) {
      if(elements_ids[i] >= 0 && m_compositionByElement[elements_ids[i]] > 0) {
         outfile << "," << m_compositionByElement[elements_ids[i]];
      } else {
         outfile << "," << zero;
      }
   }
   outfile << endl;
}
 void Species::OutputMassFactorsOnFile(ofstream &outfile) const
{
   //   if(m_massFactorsBySpeciesNotEmpty) {
      int i, j;
      outfile << m_name;
      for(i = 0, j = 1; i < m_theChemicalModel->getSpeciesManager ().getNumberOfSpecies (); ++ i, ++ j) {
         if(m_massFactorsBySpecies[i] > 0.0) {
           outfile << ","<< m_theChemicalModel->GetSpeciesNameById(j);
         }
      }
      outfile << endl;
      outfile << 1.0;
      for(i = 0; i < m_theChemicalModel->getSpeciesManager ().getNumberOfSpecies (); ++ i) {
         if(m_massFactorsBySpecies[i] > 0.0) {
            cout << ","<< m_massFactorsBySpecies[i];
         }
      }
      outfile << endl;
   //   }
}
void Species::OutputPropertiesOnScreen() const
{
   //name, properties
   cout << m_name;
   m_theProps->OutputOnScreen();
}
void Species::OutputPropertiesOnFile(ofstream &outfile) const
{
   //name, properties
   outfile << m_name;
   m_theProps->OutputOnFile(outfile);
}

/*
Sub ArrhReact2a
*/
double Species::ComputeArrheniusReactionRate2a( SimulatorStateBase &theSimulatorState, 
                                                const double s_FrequencyFactor, 
                                                const double s_Peff, 
                                                const double s_TK,  
                                                const double s_VogelFulcherTemperature,
                                                const double s_kerogenTransformationRatio, 
                                                const double s_precokeTransformationRatio, 
                                                const double s_coke2TransformationRatio )
{
   //new changes of activation entropy with coking, seems to much improve fit to lab data
   //Const dSperCoke As Single = -150

   //Dim ActU As Single
   //'activation energy as a function of kerogen conversion
   //ActU = Act(1, K) * (1 - Tr) + Act(2, K) * Tr
   double activationEnergy1 = m_theProps->GetActivationEnergy1();
   double activationEnergy2 = m_theProps->GetActivationEnergy2();
   double ActU = 0.0;
   
   if(isGX5()) {
        ActU = activationEnergy1 * (1.0 - s_kerogenTransformationRatio) + 
           activationEnergy2 * s_kerogenTransformationRatio;
    } else {
      if(m_theChemicalModel->getSpeciesManager ().getKerogenId () == m_id || m_theChemicalModel->getSpeciesManager ().getPreasphaltId () == m_id) {

         ActU = activationEnergy1 * (1.0 - s_kerogenTransformationRatio) + activationEnergy2 * s_kerogenTransformationRatio;
      } else if(m_theChemicalModel->getSpeciesManager ().getAsphaltenesId () == m_id || m_theChemicalModel->getSpeciesManager ().getResinsId () == m_id) {

         ActU = activationEnergy1 * (1.0 - s_precokeTransformationRatio) + activationEnergy2 * s_precokeTransformationRatio;
      } else {

         ActU = activationEnergy1 * (1.0 - s_coke2TransformationRatio) + activationEnergy2 * s_coke2TransformationRatio;
      }
   }

   double accessibleAN = 0.0;
   const double ccToAn = 0.95008;

   if( isTSR() ) {
      int CO3id = m_theChemicalModel->getSpeciesManager ().getCO3Id ();
      int SO4id = m_theChemicalModel->getSpeciesManager ().getSO4Id ();

      const double molWtCO3overSO4 = 0.625;

      const double val = theSimulatorState.GetSpeciesConcentrationByName( CO3id ) + 
         theSimulatorState.GetSpeciesConcentrationByName( SO4id ) * molWtCO3overSO4;
      if( val != 0.0 ) {
         accessibleAN = ccToAn - theSimulatorState.GetSpeciesConcentrationByName( CO3id ) / val;
      }
      if( accessibleAN < 0.0 ) {
         accessibleAN = 0.0;
      }

      if( m_theProps->IsOil() ) {
         theSimulatorState.incTotalOilForTSR( theSimulatorState.GetSpeciesConcentrationByName ( m_id ) );
      }

   }

   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
   if(!isGX5()) {
      static double BetaOverAlpha = theHandler.GetParameterById(GeneralParametersHandler::BetaOverAlpha);
      
      double T1 = s_VogelFulcherTemperature + s_Peff * BetaOverAlpha;
      if (T1 < s_TK) {
         double B0radical = m_theProps->GetB0radical();  
         //   ActU = ActU - 2.0 * B0radical * s_TK / (s_TK - T1);// / 2.0;
         ActU = ActU - B0radical * s_TK / (s_TK - T1) / 2.0;
      }
   }

   double Coke2Concentration = theSimulatorState.GetSpeciesConcentrationByName(m_theChemicalModel->getSpeciesManager ().getCoke2Id ());
   double entropy = m_theProps->GetEntropy();  //Act(3, K)
   double volume = m_theProps->GetVolume();    //Act(4, K)
   double ArrheniusReactionRate = 0.0;

   static double dSperCoke = theHandler.GetParameterById(GeneralParametersHandler::dSperCoke);
 
   //modify activation entropy
   if(m_id == m_theChemicalModel->getSpeciesManager ().getAsphaltenesId () || m_id == m_theChemicalModel->getSpeciesManager ().getResinsId ()) {
         ArrheniusReactionRate = s_FrequencyFactor * 
            exp((- (ActU + s_Peff * volume) / s_TK + entropy + dSperCoke * (Coke2Concentration)) / Genex6::Constants::R);
   } else if(m_id == m_theChemicalModel->getSpeciesManager ().getC15plusAroId () || m_id == m_theChemicalModel->getSpeciesManager ().getC6to14AroId ()) {
      if(isGX5()){
         ArrheniusReactionRate = s_FrequencyFactor * 
            exp((- (ActU + s_Peff * volume) / s_TK + entropy + dSperCoke * (Coke2Concentration)) / Genex6::Constants::R);
      } else {
         ArrheniusReactionRate = s_FrequencyFactor * 
            exp((-(ActU + s_Peff * volume) / s_TK + entropy) / Genex6::Constants::R);
      }
   } else {
         ArrheniusReactionRate = s_FrequencyFactor * 
            exp((-(ActU + s_Peff * volume) / s_TK + entropy) / Genex6::Constants::R);
   }

   if( isTSR() ) {
      if( GetMassFactorBySpecies( m_theChemicalModel->getSpeciesManager ().getSO4Id ()) < 0 ) {
         ArrheniusReactionRate = ArrheniusReactionRate * accessibleAN / ccToAn;
      }
      if( m_theProps->IsHCgas() and theSimulatorState.getTotalOilForTSR () > 1.0 ) {
         ArrheniusReactionRate = 0.0;
      }
   }

   return ArrheniusReactionRate;
 }
double Species::FunDiffusivityHybrid(const double s_FrequencyFactor, const double s_Peff, 
                                     const double s_TK, const double s_VogelFulcherTemperature)
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
   if (T1 < s_TK) {
      returnValue = s_FrequencyFactor * pow(jumplength, 2.0) * exp(-(Uj / s_TK + B0 / (s_TK - T1)) / Genex6::Constants::R);
   } else {
      returnValue  = s_FrequencyFactor * pow(jumplength, 2.0) * exp(-(Uj / (Genex6::Constants::R * s_TK)));
      cout << "T1>TK in FunDiffusivityHybrid" << endl;
   }
   return returnValue;
}
void Species::ComputeTimeStep(SimulatorStateBase &theSimulatorState,
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
   double concentration = 0.0;
   double concentrationApproximation = 0.0;
   bool   isOTGC_5 = m_theChemicalModel->isOTGC5(); //true;
   int    firstTimeStep = ( isGenex() ? 0 : 1 );
   int    firstTimeStepForUpdate = ( isOTGC_5 ? 0 : firstTimeStep );

   int  currentTimeStep = theSimulatorState.GetTimeStep();
   bool isTSRapproximation = isTSR() and currentTimeStep > firstTimeStep + 1;

   SpeciesState *currentSpeciesState = theSimulatorState.GetSpeciesStateById(m_id);
   if(currentSpeciesState) {
      concentration = currentSpeciesState->GetConcentration();
      if(isGX5()){
         concentrationApproximation = concentration;
      } else if(m_approximate && currentTimeStep > firstTimeStep + 1) {
         concentrationApproximation = currentSpeciesState->GetConcentrationApproximation( isTSRapproximation );
      } 
      else  { 
         concentrationApproximation = concentration;
      }
      if( isTSRapproximation ) {
         concentrationApproximation = currentSpeciesState->GetConcentrationApproximation( isTSRapproximation );
      }
    }
   
   ComputeMassTransportCoeff(s_Peff, s_TK, s_FrequencyFactor, s_DiffusionConcDependence,
                             s_VogelFulcherTemperature, in_OpenSourceRockConditions);   

   if(m_theProps->IsReactive()) {
      double ArrheniusReactionRate = ComputeArrheniusReactionRate2a(theSimulatorState, 
                                                                    s_FrequencyFactor, 
                                                                    s_Peff, 
                                                                    s_TK, 
                                                                    s_VogelFulcherTemperature,  
                                                                    s_kerogenTransformationRatio,
                                                                    s_precokeTransformationRatio, 
                                                                    s_coke2TransformationRatio);
      
      double reactionOrder = m_theProps->GetReactionOrder();

      // Sometimes  at this point concentrationApproximation = 0 but in VBA it is very small negative number.

//      if(m_approximate && (concentrationApproximation < 0) && (currentTimeStep > firstTimeStep + 1) && !isGX5()) { 
      if(m_approximate && (currentTimeStep > firstTimeStep + 1) && !isGX5()) { 
         if( isOTGC_5 ) {
            if( concentrationApproximation < 0 ) {
               concentrationApproximation = 0.0;
               reactionOrder = 1.0;
            }
         } else {
            concentrationApproximation = (concentration + m_positiveGenRate * in_dT) /
               (1.0 + (m_theta + ArrheniusReactionRate * pow(concentration, (reactionOrder - 1.0))) * in_dT);
            //reactionOrder = 1.0;
          }
      }
      if( isTSRapproximation ) {
         if( concentrationApproximation < 0 ) {
            concentrationApproximation = 0.0;
            reactionOrder = 1.0;
         }
      }
      concentration = (concentration + m_positiveGenRate * in_dT) /
         (1.0 + (m_theta + ArrheniusReactionRate * pow(concentrationApproximation, (reactionOrder - 1.0))) * in_dT);

      double NegativeGenerationRate = - ArrheniusReactionRate * pow(concentration, reactionOrder);
      
      UpdatePositiveGenerationRatesOfDaughters(NegativeGenerationRate);
   } else {
      concentration = (concentration + m_positiveGenRate * in_dT) / (1.0 + m_theta * in_dT); 
   }

   //Update the Species State
   if(currentTimeStep > firstTimeStepForUpdate) currentSpeciesState->SetConcentration(concentration);
   else                                         currentSpeciesState->UpdateConcentration(concentration);
}
void Species::ComputeMassTransportCoeff(const double s_Peff,
                                        const double s_TK,
                                        const double s_FrequencyFactor,
                                        const double s_DiffusionConcDependence,
                                        const double s_VogelFulcherTemperature,
                                        const bool in_OpenSourceRockConditions)
{
   m_theta = 0.0;
   
   if(in_OpenSourceRockConditions && m_theProps->IsMobile()) {
      GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
      static double BiotOverL2 = theHandler.GetParameterById(GeneralParametersHandler::BiotOverL2);
      
      //effective diffusion coeff
      //Deff = DiffusionConcDependence * FunDiffusivityHybrid(L, FreqF, Peff(J), B0(L), TK, T0) 'hybrid FV-transition state
      double effectiveDiffusionCoeff = s_DiffusionConcDependence * 
         FunDiffusivityHybrid(s_FrequencyFactor, s_Peff, s_TK,  s_VogelFulcherTemperature) ;
      
      //Theta(L) = 4! * BiotOverL2 * Deff
      m_theta = 4.0 * BiotOverL2 * effectiveDiffusionCoeff;
   }
}

bool Species::validate() 
{
   bool status = true;

   // if(m_id == m_theChemicalModel->getSpeciesManager ().getResinsId()) {
   //    int precokeId =m_theChemicalModel->getSpeciesManager ().getPrecokeId() - 1;
      
   //    if( m_massFactorsBySpecies[precokeId] < 0 ) {
   //       int c15plusSat = m_theChemicalModel->getSpeciesManager ().getC15plusSatId() - 1;
   //       m_massFactorsBySpecies[c15plusSat] +=  m_massFactorsBySpecies[precokeId];
   //    }
   // }
   for(int i = 0; i < m_theChemicalModel->getSpeciesManager ().getNumberOfSpecies (); ++ i) {
      if(m_massFactorsBySpecies[i] < 0.0) {
         if( m_theChemicalModel->isSim5() ) { // in GX6 massFactors can be negative 
            status = false;
            break;
         } else {
            if( !isTSR() ) {
               m_massFactorsBySpecies[i] = 0.0;
            }
        }
      } 
   }
   return status;
}
void Species::UpdatePositiveGenerationRatesOfDaughters(const double NegativeGenerationRate)
{
   //+ve generation rate of each daughter (product) species Lp from parent reactant L
   //n.b. *Start loop from 1 instead of L + 1 if any Lp < L,  i.e For Lp = 1 To Ln*
   //The only merit of starting from L + 1 is speed, and that has some limited merit!
   //For Lp = L + 1 To Ln
   //   RateGenPos(Lp) = RateGenPos(Lp) - SFmass(Lp, L) * RateGenNeg(L)
   //Next Lp

/*
//

	typedef std::map<std::string,double>::const_iterator constIt;
	for(constIt it = m_massFactorsBySpecies.begin(), itEnd = m_massFactorsBySpecies.end(); it !=itEnd; ++it)
	{
   	Species *theProduct = m_theChemicalModel->GetSpeciesByName(it->first);
   	double daughterGenerationRateToAdd = it->second * NegativeGenerationRate;

   	double  positiveGenRate = theProduct->GetPositiveGenRate();
   	positiveGenRate -= daughterGenerationRateToAdd;
   	theProduct->SetPositiveGenRate(positiveGenRate);
	}
//
*/
   int numberOfSpecies = m_theChemicalModel->GetNumberOfSpecies();
   double daughterGenerationRate;
   for (int i = m_id + 1, j = m_id; i <= numberOfSpecies; ++i, ++ j) {
      // If SMass(Lp,p) > 0
      if(m_massFactorsBySpecies[j] != 0.0) {
         daughterGenerationRate = (m_massFactorsBySpecies[j] *  NegativeGenerationRate);
         Species *const theSpecies = m_theChemicalModel->GetSpeciesById(i);
         if(theSpecies) theSpecies->UpdatePositiveGenRate(daughterGenerationRate);
      }
   }
}
const double * Species::GetProductMassFactors() const
{
   return m_massFactorsBySpecies;
}
void Species::PrintBenchmarkProperties(ofstream &outfile) const
{
/*
outfile  << "spec. no." << ","	
         << "name" << ","
         << "C" << ","
         << "H" << ","
         << "O" << ","
         << "N" << ","
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
         << "IsHCgas" << std::endl;
*/
   outfile << m_id << ","
           << m_name << ","
           << GetCompositionByElement(m_theChemicalModel->getSpeciesManager ().getCarbonId ()) << ","
           << GetCompositionByElement(m_theChemicalModel->getSpeciesManager ().getHydrogenId ()) << ","
           << GetCompositionByElement(m_theChemicalModel->getSpeciesManager ().getOxygenId ()) << ","
           << GetCompositionByElement(m_theChemicalModel->getSpeciesManager ().getNitrogenId ()) << ","
           << GetCompositionByElement(m_theChemicalModel->getSpeciesManager ().getSulphurId ()) << ",";

  m_theProps->PrintBenchmarkProperties(outfile);
}
void Species::PrintBenchmarkStoichiometry(ofstream &outfile) const
{
   int i, numberOfSpecies = m_theChemicalModel->GetNumberOfSpecies();

   bool massFactorsBySpeciesNotEmpty = false;

   for(i = 0; i < numberOfSpecies; ++ i) {
      if(m_massFactorsBySpecies[i] != 0.0) {
         massFactorsBySpeciesNotEmpty = true;
         break;
      }
   }
   if(massFactorsBySpeciesNotEmpty) {
      outfile << m_name << ",";
      for( i = 1; i <= numberOfSpecies; ++ i) {
        Species *Daughter = m_theChemicalModel->GetSpeciesById(i);
        if(Daughter) {
          const std::string productName = Daughter->GetName();
          if(productName.empty()) {
             outfile << ",";
          } else if(productName == m_name) {   
             outfile << "1.0 ,";
          } else {
             double Factor = GetMassFactorBySpecies(i);
             outfile << Factor << ",";
           }
        }
      }
      outfile << endl;
   } 
}
#if 0
void Species::LoadStoichiometry(ifstream &infile)
{
   int j, numberOfSpecies = m_theChemicalModel->GetNumberOfSpecies();
   double Factor;

   for(j = 1; j <= numberOfSpecies; ++ j) {
      infile >> Factor;
      if(j != m_id) {
         SetMassFactorBySpecies(j, Factor);
      }
   }
}
#endif
void Species::UpdateDiffusionEnergy1(const double in_diffEnergy)
{
   m_theProps->SetDiffusionEnergy1(in_diffEnergy);
}

}//end of Genex6 namespace
