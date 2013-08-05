#ifndef _Interface_INPUT_H_
#define _Interface_INPUT_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Interface/GridMap.h"
#include "Interface/CrustalThicknessData.h"
#include "LinearFunction.h"

using namespace std;
using namespace DataAccess;
using Interface::GridMap;

namespace database
{
   class Record;
   class Table;
}
namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class GridMap;
      class Grid;
   }
}

namespace CrustalThicknessInterface {
   const string TableBasicConstants = "Table:[BasicConstants]";
   const string TableLithoAndCrustProperties = "Table:[LithoAndCrustProperties]";
   const string TableTemperatureData = "Table:[TemperatureData]";
   const string TableSolidus = "Table:[Solidus]";
   const string TableMagmaLayer = "Table:[MagmaLayer]";
   const string TableUserDefinedData = "Table:[UserDefinedData]";
   const string TableMantle = "Table:[Mantle]";
   const string TableStandardCrust = "Table:[StandardCrust]";
   const string TableLowCondCrust = "Table:[LowCondCrust]";
   const string TableBasalt = "Table:[Basalt]";
   const string EndOfTable = "[EndOfTable]";

   const string t_0 = "t_0";
   const string t_r = "t_r";
   const string initialCrustThickness = "initialCrustThickness";
   const string maxBasalticCrustThickness = "maxBasalticCrustThickness";
   const string initialLithosphericThickness = "initialLithosphericThickness";
   const string seaLevelAdjustment = "seaLevelAdjustment";
   const string coeffThermExpansion = "coeffThermExpansion";
   const string initialSubsidenceMax = "initialSubsidenceMax";
   const string thermalSubsidenceMax = "thermalSubsidenceMax";
   const string pi = "pi";
   const string E0 = "E0";
   const string tau = "tau";
   const string modelTotalLithoThickness = "modelTotalLithoThickness";
   const string backstrippingMantleDensity = "backstrippingMantleDensity";
   const string lithoMantleDensity = "lithoMantleDensity";
   const string baseLithosphericTemperature = "baseLithosphericTemperature";
   const string referenceCrustThickness = "referenceCrustThickness";
   const string referenceCrustDensity = "referenceCrustDensity";
   const string waterDensity = "waterDensity";
   const string A = "A";
   const string B = "B";
   const string C = "C";
   const string D = "D";
   const string E = "E";
   const string F = "F";
   const string T = "T";
   const string decayConstant = "decayConstant";
   const string lithosphereThicknessMin = "HLmin";
   const string maxNumberOfMantleElements = "NLMEmax";
   const string maxNumberOfMantleElementsOld = "HLMEmax";
   const string Heat = "Heat";
   const string Rho  = "Rho";

   void parseLine(const string &theString, const string &theDelimiter, vector<string> &theTokens);
   int GetRank ();
   int GetNumOfProcs ();
}

class InterfaceInput : public Interface::CrustalThicknessData
{

public:

   InterfaceInput (Interface::ProjectHandle * projectHandle, database::Record * record);

   virtual ~InterfaceInput ();

   
private:

  //-------------- User interface data ------------------
   double m_t_0;
   double m_t_r;
   double m_initialCrustThickness;
   double m_initialLithosphericThickness;
   double m_maxBasalticCrustThickness;
   double m_seaLevelAdjustment;

   //-------------- Basic constants ---------------------
   double m_coeffThermExpansion;
   double m_initialSubsidenceMax;
   double m_thermalSubsidenceMax;
   double m_pi;
   double m_E0;
   double m_tau;

   //-------------- Lithospphere and crust properties ---------------------
   double m_modelTotalLithoThickness;
   double m_backstrippingMantleDensity;
   double m_lithoMantleDensity;
   double m_baseLithosphericTemperature;
   double m_referenceCrustThickness;
   double m_referenceCrustDensity;
   double m_waterDensity;
   
   //------------- Asthenosphere potential temperature data ---------------------
   double m_A;
   double m_B;
  
   //------------- Solidus (onset of adiabatic melting) ---------------------
   double m_C;
   double m_D;

   //------------- Magma-layer density ---------------------
   double m_E;
   double m_F;
   double m_decayConstant;

   //------------- Data from CTCIoTbl
   
   const GridMap * m_T0Map;
   const GridMap * m_TRMap;
   const GridMap * m_HCuMap;
   const GridMap * m_HLMuMap;
   const GridMap * m_HBuMap;
   const GridMap * m_DeltaSLMap;

   int    m_smoothRadius;
   string m_baseRiftSurfaceName; 
   //-------------   
   double m_modelCrustDensity;
   double m_TF_onset;
   double m_TF_onset_lin;
   double m_TF_onset_mig;
   double m_PTa;
   double m_magmaticDensity;
   double m_WLS_onset;
   double m_WLS_crit;
   double m_WLS_exhume;
   double m_WLS_exhume_serp;

   void clean();
   
public:
   void loadInputDataAndConfigurationFile( const string & inFile );
   void loadInputData();
   void loadConfigurationFile( const string & inFile );

   void LoadBasicConstants( ifstream &ConfigurationFile );
   void LoadLithoAndCrustProperties( ifstream &ConfigurationFile );
   void LoadTemperatureData( ifstream &ConfigurationFile );
   void LoadSolidus( ifstream &ConfigurationFile );
   void LoadMagmaLayer( ifstream &ConfigurationFile );
   void LoadUserDefinedData( ifstream &ConfigurationFile );

   int    getSmoothRadius() const;
   double getMidAge() const;
   double getDensityDifference() const;
   double getInitialCrustThickness() const;
   double getInitialLithosphereThickness() const;
   double getBackstrippingMantleDensity() const;
   double getWaterDensity() const;
   double getEstimatedCrustDensity() const;
   double getTFOnset() const;
   double getTFOnsetLin() const;
   double getTFOnsetMig() const;
   double getPTa() const;
   double getMagmaticDensity() const;
   double getWLSexhume() const;
   double getWLScrit() const;
   double getWLSonset() const;
   double getWLSexhumeSerp() const;
   double getDeltaSLValue(unsigned int i, unsigned int j) const;
   const string& getBaseRiftSurfaceName() const;
   bool defineLinearFunction( LinearFunction & theFunction, unsigned int i, unsigned int j );

   void retrieveData();
   void restoreData();

   unsigned firstI() const;
   unsigned firstJ() const;
   unsigned lastI() const;
   unsigned lastJ() const;

   double getInitialSubsidence() const { return m_initialSubsidenceMax; }

   const GridMap* getT0Map() const;
   const GridMap* getTRMap() const;
   const GridMap* getHCuMap() const;
   const GridMap* getHLMuMap() const;
   const GridMap* getDeltaSLMap() const;
};


inline const GridMap* InterfaceInput::getT0Map() const {
   return m_T0Map;
}

inline const GridMap* InterfaceInput::getTRMap() const {
   return m_TRMap;
}

inline const GridMap* InterfaceInput::getHCuMap() const {
   return m_HCuMap;
}

inline const GridMap* InterfaceInput::getHLMuMap() const {
   return m_HLMuMap;
}

inline const GridMap* InterfaceInput::getDeltaSLMap() const {
   return m_DeltaSLMap;
}

inline int InterfaceInput::getSmoothRadius() const {
   return m_smoothRadius;
}

inline double InterfaceInput::getDeltaSLValue( unsigned int i, unsigned int j ) const {
   
   if( m_DeltaSLMap->getValue(i, j) != m_DeltaSLMap->getUndefinedValue() ) {
      return m_DeltaSLMap->getValue(i, j);
   } else {
      return Interface::DefaultUndefinedMapValue;
   }
}

inline double InterfaceInput::getMidAge() const {
   return (m_t_r + m_t_0) / 2;
}

inline double InterfaceInput::getInitialCrustThickness() const {
   
   return m_initialCrustThickness;
}

inline double InterfaceInput::getInitialLithosphereThickness() const {
   
   return m_initialLithosphericThickness;
}

inline double InterfaceInput::getBackstrippingMantleDensity() const {
   
   return m_backstrippingMantleDensity;
}

inline double InterfaceInput::getWaterDensity() const {
   
   return m_waterDensity;
}

inline double InterfaceInput::getEstimatedCrustDensity() const {
   
   return m_modelCrustDensity;
}

inline double InterfaceInput::getTFOnset() const {
   
   return m_TF_onset;
}

inline double InterfaceInput::getTFOnsetLin() const {
   
   return m_TF_onset_lin;
}

inline double InterfaceInput::getTFOnsetMig() const {
   
   return m_TF_onset_mig;
}

inline double InterfaceInput::getPTa() const {
   
   return m_PTa;
}

inline double InterfaceInput::getMagmaticDensity() const {
   
   return m_magmaticDensity;
}

inline double InterfaceInput::getWLSexhume() const {
   
   return m_WLS_exhume;
}

inline double InterfaceInput::getWLScrit() const {
   
   return m_WLS_crit;
}

inline double InterfaceInput::getWLSonset() const {
   
   return m_WLS_onset;
}

inline double InterfaceInput::getWLSexhumeSerp() const {
   
   return m_WLS_exhume_serp;
}

inline const string& InterfaceInput::getBaseRiftSurfaceName() const {
   return m_baseRiftSurfaceName;
}

inline unsigned InterfaceInput::firstI() const {
   return m_T0Map->firstI();
}

inline unsigned InterfaceInput::firstJ() const {
   return m_T0Map->firstJ();
}
inline unsigned InterfaceInput::lastI() const {
   return m_T0Map->lastI();
}

inline unsigned InterfaceInput::lastJ() const {
   return m_T0Map->lastJ();
}

#endif

