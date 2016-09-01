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

// TableIO library
#include "cauldronschemafuncs.h"

// DataAccess library
#include "errorhandling.h"
#include "Interface/CrustFormation.h"
#include "Interface/Formation.h"
#include "Interface/MantleFormation.h"
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
   m_T0Map                      = nullptr;
   m_TRMap                      = nullptr;
   m_HCuMap                     = nullptr;
   m_HLMuMap                    = nullptr;
   m_HBuMap                     = nullptr;
   m_DeltaSLMap                 = nullptr;
   m_derivedManager             = nullptr;
   m_bottomOfSedimentSurface    = nullptr;
   m_topOfSedimentSurface       = nullptr;
   m_topOfMantle                = nullptr;
   m_botOfMantle                = nullptr;
   m_derivedManager             = nullptr;
   m_pressureBasement           = nullptr;
   m_pressureWaterBottom        = nullptr;
   m_pressureMantle             = nullptr;
   m_pressureMantleAtPresentDay = nullptr;
   m_depthBasement              = nullptr;
   m_depthWaterBottom           = nullptr;
   m_smoothRadius               = 0;
   m_t_felxural                 = 0.0;
   m_constants                  = CrustalThickness::ConfigFileParameterCtc();
   m_baseRiftSurfaceName        = "";
}

//------------------------------------------------------------//
InterfaceInput::~InterfaceInput() {
   if (m_derivedManager != nullptr){
      delete m_derivedManager;
   }
} 

//------------------------------------------------------------//
void InterfaceInput::loadInputDataAndConfigurationFile( const string & inFile ) {
   ///1. Load input data
   try {
      loadInputData();
   }
   catch (InputException& ex){
      LogHandler( LogHandler::ERROR_SEVERITY ) << "CTC error when loading input data";
      throw ex;
   }
   catch (...){
      LogHandler( LogHandler::FATAL_SEVERITY ) << "CTC fatal error when loading input data";
      throw;
   }
   ///2. Load configuration file
   try {
      m_constants.loadConfigurationFileCtc( inFile );
   }
   catch (InputException& ex){
      LogHandler( LogHandler::ERROR_SEVERITY ) << "CTC error when loading configuration file";
      throw ex;
   }
   catch (...){
      LogHandler( LogHandler::FATAL_SEVERITY ) << "CTC fatal error when loading configuration file";
      throw;
   }
}
//------------------------------------------------------------//
void InterfaceInput::loadInputData() {

   m_TRMap      = getMap (Interface::TRIni  );
   m_T0Map      = getMap (Interface::T0Ini  );
   m_HCuMap     = getMap (Interface::HCuIni );
   m_HLMuMap    = getMap (Interface::HLMuIni);
   m_HBuMap     = getMap (Interface::HBu    );
   m_DeltaSLMap = getMap (Interface::DeltaSL);
   m_baseRiftSurfaceName = getSurfaceName();
   m_smoothRadius        = getFilterHalfWidth();
   m_t_felxural          = getLastComputationAge();
   if (m_T0Map == nullptr or m_TRMap == nullptr or m_HCuMap == nullptr or m_HLMuMap == nullptr or m_DeltaSLMap == nullptr) {
      throw InputException() << "One of the input maps cannot be retreived by the interface input";
   }

}

//------------------------------------------------------------//
void InterfaceInput::loadDerivedPropertyManager(){
   if (m_derivedManager == nullptr){
      GeoPhysics::ProjectHandle* projectHandle = dynamic_cast<GeoPhysics::ProjectHandle*>(this->getProjectHandle());
      if (projectHandle == nullptr){
         throw InputException() << "Cannot access the derived property manager";
      }
      m_derivedManager = new DerivedProperties::DerivedPropertyManager( projectHandle );
      if (m_derivedManager == nullptr){
         throw InputException() << "Derived property manager cannot be retreived by the interface input";
      }
   }
   else{
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "Derived property manager already loaded";
   }
}

//------------------------------------------------------------//
void InterfaceInput::loadTopAndBottomOfSediments( GeoPhysics::ProjectHandle* projectHandle, const double snapshotAge, const string & baseSurfaceName ) {

   const Interface::Snapshot * currentSnapshot = projectHandle->findSnapshot( snapshotAge, MINOR | MAJOR );
   if (currentSnapshot == nullptr){
      throw InputException() << "Could not retrieve snapshot " << snapshotAge;
   }

   // - SEDIMENTS -
   //1. Find the bottom of the sediment
   //1.1 If the base of the rift is the base of the stratigraphy (ie. no crust in the stratigraphy)
   if (baseSurfaceName == "") {
      const Interface::CrustFormation * formationCrust  = projectHandle->getCrustFormation();
      if (formationCrust == nullptr) {
         throw InputException() << "Could not find Crust formation at the age " << currentSnapshot->getTime();
      }
      else{
         m_bottomOfSedimentSurface = formationCrust->getTopSurface();
      }
   }
   //1.2 If the base of the rift is the not base of the stratigraphy (ie. crust layers in the stratigraphy)
   // This is an R&D feature only which can be acctivated by edditing the project handle CTCIoTbl SurfaceName field
   else {
      m_bottomOfSedimentSurface = projectHandle->findSurface( baseSurfaceName );
      if (m_bottomOfSedimentSurface == nullptr) {
         throw InputException() << "Could not find user defined base surface of the rift event: '" << baseSurfaceName;
      }
   }

   //2. Find the top of the sediment
   Interface::FormationList * myFormations = projectHandle->getFormations( currentSnapshot, true );
   const Interface::Formation * formationWB = (*myFormations)[0]; // find Water bottom
   if (formationWB == nullptr) {
      throw InputException() << "Could not find the Water bottom formation at the age " << currentSnapshot->getTime();
   }
   else{
      m_topOfSedimentSurface = formationWB->getTopSurface();
   }


   // - MANTLE -
   const Interface::CrustFormation * formationMantle = projectHandle->getCrustFormation();
   if (formationMantle == nullptr) {
      throw InputException() << "Could not find Mantle formation at the age " << currentSnapshot->getTime();
   }
   else{
      //3. Find the top of the mantle
      m_topOfMantle = formationMantle->getTopSurface();
      //4. Find the bottom of the mantle
      m_botOfMantle = formationMantle->getBottomSurface();
   }

   if (m_bottomOfSedimentSurface == nullptr) {
      throw InputException() << "Could not find bottom sediment surface at the age " << currentSnapshot->getTime();
   }
   else if (m_topOfSedimentSurface == nullptr) {
      throw InputException() << "Could not find top sediment surface at the age " << currentSnapshot->getTime();
   }
   else if (m_topOfMantle == nullptr) {
      throw InputException() << "Could not find top mantle surface at the age " << currentSnapshot->getTime();
   }
   else if (m_botOfMantle == nullptr) {
      throw InputException() << "Could not find bottom mantle surface at the age " << currentSnapshot->getTime();
   }

   LogHandler( LogHandler::DEBUG_SEVERITY ) << "Loading surfaces for snapshot " << currentSnapshot->getTime() << ":";
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #Top sediment surface   "     << m_topOfSedimentSurface->getName();
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #Bot sediment surface   "     << m_bottomOfSedimentSurface->getName();
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #Top mantle surface     "     << m_topOfMantle->getName();
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #Bop mantle surface     "     << m_botOfMantle->getName();

}

//------------------------------------------------------------//
const DataModel::AbstractProperty* InterfaceInput::loadDepthProperty () {

   if (m_derivedManager == nullptr) loadDerivedPropertyManager();
   const DataModel::AbstractProperty* depthProperty = m_derivedManager->getProperty( "Depth" );
   if (depthProperty == nullptr) {
      throw InputException() << "Could not find property named Depth";
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
      if (m_depthBasement == nullptr or m_depthWaterBottom == nullptr){
         throw InputException() << "Could not create surface derived property objects for depth property";
      }
   }
   catch (...){
      LogHandler( LogHandler::FATAL_SEVERITY ) << "Could not load depth data for property " << depthProperty->getName() << " @ snapshot " << snapshotAge;
      throw;
   }
}

//------------------------------------------------------------//
const DataModel::AbstractProperty* InterfaceInput::loadPressureProperty () {

   if (m_derivedManager == nullptr) loadDerivedPropertyManager();
   const DataModel::AbstractProperty* pressureProperty = m_derivedManager->getProperty( "LithoStaticPressure" );
   if (pressureProperty == nullptr) {
      throw InputException() << "Could not find property named LithoStaticPressure";
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
      if (m_pressureBasement == nullptr or m_pressureWaterBottom == nullptr or m_pressureMantle == nullptr or m_pressureMantleAtPresentDay == nullptr){
         throw InputException() << "Could not create surface derived property objects for pressure property";
      }
   }
   catch (...){
      LogHandler( LogHandler::FATAL_SEVERITY ) << "Could not load pressure data for property " << pressureProperty->getName() << " @ snapshot " << snapshotAge;
      throw;
   }

}

//------------------------------------------------------------//
GridMap* InterfaceInput::loadPropertyDataFromDepthMap( DataAccess::Mining::ProjectHandle* handle,
                                                       const GridMap* depthMap,
                                                       const Interface::Property* property,
                                                       const Interface::Snapshot* snapshot ){

   if (handle == nullptr or depthMap == nullptr or property == nullptr or snapshot == nullptr){
      throw InputException() << "Could not load property " << property->getName() << " from depth map @ snapshot " <<
         snapshot->getTime() << " because one of the function inputs is a null pointer";
   }
   if (m_derivedManager == nullptr) loadDerivedPropertyManager();
   GridMap* outputPropertyMap = getFactory()->produceGridMap( 0, 0, handle->getActivityOutputGrid(), Interface::DefaultUndefinedMapValue, 1 );
   if (outputPropertyMap == nullptr){
      throw InputException() << "Could not create grid map to store property values";
   }
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
   const unsigned int firstI = depthMap->firstI();
   const unsigned int lastI  = depthMap->lastI();
   const unsigned int firstJ = depthMap->firstJ();
   const unsigned int lastJ  = depthMap->lastJ();
   const double deltaX = depthMap->deltaI();
   const double deltaY = depthMap->deltaJ();
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
const GridMap& InterfaceInput::getT0Map() const {
   if (m_T0Map == nullptr){
      throw InputException() << "Undefined starting rift age map (null pointer)";
   }
   return *m_T0Map;
}

//------------------------------------------------------------//
const GridMap& InterfaceInput::getTRMap() const {
   if (m_TRMap == nullptr){
      throw InputException() << "Undefined ending rift age map (null pointer)";
   }
   return *m_TRMap;
}

//------------------------------------------------------------//
const GridMap& InterfaceInput::getHCuMap() const {
   if (m_HCuMap == nullptr){
      throw InputException() << "Undefined initial crust thickness map (null pointer)";
   }
   return *m_HCuMap;
}

//------------------------------------------------------------//
const GridMap& InterfaceInput::getHBuMap() const {
   if (m_HBuMap == nullptr){
      throw InputException() << "Undefined maximum basalt thickness map (null pointer)";
   }
   return *m_HBuMap;
}

//------------------------------------------------------------//
const GridMap& InterfaceInput::getHLMuMap() const {
   if (m_HLMuMap == nullptr){
      throw InputException() << "Undefined initial lithospheric mantle thickness map (null pointer)";
   }
   return *m_HLMuMap;
}

//------------------------------------------------------------//
const GridMap& InterfaceInput::getDeltaSLMap() const {
   if (m_DeltaSLMap == nullptr){
      throw InputException() << "Undefined delta see level map (null pointer)";
   }
   return *m_DeltaSLMap;
}

