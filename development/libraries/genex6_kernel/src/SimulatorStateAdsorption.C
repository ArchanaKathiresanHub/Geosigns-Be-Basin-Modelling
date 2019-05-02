#include <math.h>
#include "SimulatorStateAdsorption.h"
#include "ChemicalModel.h"
#include "SpeciesProperties.h"
#include "SpeciesResult.h"
#include "GenexResultManager.h"
#include "ConstantsGenex.h"

#include "EosPack.h"

namespace Genex6
{

   SimulatorStateAdsorption::SimulatorStateAdsorption() {

      clean();
      m_solidVolOnset  = 0.0;
      m_solidVolOnset2 = 0.0;
      m_vreOnset = 0.0;
      m_omPhiMax = 0.0;
      m_solidOmMaxRatio = 0.0;
      m_maxVolCoke1 = 0.0;
      m_maxVolCoke1J = 0;
      m_jonsetOrgPor = 0;
      m_NmaxFree = 0.0;
      m_NmaxAds = 0.0;
      m_maxVolPrecoke = 0.0;
      m_poreD = 10.0;
      
      m_chemicalModel = 0;
      m_simulatorState = 0; // not used??
   }
     
   SimulatorStateAdsorption:: ~SimulatorStateAdsorption() {

      for( unsigned int i = 0; i < m_Results.size(); ++ i ) {
         delete m_Results[i].first;
         delete m_Results[i].second;
      }

      for( unsigned int i = 0; i < m_genexResults.size(); ++ i ) {
         delete [] m_genexResults[i].second;
      }

   }

   void SimulatorStateAdsorption::clean() {
      m_solidVol = 0.0;
      m_poreVol = 0.0;
      m_poreVolp = 0.0;
      m_nPores = 0.0;
      m_organoPorosity = 0.0;
      m_molesGas = 0.0;
      m_molarVolGas = 0.0;
      m_PRa = 0.0;
      m_PRb = 0.0;
      m_gasPressurePR = 0.0;
      m_gasPressureRealGas = 0.0;
      m_gasPressureVDW = 0.0;
      m_phalfLangmuir = 0.0;
      m_phalfPrangmuir = 0.0;
      m_pressurePrangmuir = 0.0;
      m_pressureLangmuir = 0.0;
      m_pressureJGS = 0.0;
      m_pressureVirial = 0.0;
      m_volFractFreeGas = 0.0;
      m_volFractAdsorbGas = 0.0;
      m_volSinglePore = 0.0;
      
      m_adsGas =  0.0;
      m_freeGas =  0.0;

      m_concGas = 0.0;

      m_setSolidVol = false;

      //     m_chemicalModel = 0;
      //     m_simulatorState = 0; // not used??
   }

   SimulatorState * SimulatorStateAdsorption::getSimulatorState() const {
      return m_simulatorState;
   }

   void SimulatorStateAdsorption::setChemicalModel( const ChemicalModel * inModel ) {

      m_chemicalModel = inModel;
   }

   void SimulatorStateAdsorption::setSimulatorState( SimulatorState * inState ) {

      m_simulatorState = inState;
   }
      
   bool SimulatorStateAdsorption::setOrganoporosityVars ( const double inOmPhiMax, const double inPoreD ) {
      
      if( inOmPhiMax <= 0.0 or inOmPhiMax > 40.0 ) {
         // error message
         // target OM porosity is closing in on the total OM volume or 0
         return false;
      }
      m_omPhiMax = inOmPhiMax;
      m_poreD = inPoreD;

      return true;
   }
         
   int SimulatorStateAdsorption::getTimeSteps() const {            
      return m_Results.size();
   }

   void  SimulatorStateAdsorption::cleanResults( ) {
   
      for( unsigned int i = 0; i < m_Results.size(); ++ i ) {
         delete m_Results[i].first;
         delete m_Results[i].second;
      }
      m_Results.clear();

   }
   // calculate onsets in genex calculation
   void SimulatorStateAdsorption::calculateSolidVol( SimulatorState * simulatorState, const double vre ) {

      double prevSolidVol = m_solidVol;
      bool setSolidVolume2 = false;

      m_solidVol = 0.0;
      int coke1Id   = m_chemicalModel->getSpeciesManager().getCoke1Id();
      int precokeId = m_chemicalModel->getSpeciesManager().getPrecokeId();

      // SimulatorState->GetTimeStep() is the next time step

      // also check the onset for organo porosity and max vol coke1

      for( int i = 0, speciesId = 1; i < m_chemicalModel->GetNumberOfSpecies(); ++i, ++ speciesId ) {
         
         Species *currentSpecies =  m_chemicalModel->GetSpeciesById( speciesId );
         if(currentSpecies == NULL) continue;
 
         SpeciesProperties * speciesProps = currentSpecies->GetSpeciesProperties();
         double concentration = simulatorState->GetSpeciesResult(speciesId).GetConcentration();
           
         //solid om (immobile) species
         if(not speciesProps->IsMobile()) {
            //volume of each species at...
            const double speciesVolume = concentration / speciesProps->GetDensity();
            m_solidVol +=  speciesVolume; 

            // find timestep with precoke onset - the onset of OMphi 
            if( currentSpecies->GetId() == precokeId ) {
               if( not m_setSolidVol and concentration > 0.001 ) {
                  m_setSolidVol = true;
                  m_solidVolOnset = prevSolidVol;
                  m_jonsetOrgPor = NumericFunctions::Maximum <int>( simulatorState->GetTimeStep() - 2, 0 ); // previous time step
                  m_vreOnset = vre; // must be vre of the previous time step
               }
               // find maximum volume of precoke (for Unit Test only)
               if( speciesVolume > m_maxVolPrecoke ) {
                  m_maxVolPrecoke = speciesVolume;
                  setSolidVolume2 = true;
               }
               // find timestep with cross-over of precoke and coke1 as OMphi plateau point
               const double coke1conc = simulatorState->GetSpeciesResult(coke1Id).GetConcentration();
               if( coke1conc > 0.0 and m_maxVolCoke1J == 0 and coke1conc > concentration ) {
                  m_maxVolCoke1 = concentration;
                  m_maxVolCoke1J = simulatorState->GetTimeStep(); // timeStep + 1;
               }
               
            }
         }
            
      }
      // for Unit Test only
      if( setSolidVolume2 ) {
         m_solidVolOnset2 = m_solidVol;
      }
   }

   //// unit test methods
   void SimulatorStateAdsorption::calculateAdsorptionTimeStep( int timeStep, oneTimeStepUnit oneTimeStep, const bool output ) {
      
      calculateAdsorptionTimeStep( timeStep, oneTimeStep.first[0], oneTimeStep.first[1], oneTimeStep.second, output );
   }
   // post-process unit test
   bool SimulatorStateAdsorption::postProcessAdsorption( const ChemicalModel * chemModel, SimulatorState * simulatorState, std::vector<Input*> &inputStates ) {

      m_chemicalModel = chemModel;
      m_simulatorState = simulatorState;

      int timeSteps = m_genexResults.size();
      if( timeSteps != inputStates.size() - 1 ) {
         return false;
      }
      calculateSolidOMMaxRatio();
      calculateVolFractions();

      for ( int i = 0; i < timeSteps; ++ i ) {
         calculateAdsorptionTimeStep( i, m_genexResults[i] );
         setAdsorptionState ( inputStates[i + 1] );
      }
      return true;
   }

   // save a genex time step results for the later postprocessing (unitTest)
   void SimulatorStateAdsorption::saveTimeStepResultsForAdsorption( const double temperature, const double vre, const SpeciesResult * results ) {

      std::vector<double> pt (2);
      pt[0] = temperature;
      pt[1] = vre;

      SpeciesResult * oneTS = new SpeciesResult[m_chemicalModel->GetNumberOfSpecies()];
      for( int i = 0; i < m_chemicalModel->GetNumberOfSpecies(); ++ i ) {
         oneTS[i] = results[i];
      }
      m_genexResults.push_back (oneTimeStepUnit( pt, oneTS ));
     

   }

   // set adsorption results for UnitTest output
   void SimulatorStateAdsorption::setAdsorptionState( Input * inputBaseState ) {
      
      OutputUnitTest * inputState = dynamic_cast<OutputUnitTest *>( inputBaseState );

      inputState->m_solidVol = m_solidVol;
      inputState->m_poreVol = m_poreVol;
      inputState->m_poreVolp = m_poreVolp;
      inputState->m_nPores = m_nPores;
      inputState->m_organoPorosity = m_organoPorosity;
      inputState->m_molesGas = m_molesGas;
      inputState->m_molarVolGas = m_molarVolGas;
      inputState->m_PRa = m_PRa ;
      inputState->m_gasPressurePR = m_gasPressurePR; 
      inputState->m_gasPressureRealGas = m_gasPressureRealGas;
      inputState->m_gasPressureVDW = m_gasPressureVDW;
      inputState->m_phalfLangmuir = m_phalfLangmuir;
      inputState->m_phalfPrangmuir = m_phalfPrangmuir;
      inputState->m_pressurePrangmuir = m_pressurePrangmuir;
      inputState->m_pressureLangmuir = m_pressureLangmuir;
      inputState->m_pressureJGS = m_pressureJGS;
      inputState->m_pressureVirial = m_pressureVirial;
      
   }
 
   // Compute adsorption in fastgenex6
   void SimulatorStateAdsorption::calculateAdsorptionTimeStep( int timeStep ) {

      if( timeStep == 0.0 ) {
         clean();
         calculateSolidOMMaxRatio();
         calculateVolFractions(); 
      }
      oneTimeStep oneTimeStep = getOneTimeStepResult( timeStep );

      calculateAdsorptionTimeStep( timeStep, oneTimeStep.second->GetTemperatureCelsius(), oneTimeStep.second->getVre(), 
                                   oneTimeStep.first->getSpeciesResults(), false );
   }

   oneTimeStep & SimulatorStateAdsorption::getOneTimeStepResult ( int timeStep ) {

      return m_Results [timeStep];
   }
   void SimulatorStateAdsorption::removeOneTimeStepResult ( int timeStep ) {

      delete m_Results [timeStep].first;
      delete m_Results [timeStep].second;

      m_Results.erase ( m_Results.begin() + timeStep );
   }

   // (not used)
   void SimulatorStateAdsorption::postProcessAdsorption( const ChemicalModel * chemModel, const bool output ) {

      m_chemicalModel = chemModel;
      calculateSolidOMMaxRatio();
      calculateVolFractions(); 

      int timeSteps = m_Results.size();     

      for ( int i = 0; i < timeSteps; ++ i ) {
         double temp = m_Results[i].second->GetTemperatureCelsius();
         double vre  = m_Results[i].second->getVre();
         SpeciesResult * results = m_Results[i].first->getSpeciesResults();
         calculateAdsorptionTimeStep( i, temp, vre, results, output );
         //     testingPVT ( m_genexResultsPT[i], output );
      }
   }

   // save a genex time step results for the later postprocessing (for shale-gas)
   void SimulatorStateAdsorption::saveTimeStepResultsForAdsorption( SimulatorState * state, Input * inInput, const bool snapshot ) {
      
      Input * timeInput = new Input( * inInput );
      SimulatorStateResult * copyState = new SimulatorStateResult( * state, snapshot );

      m_Results.push_back ( oneTimeStep( copyState, timeInput ) );
   }

    // both unit test and fastgenex Post processing methods
   void SimulatorStateAdsorption::calculatePoreVol ( double vre ) {

     if( vre > m_vreOnset ) {
        int preasphaltId = m_chemicalModel->getSpeciesManager().getPreasphaltId();
        Species * preasphalt = m_chemicalModel->GetSpeciesById( preasphaltId );
        double preasphaltAtomH = preasphalt->GetCompositionByElement( m_chemicalModel->getSpeciesManager().getHydrogenId ());

        //      m_poreVol = 0.5 * ( tanh( 5.0 * ( vre - (1.14 * exp( 0.0241 * emean )) * (( preasphaltAtomH - 0.2 ) / preasphaltAtomH ))) + 1.0 );
       m_poreVol = 0.5 * ( tanh( 5.0 * ( vre - 1.14 * (( preasphaltAtomH - 0.2 ) / preasphaltAtomH ))) + 1.0 );

     } else {
        m_poreVol = 0.0;
     }

   }

   void SimulatorStateAdsorption::calculateAdsorptionTimeStep( int timeStep, const double temperature, const double vre, const SpeciesResult * results, const bool output ) {
      
      calculatePoreVol( vre );
      calculateOrganoPorosity( timeStep );

      // the next methods are to calculate pressures 
      calculateSolidVol( results );
      calculatePoreVolp( results );
      calculateMolarVolGas( results );
      pengRobinsonAB( temperature );
      funcPressure( temperature );
      funGasPressure( output );
   }


    void SimulatorStateAdsorption::calculateOrganoPorosity ( int timeStep ) {
    
      // porosity (percent) 
      if( m_solidVolOnset > 0 ) { 
          m_organoPorosity = m_poreVol / m_solidVolOnset * 100;
      } 
      if( timeStep > m_jonsetOrgPor and timeStep < m_maxVolCoke1J and m_solidOmMaxRatio > 0.0 ) {
         m_organoPorosity = m_organoPorosity / m_solidOmMaxRatio;
      } else if( timeStep <= m_jonsetOrgPor ) {
         m_organoPorosity = 0.0;
      } else {
         m_organoPorosity = m_omPhiMax;
      }
   }

   void SimulatorStateAdsorption::calculateSolidOMMaxRatio ( const double vre ) {

      // calculated at the last genex time step
      calculatePoreVol ( vre );
      
      // porosity (percent)
      if( m_solidVolOnset > 0 ) { 
         m_organoPorosity = m_poreVol / m_solidVolOnset * 100;
      } 

      m_solidOmMaxRatio = m_organoPorosity / m_omPhiMax;
   
   }

   void SimulatorStateAdsorption::calculateSolidOMMaxRatio() {

      // m_chemicalModel must be set
      // needs Vr at the last time step

      if(  m_genexResults.size() > 0 ) {
         // unit test
         int timeSteps = m_genexResults.size();
         calculateSolidOMMaxRatio( m_genexResults[timeSteps - 1].first[1] ); 
      }  else if ( m_Results.size() > 0 ) {
         //fastgenex
         int timeSteps = m_Results.size();
         calculateSolidOMMaxRatio( ( m_Results[timeSteps - 1].second)->getVre() ); 
      }
   }

   void SimulatorStateAdsorption::calculateSolidVol( const SpeciesResult * results ) {

      m_solidVol = 0.0;
      
      for( int i = 0, speciesId = 1; i < m_chemicalModel->GetNumberOfSpecies(); ++i, ++ speciesId ) {
         
         Species *currentSpecies =  m_chemicalModel->GetSpeciesById( speciesId );
         if(currentSpecies == NULL) continue;
 
         SpeciesProperties * speciesProps = currentSpecies->GetSpeciesProperties();
         double concentration = results[i].GetConcentration();
           
         //solid om (immobile) species
         if(not speciesProps->IsMobile()) {
            //volume of each species at...
            const double speciesVolume = concentration / speciesProps->GetDensity();
            m_solidVol +=  speciesVolume; 
         }
      }
   }

   // Pressures methods 

   void SimulatorStateAdsorption::calculateVolFractions () {

      const double rpore = m_poreD * 5e-10; // 2e+9;

      // Volume of single equivalent spherical pore
      m_volSinglePore = 4.0 / 3.0 *  Genex6::Constants::pi * std::pow( rpore, 3 );

      // Volume fraction of free gas in an equivalent spherical pore in the OM
      m_volFractFreeGas = std::pow( (1.0 - Genex6::Constants::monolayerThick / rpore), 3 );

      // Volume fraction of adsorbed gas in an equivalent spherical pore in the OM
      m_volFractAdsorbGas = 1.0 - m_volFractFreeGas;
   }

   //Peng-Robinson parameters a and b
   void SimulatorStateAdsorption::pengRobinsonAB( const double temperature ) {
      
      // Parameter a in the Peng-Robinson equation of state as a function of temperature
      // Critical temperature & pressure of hydrocarbon gas, ***provisionally taken as pure methane
      // Omega is Pitzer's Acentric Factor, which is a measure of the non-sphericity of the molecule (here methane)
      const double Kappa = Genex6::Constants::KappaA + Genex6::Constants::KappaB * Genex6::Constants::Omega +
         Genex6::Constants::KappaC * std::pow ( Genex6::Constants::Omega, 2 );
    
      const double TK  = temperature + Utilities::Maths::CelciusToKelvin;
      const double Alpha = std:: pow (1 + Kappa * (1 - std::sqrt(TK / Genex6::Constants::C1Tcrit)), 2 );
      m_PRa = Genex6::Constants::Praconst * std::pow (Genex6::Constants::R * Genex6::Constants::C1Tcrit, 2 ) / Genex6::Constants::C1Pcrit * Alpha;
      m_PRb = Genex6::Constants::Prbconst * Genex6::Constants::R * Genex6::Constants::C1Tcrit / Genex6::Constants::C1Pcrit;
   }

   void SimulatorStateAdsorption::funcPressure ( const double temperatureC ) {
   
      const double temperature = temperatureC + Utilities::Maths::CelciusToKelvin;

      //  Gas pressure a/c to Peng-Robinson EOS (MPa), treating gas as all free, none adsorbed
      m_gasPressurePR  = ( Genex6::Constants::R * temperature / ( m_molarVolGas - m_PRb ) - m_PRa / ( m_molarVolGas * (  m_molarVolGas + m_PRb ) + 
                                                                                                      m_PRb * ( m_molarVolGas - m_PRb ))) * 1e-6; 
      if( m_gasPressurePR  < 0.0 ) {
         m_gasPressurePR = - m_gasPressurePR;
      }
      // Gas pressure (MPa) a/c to a simple real gas equation. Only used for reference/comparison
      if( m_poreVol > 0.0 ) {
         m_gasPressureRealGas = Genex6::Constants::Zfactor *  m_molesGas * Genex6::Constants::R * temperature / m_poreVol * 1e-6;
      } else {
         m_gasPressureRealGas = 0.0;
      }

 #if 0
     // Gas pressure a/c to Van der Waal's equation, for reference
      const double C1Tcrit = 190.45; // K
      const double C1Pcrit = 4596000.0; // Pa

      // Gas pressure (MPa) a/c to Van der Waal's equation, for methane. For comparison only
      const double VanderWaalA = 27.0 * ( R * R ) * ( C1Tcrit * C1Tcrit ) / ( 64.0 * C1Pcrit );
      const double VanderWaalB = R * C1Tcrit / ( 8.0 * C1Pcrit );
      m_gasPressureVDW = (( R * temperature ) / (  m_molarVolGas - VanderWaalB ) - VanderWaalA / (  m_molarVolGas *  m_molarVolGas )) * 1e-6;
#endif
      m_gasPressureVDW = 0.0;

      //Gas pressure a/c to a virial EOS (MPa)

      const double SecondVirialCoeff = Genex6::Constants::VirialA + Genex6::Constants::VirialB * exp(Genex6::Constants::VirialC / pow (temperature, 2 ));
      m_pressureVirial =  1.0 / (  m_molarVolGas / (Genex6::Constants::R * temperature) - SecondVirialCoeff) * 1e-6;

      // Langmuir pressure at half the maximum adsorbed gas capacity
      // The Langmuir pressure (MPa), which is the pressure corresponding to half the Langmuir adsorption capacity
 
      //This function uses the empirical equation of Zhang et al (2012) for the temperature dependence for the Langmuir constant K
      //Ln(K) = 3366 / T - 11.09
      //for immature Type III kerogen (Cameo coal HI = 239 at 0.5 Ro)
      //Temperature T is in Kelvin and the Langmuir constant K has units of 1/MPa.
      //This is also roughly applicable for highly mature (and therefore highly aromatic) kerogen in the shale gas window
      //regardless of the initial kerogen type or composition.
      // The Langmuir pressure is the reciprocal of K
      m_phalfLangmuir = 1.0 / exp( Genex6::Constants::TypeIIIconstA / temperature - Genex6::Constants::TypeIIIconstB );

      // "Prangmuir pressure" at half the maximum free gas capacity
      //This is the approximate pressure (MPa) at half the maximum free gas capacity a/c to the Peng-Robinson EOS
      //It is used to (closely) approximate the Peng-Robinson EOS with a simpler equation having
      //the form of the Langmuir isotherm. I call this the "Prangmuir" approximation and (as far as I know) it has
      //not been used before.
      m_phalfPrangmuir = ( Genex6::Constants::R * temperature /  m_PRb - m_PRa / ( 7.0 * ( m_PRb * m_PRb ))) * 1e-6;// 1000000.0;

      // Gas pressure a/c to Stainforth "Prangmuir" EOS (MPa), treating gas as all free, none adsorbed
      //Pj is the "Prangmuir pressure" at half the free gas capacity a/c to Peng-Robinson EOS
      //B is the Peng-Robinson minimum molar volume for methane, i.e., at infinite pressure
      //VmGas is the molar volume of the gas at current PT conditions
      m_pressurePrangmuir =  m_PRb * m_phalfPrangmuir / ( m_molarVolGas - m_PRb );

      if( m_pressurePrangmuir < 0.0 ) {
         m_pressurePrangmuir = - m_pressurePrangmuir;
      }

      // Gas pressure a/c to Langmuir isotherm (MPa) assuming all gas is absorbed, none free
      // VmGasMax is the molar volume (of methane) in the adsorption monolayer at maximum adsorption capacity
      // (For simplicity, this is taken to be the same as the Peng-Robinson minimum molar volume of methane at infinite pressure, PRb)
      // VmGas is the molar volume of methane at the PT conditions

      m_pressureLangmuir = m_PRb * m_phalfLangmuir / ( m_molarVolGas - m_PRb );

      if( m_pressureLangmuir < 0.0 ) {
         m_pressureLangmuir = - m_pressureLangmuir;
      }
   }

   void SimulatorStateAdsorption::funGasPressure ( const bool output ) {
      //Maximum number of moles of free gas (methane) that can stored in a spherical pore of radius Rpore
      //   PackingFactor is to deviate the minimum molar vol from the Peng-Robinson value, if necessary

      //Maximum number of moles of free gas (methane) that can stored in a spherical pore of radius Rp (m)
      //VmolarC1 is the minimum molar volume of methane in the pore at infinite pressure
      //(Currently, this is identified with the Peng-Robinson b volume)
      //The packing factor allows deviations from this, but is currently set to 1 (i.e., no deviation)
 
      const double nMaxC1perpore = m_volSinglePore /  m_PRb * Genex6::Constants::PackingFactor;

      // maximum number of moles of free gas (methane) that the pores can hold
      m_NmaxFree = nMaxC1perpore * m_volFractFreeGas * m_nPores;

      //maximum number of moles of adsorbed gas (methane) that the pores can hold
      m_NmaxAds = nMaxC1perpore * m_volFractAdsorbGas * m_nPores;

      // Gas pressure a/c to combined free and adsorbed gas model of JGS
      // Adsorbed gas assumes a Langmuir isotherm
      // Free gas assumes a "Prangmuir" isotherm, which is an approximation of the Peng-Robinson EOS
      // The two combined lead to a quadratic eqn for the pore pressure, which this function solves

      // NmaxFree is the maximum number of moles of free gas (C1) that the organoporosity could hold
      // NmaxAdsorb is the maximum number of moles of adsorbed gas (C1) that the organoporosity could hold
      // m_molesGas is the number of moles of free and adsorbed gas actually in the pores at current conditions
      // m_pressureLangmuir is the Langmuir pressure, i.e., the pressure at half max adsorption capacity (MPa)
      // m_pressurePrangmuir is the Prangmuir pressure, i.e., the pressure at half max free capacity (MPa)

      // a, b and c values in the quadratic eqn ax2 + Bx + c = 0
      float JGSa =  m_NmaxFree + m_NmaxAds - m_molesGas;
      float JGSb = (m_NmaxFree - m_molesGas) * m_phalfLangmuir + (m_NmaxAds - m_molesGas) * m_phalfPrangmuir;
      float JGSc = - m_phalfPrangmuir * m_phalfLangmuir * m_molesGas;

      m_pressureJGS = (- JGSb + std::sqrt( pow( JGSb, 2 ) - 4.0 * JGSa * JGSc)) / (2.0 * JGSa);

      if( m_pressureJGS < 0.0 ) {
         m_pressureJGS = - m_pressureJGS;
      }
 
      m_adsGas = m_pressureJGS *  m_NmaxAds /(  m_phalfLangmuir + m_pressureJGS );
      m_freeGas = m_pressureJGS * m_NmaxFree /(  m_phalfPrangmuir + m_pressureJGS );
      
      if( output ) {
         cout << " c1 state (moles): " << "fg=" << " " << m_freeGas  << " ag=" << " " << m_adsGas  << " vl=" << " " << m_NmaxAds
              << " phl=" << " " << m_phalfLangmuir 
              << " pl=" << " " <<  m_pressureLangmuir << " mvg_pt=" << " " <<
            m_molarVolGas << " pjsg=" << " " <<  m_pressureJGS * 1e+6 << " " << m_molarVolGas << endl; ;
      }
       
   }

   void SimulatorStateAdsorption::calculateMolarVolGas ( const SpeciesResult * results ) {  
      // moles of gas in organopores. We need to use the molecular weights, not the formula weights
      
      // COx = 44.01
      // C5 = 72.15
      // C4 = 58.12
      // C3 = 44.1
      // C2 = 30.07
      // C1 = 16.04
      m_molesGas = 0.0;

      int gasId = m_chemicalModel->getSpeciesManager().getCOxId();
      m_molesGas += results[gasId - 1].GetConcentration()/ Genex6::Constants::COxMolWeight;
  
      gasId = m_chemicalModel->getSpeciesManager().getC5Id();
      Species * gasSpecies = m_chemicalModel->GetSpeciesById( gasId );
      if( gasSpecies->GetSpeciesProperties()->IsHCgas() ) {
         m_molesGas += results[gasId - 1].GetConcentration() / Genex6::Constants::C5MolWeight;
      }

      gasId = m_chemicalModel->getSpeciesManager().getC4Id();
      gasSpecies = m_chemicalModel->GetSpeciesById( gasId );
      if( gasSpecies->GetSpeciesProperties()->IsHCgas() ) {
         m_molesGas += results[gasId - 1].GetConcentration() / Genex6::Constants::C4MolWeight;
     }

      gasId = m_chemicalModel->getSpeciesManager().getC3Id();
      gasSpecies = m_chemicalModel->GetSpeciesById( gasId );
      if( gasSpecies->GetSpeciesProperties()->IsHCgas() ) {
         m_molesGas += results[gasId - 1].GetConcentration() / Genex6::Constants::C3MolWeight;
     }

      gasId = m_chemicalModel->getSpeciesManager().getC2Id();
      gasSpecies = m_chemicalModel->GetSpeciesById( gasId );
      if( gasSpecies->GetSpeciesProperties()->IsHCgas() ) {
         m_molesGas += results[gasId - 1].GetConcentration() / Genex6::Constants::C2MolWeight;
      }

      gasId = m_chemicalModel->getSpeciesManager().getC1Id();
      gasSpecies = m_chemicalModel->GetSpeciesById( gasId );
      if( gasSpecies->GetSpeciesProperties()->IsHCgas() ) {
         m_molesGas += results[gasId - 1].GetConcentration() / Genex6::Constants::C1MolWeight;
     } else {

         cout << "C1 is in liquid phase" << endl;
      }


      m_molesGas *= 1000;   

      // molar volume (m3) as in organopores = volume of pores/ number of moles of gas
      m_molarVolGas = m_poreVolp / m_molesGas;
  
   }

   void SimulatorStateAdsorption::calculatePoreVolp ( const SpeciesResult * results ) {

     // minimum porosity in organic matter; arbitrary
     // minOrganPorosity - provisional factor by which the organoporosity compacts
     // cpctFactor - volume of organoporosity less than volume of water

     if( m_solidVol < m_solidVolOnset2 ) {
        int h2oId = m_chemicalModel->getSpeciesManager().getH2OId();
        Species * h2o = m_chemicalModel->GetSpeciesById( h2oId );
        
        const double densityH2O = h2o->GetSpeciesProperties()->GetDensity();
        const double concentrationH2O = results [h2oId - 1].GetConcentration();
        
        m_poreVolp = ( m_solidVolOnset2 - m_solidVol - concentrationH2O / densityH2O + Genex6::Constants::minOrganPorosity ) * Genex6::Constants::cpctFactor;
     } else {
        m_poreVolp = Genex6::Constants::minOrganPorosity;
     }
     // number of equivalent spherical pores
     m_nPores = m_poreVolp / m_volSinglePore;

   }

 
}
