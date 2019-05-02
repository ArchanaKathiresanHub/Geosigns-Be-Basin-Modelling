#ifndef _GENEX6_KERNEL__SIMULATORSTATEADSORPTION_H_
#define _GENEX6_KERNEL__SIMULATORSTATEADSORPTION_H_

#include "ChemicalModel.h"
#include "SimulatorState.h"
#include "SimulatorStateResult.h"
#include "OutputUnitTest.h"
 
using namespace std;
using namespace CBMGenerics;

namespace Genex6
{
   // for UnitTest
   typedef std::pair <std::vector<double>, const SpeciesResult * > oneTimeStepUnit;
   typedef std::vector<oneTimeStepUnit> timeStepResultsUnit;

   // for fastgenex
   typedef std::pair <SimulatorStateResult *, Input *> oneTimeStep;
   typedef std::vector<oneTimeStep> timeStepResults;
   
class SimulatorStateAdsorption
{
public:
   SimulatorStateAdsorption();
   
   ~SimulatorStateAdsorption();
   
   void clean();
   void cleanResults();
   void setChemicalModel( const ChemicalModel * inModel );
   void setSimulatorState( SimulatorState * inState );
   SimulatorState * getSimulatorState() const;
   const ChemicalModel * getChemicalModel() const { return m_chemicalModel; }

   void calculateSolidVol ( SimulatorState * simulatorState, const double vre );
   // for UnitTest
   void saveTimeStepResultsForAdsorption( const double temperature, const double vre, const SpeciesResult * results );
   bool postProcessAdsorption( const ChemicalModel * chemModel, SimulatorState * simulatorState, std::vector<Input*> &inputStates );
   void setAdsorptionState( Input * inputState );
   // for fastgenex
   void saveTimeStepResultsForAdsorption( SimulatorState * state, Input * inInput, const bool snapshot = false );
   void postProcessAdsorption( const ChemicalModel * chemModel, const bool output = false );
   bool setOrganoporosityVars ( const double inOmPhiMax, const double inPoreD );
   oneTimeStep & getOneTimeStepResult ( int timeStep );
   void calculateAdsorptionTimeStep ( int timeStep );
   void removeOneTimeStepResult ( int timeStep );
   int getTimeSteps() const;

   double getOrganoPorosity() const;
   double getOMPhi() const;
   double getPessureJGS() const;
   double getPessurePrangmuir() const;
   double getPessureLangmuir() const;
   double getAdsGas() const;
   double getFreeGas() const;
   double getNmaxAds() const;
   double getNmaxFree() const;
   double getMolesGas() const;
   double getMolarGasVol() const;
private:
   timeStepResultsUnit m_genexResults; // unit test
   timeStepResults     m_Results; // fastgenex

   bool   m_setSolidVol;
   double m_solidVol;
   double m_solidVolOnset;
   double m_vreOnset;
   double m_poreVol;
   double m_poreVolp;
   double m_nPores;
   double m_organoPorosity;
   double m_molesGas;
   double m_molarVolGas;
   double m_PRa;
   double m_PRb;
   double m_gasPressurePR;
   double m_gasPressureRealGas;
   double m_gasPressureVDW;
   double m_phalfLangmuir;
   double m_phalfPrangmuir;
   double m_pressurePrangmuir;
   double m_pressureLangmuir;
   float  m_pressureJGS;
   double m_pressureVirial;
   double m_volFractFreeGas;
   double m_volFractAdsorbGas;
   double m_volSinglePore;

   double m_omPhiMax;
   double m_solidOmMaxRatio;
   double m_solidVolOnset2;
   double m_maxVolCoke1;
   int    m_maxVolCoke1J;
   int    m_jonsetOrgPor;
   double m_maxVolPrecoke;
   double m_poreD;

   double m_NmaxFree;
   double m_NmaxAds;
   double m_adsGas;
   double m_freeGas;
   double m_concGas;

   const ChemicalModel *m_chemicalModel;
   SimulatorState      *m_simulatorState;
   Input               *m_sourceRockInput;
 
   void calculateAdsorptionTimeStep( int timeStep, oneTimeStepUnit oneTimeStep, const bool output = false );
   void calculateAdsorptionTimeStep( int timeStep, const double temperature, const double vre, const SpeciesResult * results, const bool output = false );

   void calculateSolidOMMaxRatio( const double vre );
   void calculateSolidOMMaxRatio();

   void calculateSolidVol( const SpeciesResult * results );

   void calculatePoreVol( const double vre );
   void calculatePoreVolp( const SpeciesResult * results );

   void calculateVolFractions();
   void calculateOrganoPorosity( int timeStep );
   void calculateMolarVolGas( const SpeciesResult * results );

   void pengRobinsonAB( const double temperature );
   void funcPressure ( const double temperature ) ;
   void funGasPressure( const bool output = false );

  };

   inline double SimulatorStateAdsorption::getOrganoPorosity() const {

      return m_organoPorosity;
   }
   inline double SimulatorStateAdsorption::getOMPhi() const {

      return m_omPhiMax;
   }
   inline double SimulatorStateAdsorption::getPessureLangmuir() const {

      return m_pressureLangmuir;
   }
   inline double SimulatorStateAdsorption::getPessurePrangmuir() const {

      return m_pressurePrangmuir;
   }
   inline double SimulatorStateAdsorption::getPessureJGS() const {

      return m_pressureJGS;
   }
   inline double SimulatorStateAdsorption::getAdsGas() const {

      return m_adsGas;
   }
   inline double SimulatorStateAdsorption::getFreeGas() const {

      return m_freeGas;
   }
   inline double SimulatorStateAdsorption::getNmaxAds() const {

      return m_NmaxAds;
   }
   inline double SimulatorStateAdsorption::getNmaxFree() const {

      return m_NmaxFree;
   }
   inline double SimulatorStateAdsorption::getMolesGas() const {

      return m_molesGas;
   }
   inline double SimulatorStateAdsorption::getMolarGasVol() const {

      return m_molarVolGas;
   }

}
#endif
