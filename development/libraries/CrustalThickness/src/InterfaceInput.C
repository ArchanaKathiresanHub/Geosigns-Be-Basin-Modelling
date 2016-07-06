//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "InterfaceInput.h"
#include "FilePath.h"

// std library
#define _USE_MATH_DEFINES
#include <math.h>

// TableIO library
#include "cauldronschemafuncs.h"

// DataAccess library
#include "errorhandling.h"
#include "Interface/CrustFormation.h"
#include "Interface/Formation.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"
#include "Interface/Grid.h"


// GeoPhysics library
#include "GeoPhysicsProjectHandle.h"

// DataMining library
#include "CauldronDomain.h"
#include "DataMiner.h"
#include "ElementPosition.h"
#include "InterpolatedPropertyValues.h"
#include "DataMiningProjectHandle.h"

// DerivedProperties library
#include "DerivedPropertyManager.h"

//utility
#include "LogHandler.h"
#include "StringHandler.h"

//------------------------------------------------------------//
InterfaceInput::InterfaceInput(Interface::ProjectHandle * projectHandle, database::Record * record) :
   Interface::CrustalThicknessData( projectHandle, record )
   {

   clean();
   //-------------
   m_T0Map = 0;
   m_TRMap = 0;
   m_HCuMap = 0;
   m_HLMuMap = 0;
   m_HBuMap = 0;
   m_DeltaSLMap = 0;
   m_smoothRadius = 0;

   m_baseRiftSurfaceName = "";
   m_derivedManager = nullptr;

}
//------------------------------------------------------------//
InterfaceInput::~InterfaceInput() {
   if (m_derivedManager != nullptr){
      delete m_derivedManager;
   }
} 
//------------------------------------------------------------//
void InterfaceInput::clean() {

   //-------------- User defined ---------------------
   m_t_0                          = 0.0;
   m_t_r                          = 0.0;
   m_initialCrustThickness        = 0.0;
   m_initialLithosphericThickness = 0.0;
   m_maxBasalticCrustThickness    = 0.0;
   m_seaLevelAdjustment           = 0.0;

   //-------------
   m_modelCrustDensity = 0;
   m_TF_onset          = 0;
   m_TF_onset_lin      = 0;
   m_TF_onset_mig      = 0;
   m_PTa               = 0;
   m_magmaticDensity   = 0;
   m_WLS_exhume        = 0;
   m_WLS_crit          = 0;
   m_WLS_onset         = 0;
   m_WLS_exhume_serp   = 0;  
}
//------------------------------------------------------------//
void InterfaceInput::loadInputDataAndConfigurationFile( const string & inFile ) {
   ///1. Load input data
   try {
   loadInputData();
   }
   catch (InputException& ex){
      LogHandler( LogHandler::ERROR_SEVERITY ) << ex.what();
   }
   catch (...){
      LogHandler( LogHandler::FATAL_SEVERITY ) << "CTC fatal error when loading input data";
   }
   ///2. Load configuration file
   try {
      m_constants.loadConfigurationFileCtc( inFile );
   }
   catch (InputException& ex){
      LogHandler( LogHandler::ERROR_SEVERITY ) << ex.what();
   }
   catch (...){
      LogHandler( LogHandler::FATAL_SEVERITY ) << "CTC fatal error when loading configuration file";
   }
}
//------------------------------------------------------------//
void InterfaceInput::loadInputData() {
   
   m_TRMap   = getMap( Interface::TRIni );
   m_T0Map   = getMap (Interface::T0Ini);
   m_HCuMap  = getMap (Interface::HCuIni);
   m_HLMuMap = getMap (Interface::HLMuIni);
   m_HBuMap  = getMap (Interface::HBu);
   m_DeltaSLMap = getMap (Interface::DeltaSL);
   m_baseRiftSurfaceName = getSurfaceName();
   m_smoothRadius = getFilterHalfWidth();
   static const DataAccess::Interface::Grid * S_a0_FORTEST = m_T0Map->getGrid();
   if (m_T0Map == 0 || m_TRMap == 0 || m_HCuMap == 0 || m_HLMuMap == 0 || m_DeltaSLMap == 0) {
      throw InputException() << "Cannot load input data... Aborting... ";
   }
     
}

//------------------------------------------------------------//
void InterfaceInput::LoadUserDefinedData( ifstream &ConfigurationFile ) {

   string line;
   vector<string> theTokens;
   string delim = ",";
   size_t firstNotSpace;
   int countParam = 0;
   
   for(;;) {
      getline (ConfigurationFile, line, '\n');
        
      if( line == ConfigFileAlcCtc::EndOfTable || line.size() == 0) {
         break;
      }
      firstNotSpace = line.find_first_not_of(" \t"); 
      
      if( line[firstNotSpace] != '#' ) {
      
      StringHandler::parseLine( line, delim, theTokens );
      
      if( theTokens.size() == 2 ) {

         if( theTokens[0] == ConfigFileAlcCtc::t_0 ) {

            m_t_0 = atof( theTokens[1].c_str() );
               ++ countParam;

         } else if( theTokens[0] == ConfigFileAlcCtc::t_r ) {

            m_t_r = atof( theTokens[1].c_str() );
               ++ countParam;

         } else if( theTokens[0] == ConfigFileAlcCtc::initialCrustThickness ) {

            m_initialCrustThickness = atof( theTokens[1].c_str() );
               ++ countParam;

         } else if( theTokens[0] == ConfigFileAlcCtc::maxBasalticCrustThickness ) {

            m_maxBasalticCrustThickness = atof ( theTokens[1].c_str() );
               ++ countParam;

         } else if( theTokens[0] == ConfigFileAlcCtc::initialLithosphericThickness ) {

            m_initialLithosphericThickness = atof( theTokens[1].c_str() );
               ++ countParam;

         } else if( theTokens[0] == ConfigFileAlcCtc::seaLevelAdjustment ) {

            m_seaLevelAdjustment = atof( theTokens[1].c_str() );
               ++ countParam;
            }
            else{
               LogHandler( LogHandler::WARNING_SEVERITY ) << "CTC configuration file UserDefinedData table: unknown CTC parameter '" << theTokens[0] << "'.";
            }
         } 
         else {
         theTokens.clear();
            throw InputException() << "CTC configuration file UserDefinedData table: unexpected parameter definition (should be Name, Value).";
      }
      theTokens.clear();
   }
}
   if( countParam != 6 ) {
      throw InputException() << "CTC configuration file UserDefinedData table: 6 parameters expected but only " << countParam << " found.";
   }
}

//------------------------------------------------------------//
void InterfaceInput::loadDerivedPropertyManager(){
   if (m_derivedManager == nullptr){
      GeoPhysics::ProjectHandle* projectHandle = dynamic_cast<GeoPhysics::ProjectHandle*>(this->getProjectHandle());
      if (projectHandle == nullptr){
         throw InputException() << "Cannot access the derived property manager.";
      }
      m_derivedManager = new DerivedProperties::DerivedPropertyManager( projectHandle );
   }
   else{
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "Derived property manager already loaded.";
   }
}

//------------------------------------------------------------//
void InterfaceInput::loadTopAndBottomOfSediments( GeoPhysics::ProjectHandle* projectHandle, const double snapshotAge, const string & baseSurfaceName ) {

   const Interface::Snapshot * currentSnapshot = projectHandle->findSnapshot( snapshotAge, MINOR | MAJOR );

   //1. Find the bottom of the sediment
   if (baseSurfaceName == "") {

      const Interface::CrustFormation * formationCrust = dynamic_cast<const Interface::CrustFormation *>(projectHandle->getCrustFormation());

      if (!formationCrust) {
         throw InputException() << "Could not find Crust formation at the age " << currentSnapshot->getTime() << ".";
      }
      m_bottomOfSedimentSurface = formationCrust->getTopSurface();
      m_topOfMantle = formationCrust->getBottomSurface();
      m_botOfMantle = formationCrust->getTopSurface();
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "Crust formation: '" << formationCrust->getName() << "', surface above '" << m_bottomOfSedimentSurface->getName() << "'.";
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "Crust formation: '" << formationCrust->getName() << "', surface under '" << m_topOfMantle->getName()             << "'.";
   }
   else {
      m_bottomOfSedimentSurface = projectHandle->findSurface( baseSurfaceName );
      if (!m_bottomOfSedimentSurface) {
         throw InputException() << "Could not find user defined base surface of the rift event: '" << baseSurfaceName << "'.";
      }
      else {
         m_topOfMantle = m_bottomOfSedimentSurface->getBottomFormation()->getBottomSurface();
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "Using surface '" << m_bottomOfSedimentSurface->getName() << "' as the base of syn-rift.";
      }
   }

   //2. Find the top of the sediment
   Interface::FormationList * myFormations = projectHandle->getFormations( currentSnapshot, true );
   const Interface::Formation * formationWB = (*myFormations)[0]; // find Water bottom

   if (!formationWB) {
      throw InputException() << "Could not find the Water bottom formation at the age " << currentSnapshot->getTime();
   }

   m_topOfSedimentSurface = formationWB->getTopSurface();

   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Top sediment surface: '" << m_topOfSedimentSurface->getName()    << "'.";
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Bot sediment surface: '" << m_bottomOfSedimentSurface->getName() << "'.";
}

//------------------------------------------------------------//
const DataModel::AbstractProperty* InterfaceInput::loadDepthProperty () {

   if (m_derivedManager == nullptr) loadDerivedPropertyManager();
   const DataModel::AbstractProperty* depthProperty = m_derivedManager->getProperty( "Depth" );
   if (!depthProperty) {
      throw InputException() << "Could not find property named Depth.";
   }
   return depthProperty;
}

//------------------------------------------------------------//
void InterfaceInput::loadDepthData( GeoPhysics::ProjectHandle* projectHandle, const DataModel::AbstractProperty* depthProperty, const double snapshotAge ) {

   if (m_derivedManager == nullptr) loadDerivedPropertyManager();
   const Interface::Snapshot * currentSnapshot = projectHandle->findSnapshot( snapshotAge, MINOR | MAJOR );
   try{
      // Find the depth property of the bottom of sediment
      m_depthBasement = m_derivedManager->getSurfaceProperty( depthProperty, currentSnapshot, m_bottomOfSedimentSurface );
      // Find the depth property of the top of sediment
      m_depthWaterBottom = m_derivedManager->getSurfaceProperty( depthProperty, currentSnapshot, m_topOfSedimentSurface );
   }
   catch (InputException& ex){
      LogHandler( LogHandler::ERROR_SEVERITY ) << ex.what();
   }
   catch (...){
      LogHandler( LogHandler::FATAL_SEVERITY ) << "Could not load depth data for property " << depthProperty->getName() << " @ snapshot " << snapshotAge << ".";
   }
}

//------------------------------------------------------------//
const DataModel::AbstractProperty* InterfaceInput::loadPressureProperty () {

   if (m_derivedManager == nullptr) loadDerivedPropertyManager();
   const DataModel::AbstractProperty* pressureProperty = m_derivedManager->getProperty( "LithoStaticPressure" );
   if (!pressureProperty) {
      throw InputException() << "Could not find property named LithoStaticPressure.";
   }
   return pressureProperty;
}

//------------------------------------------------------------//
void InterfaceInput::loadPressureData( GeoPhysics::ProjectHandle* projectHandle, const DataModel::AbstractProperty* pressureProperty, const double snapshotAge ) {

   if (m_derivedManager == nullptr) loadDerivedPropertyManager();
   const Interface::Snapshot * currentSnapshot    = projectHandle->findSnapshot( snapshotAge, MINOR | MAJOR );
   const Interface::Snapshot * presentDaySnapshot = projectHandle->findSnapshot( 0.0,         MINOR | MAJOR );
   try{
      // Find the pressure property of the bottom of sediment
      m_pressureBasement = m_derivedManager->getSurfaceProperty( pressureProperty, currentSnapshot, m_bottomOfSedimentSurface );
      // Find the pressure property of the top of sediment
      m_pressureWaterBottom = m_derivedManager->getSurfaceProperty( pressureProperty, currentSnapshot, m_topOfSedimentSurface );
      // Find the pressure property of the bottom of the mantle
      m_pressureMantle             = m_derivedManager->getSurfaceProperty( pressureProperty, currentSnapshot,    m_botOfMantle );
      m_pressureMantleAtPresentDay = m_derivedManager->getSurfaceProperty( pressureProperty, presentDaySnapshot, m_botOfMantle );
   }
   catch (InputException& ex){
      LogHandler( LogHandler::ERROR_SEVERITY ) << ex.what();
   }
   catch (...){
      LogHandler( LogHandler::FATAL_SEVERITY ) << "Could not load pressure data for property " << pressureProperty->getName() << " @ snapshot " << snapshotAge << ".";
   }

}

GridMap* InterfaceInput::loadPropertyDataFromDepthMap( DataAccess::Mining::ProjectHandle* handle,
                                                       const GridMap* depthMap,
                                                       const Interface::Property* property,
                                                       const Interface::Snapshot* snapshot ){
   if (m_derivedManager == nullptr) loadDerivedPropertyManager();
   GridMap* outputPropertyMap = getFactory()->produceGridMap( 0, 0, handle->getActivityOutputGrid(), Interface::DefaultUndefinedMapValue, 1 );
   outputPropertyMap->retrieveData();

   ///1. Set the dataminer to the property we want to extract
   DataAccess::Mining::DataMiner dataMiner( handle, *m_derivedManager );
   std::vector<const Interface::Property*> propertySet;
   propertySet.push_back( property );
   handle->getDomainPropertyCollection()->setSnapshot( snapshot );
   dataMiner.setProperties( propertySet );
   ///2. Set the cauldron domain to the snapshot we want to extract
   DataAccess::Mining::CauldronDomain cauldronDomain( handle );
   cauldronDomain.setSnapshot( snapshot, *m_derivedManager );
   ///3. Iniliasize list of elements and interpolated values
   DataAccess::Mining::ElementPosition elementPosition;
   std::vector<DataAccess::Mining::ElementPosition> elementPositionSequence;
   std::vector<DataAccess::Mining::InterpolatedPropertyValues> interpolatedValues;
   ///4. Find x,y,z position
   std::map<unsigned int, map<unsigned int, size_t>> mapIJtoElement;
   unsigned int firstI = depthMap->firstI();
   unsigned int lastI  = depthMap->lastI();
   unsigned int firstJ = depthMap->firstJ();
   unsigned int lastJ  = depthMap->lastJ();
   double deltaX = depthMap->deltaI();
   double deltaY = depthMap->deltaJ();
   double x, y, z;
   for (unsigned int i = firstI; i <= lastI; i++){
      for (unsigned int j = firstJ; j <= lastJ; j++){
         if (handle->getNodeIsValid( i, j )){
            x = depthMap->minI() + double( i ) * deltaX;
            y = depthMap->minJ() + double( j ) * deltaY;
            z = depthMap->getValue(i,j);
            cauldronDomain.findLocation( x, y, z, elementPosition );
            elementPositionSequence.push_back( elementPosition );
            mapIJtoElement[i][j] = elementPositionSequence.size() - 1;
         }
      }
   }
   ///5. Interpolate property values for each x,y,z location
   dataMiner.compute( elementPositionSequence, property, interpolatedValues );
   ///6. Set values to grid map
   for (unsigned int i = firstI; i <= lastI; i++){
      for (unsigned int j = firstJ; j <= lastJ; j++){
         if (handle->getNodeIsValid( i, j )){
            double test = interpolatedValues[mapIJtoElement[i][j]].operator()( property );
            outputPropertyMap->setValue( i, j, interpolatedValues[mapIJtoElement[i][j]].operator()( property ) );
         }
         else{
            outputPropertyMap->setValue( i, j, Interface::DefaultUndefinedMapValue );
         }
      }
   }
   outputPropertyMap->restoreData();
   return outputPropertyMap;

}


//------------------------------------------------------------//
bool InterfaceInput::defineLinearFunction( LinearFunction & theFunction, unsigned int i, unsigned int j ) {

   // This method is to calculate coefficients for linear function to invert from WLS to TF (thinning factor)
   const double pi2 = pow (m_constants.getPi(), 2);
   const double pi2_8 = pi2 / 8;
   
   // Step 4.1
   // average uppermost mantle density 
   if( m_HCuMap->getValue( i, j ) == m_HCuMap->getUndefinedValue() ) return false;
   m_initialCrustThickness = m_HCuMap->getValue( i, j );

   if( m_initialCrustThickness == 0 ) {
      throw InputException() << "InitialCrustThickness=0 but should be !=0";
   }
   const double mantleDensityAV = m_constants.getLithoMantleDensity() * (1 - (m_constants.getCoeffThermExpansion() * m_constants.getBaseLithosphericTemperature() / 2)
      * ((m_constants.getReferenceCrustThickness() + m_initialCrustThickness) / m_constants.getModelTotalLithoThickness()));
   // estimated continental crust density
   m_modelCrustDensity = (m_constants.getReferenceCrustDensity() * m_constants.getReferenceCrustThickness()
      + mantleDensityAV * (m_initialCrustThickness - m_constants.getReferenceCrustThickness())) / m_initialCrustThickness;
   
   // Step 4.2
   // asthenosphere potential temperature
   if( m_HBuMap->getValue( i, j ) == m_HBuMap->getUndefinedValue() ) return false;
   m_maxBasalticCrustThickness = m_HBuMap->getValue( i, j );
   m_PTa = m_constants.getB() + m_constants.getA() * sqrt( m_maxBasalticCrustThickness );
   const double Hsol = m_constants.getC() * m_PTa + m_constants.getD();
   // crustal thinning factor at melt onset
   if( m_HLMuMap->getValue( i, j ) == m_HLMuMap->getUndefinedValue() ) return false;
   m_initialLithosphericThickness = ( m_HLMuMap->getValue( i, j ) + m_initialCrustThickness );
   m_TF_onset = 1 - Hsol / m_initialLithosphericThickness;
   // liner approximation of thinning factor at melt onset
   m_TF_onset_lin = ( 1 + 2 * m_TF_onset ) / 3;
   // crustal thinning factor at threshold
   if( m_maxBasalticCrustThickness == 0 ) {
      m_TF_onset_mig = m_TF_onset;
   } else {
      m_TF_onset_mig = m_TF_onset + ((1 - m_TF_onset) * sqrt(2000 / m_maxBasalticCrustThickness));
   }
   if (m_constants.getDecayConstant() == 0) { m_constants.setDecayConstant( 1 ); }
      
   m_magmaticDensity = m_constants.getE() + (m_constants.getF() - m_constants.getE())
      * (1 - exp( -1 * m_maxBasalticCrustThickness / m_constants.getDecayConstant() ));

   // Step 4.3
   if( m_T0Map->getValue( i, j ) == m_T0Map->getUndefinedValue() || m_TRMap->getValue( i, j ) == m_TRMap->getUndefinedValue()) return false;
   const double t_mr = (m_T0Map->getValue(i, j) + m_TRMap->getValue(i, j)) / 2; 
   
   if( t_mr < 0 ) return false; // simple check if input data is valid 
   
   const double expValue = 1 - exp( -15 * t_mr / m_constants.getTau() );

   m_WLS_exhume = m_constants.getInitialSubsidenceMax() + m_constants.getE0()
      * ((1 - exp( -t_mr / m_constants.getTau() )) + (pi2_8 - 1) * expValue);

   m_WLS_crit = m_WLS_exhume - m_maxBasalticCrustThickness * (m_constants.getBackstrippingMantleDensity() - m_magmaticDensity)
      / (m_constants.getBackstrippingMantleDensity() - m_constants.getWaterDensity());
   
   theFunction.setWLS_crit( m_WLS_crit );
   
   const double r = (m_TF_onset_lin == 1.0 ? 1.0 : sin( m_constants.getPi()
      * (1 - m_TF_onset_lin) ) / (m_constants.getPi() * (1 - m_TF_onset_lin)));

   m_WLS_onset = m_TF_onset_lin * m_constants.getInitialSubsidenceMax() + m_constants.getE0()
      * (r * (1 - exp( -t_mr / m_constants.getTau() )) + (pi2_8 * m_TF_onset_lin - r) * expValue);
   
   //  if( m_WLS_crit < m_WLS_onset )  return false;

   theFunction.setWLS_onset( m_WLS_onset );
   
   m_WLS_exhume_serp = m_WLS_exhume - 681.6394;
   
  // Step 4.4
  const double m1 = m_TF_onset_lin / m_WLS_onset; // form Y = m1 * X
  const double m2 = ( m_WLS_crit == m_WLS_onset ? 0 : (1 -  m_TF_onset_lin) / ( m_WLS_crit - m_WLS_onset )); // form Y = m2 * X + c2
  const double c2 = m_TF_onset_lin - m_WLS_onset * m2;

  if (m_maxBasalticCrustThickness != 0 && (m_constants.getBackstrippingMantleDensity() - m_magmaticDensity) == 0.0) {
     throw InputException() << "Backstripping Mantle density == Magmatic density but they should be !=";
  }     
  const double magmaThicknessCoeff = (m_constants.getBackstrippingMantleDensity() - m_constants.getWaterDensity())
     / (m_constants.getBackstrippingMantleDensity() - m_magmaticDensity);

  theFunction.setM1( m1 );
  theFunction.setM2( m2 );
  theFunction.setC2( c2 );
  theFunction.setMagmaThicknessCoeff( magmaThicknessCoeff );
  theFunction.setMaxBasalticCrustThickness( m_maxBasalticCrustThickness );
   
  return true;
}
//------------------------------------------------------------//
void InterfaceInput::retrieveData() {

   m_T0Map     ->retrieveData();
   m_TRMap     ->retrieveData();
   m_HCuMap    ->retrieveData();
   m_HLMuMap   ->retrieveData();
   m_HBuMap    ->retrieveData();
   m_DeltaSLMap->retrieveData();
}
//------------------------------------------------------------//
void InterfaceInput::restoreData() {

   m_T0Map     ->restoreData();
   m_TRMap     ->restoreData();
   m_HCuMap    ->restoreData();
   m_HLMuMap   ->restoreData();
   m_HBuMap    ->restoreData();
   m_DeltaSLMap->restoreData();
}

