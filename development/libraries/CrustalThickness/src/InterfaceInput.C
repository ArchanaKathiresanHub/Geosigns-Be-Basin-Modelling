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

// DataAccess library
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
#include "NumericFunctions.h"
#include "LogHandler.h"
#include "StringHandler.h"

//------------------------------------------------------------//
InterfaceInput::InterfaceInput( const std::shared_ptr< const CrustalThicknessData>                            crustalThicknessData,
                                const std::vector<std::shared_ptr<const CrustalThicknessRiftingHistoryData>>& crustalThicknessRiftingHistoryData ) :
   m_crustalThicknessData              ( crustalThicknessData               ),
   m_crustalThicknessRiftingHistoryData( crustalThicknessRiftingHistoryData ),
   m_smoothRadius              (0  ),
   m_flexuralAge               (0.0),
   m_firstRiftAge              (0.0),
   m_HCuMap                    (nullptr),
   m_HLMuMap                   (nullptr),
   m_continentalCrustRatio     (0.0),
   m_oceanicCrustRatio         (0.0),
   m_bottomOfSedimentSurface   (nullptr),
   m_topOfSedimentSurface      (nullptr),
   m_topOfMantle               (nullptr),
   m_botOfMantle               (nullptr),
   m_derivedManager            (nullptr),
   m_pressureBasement          (nullptr),
   m_pressureBasementPresentDay(nullptr),
   m_pressureWaterBottom       (nullptr),
   m_pressureMantle            (nullptr),
   m_pressureMantleAtPresentDay(nullptr),
   m_depthBasement             (nullptr),
   m_depthWaterBottom          (nullptr),
   m_constants                 (CrustalThickness::ConfigFileParameterCtc()),
   m_baseRiftSurfaceName       ("")
{
   if (m_crustalThicknessData == nullptr){
      throw std::invalid_argument( "No crustal thickness data provided to the CTC" );
   }
   else if (m_crustalThicknessRiftingHistoryData.empty()){
      throw std::invalid_argument( "No crustal thickness rifting history data provided to the CTC" );
   }
   else{
      for (std::size_t i = 0; i < m_crustalThicknessRiftingHistoryData.size(); i++){
         if (m_crustalThicknessRiftingHistoryData[i] == nullptr){
            throw std::invalid_argument( "The crustal thickness rifting event data number " +
               std::to_string(i) + " provided to the CTC is corrupted" );
         }
      }
   }
}

//------------------------------------------------------------//
InterfaceInput::~InterfaceInput() {
   if (m_derivedManager != nullptr){
      delete m_derivedManager;
   }
} 

//------------------------------------------------------------//
void InterfaceInput::loadInputData( const string & inFile ) {
   ///1. Load configuration file
   LogHandler( LogHandler::INFO_SEVERITY ) << "   -> loading CTC configuration file";
   m_constants.loadConfigurationFileCtc( inFile );
   ///2. Load parameters input data
   LogHandler( LogHandler::INFO_SEVERITY ) << "   -> loading user input data from CTC Parameters Table";
   loadCTCIoTblData();
   ///3. Load history input data
   LogHandler( LogHandler::INFO_SEVERITY ) << "   -> loading user input data from CTC Rifting History Table";
   loadCTCRiftingHistoryIoTblData();
}

//------------------------------------------------------------//
void InterfaceInput::loadCTCIoTblData() {

   //UI inputs
   m_HCuMap       = m_crustalThicknessData->getMap (Interface::HCuIni );
   m_HLMuMap      = m_crustalThicknessData->getMap (Interface::HLMuIni);
   const int smoothingRadius = m_crustalThicknessData->getFilterHalfWidth();
   if (smoothingRadius < 0){
      throw std::invalid_argument( "The smoothing radius is set to a negative value" );
   }
   else{
      m_smoothRadius = (unsigned int)smoothingRadius;
   }

   //Debug R&D project file inputs
   m_continentalCrustRatio = m_crustalThicknessData->getUpperLowerContinentalCrustRatio();
   m_oceanicCrustRatio     = m_crustalThicknessData->getUpperLowerOceanicCrustRatio();
   m_baseRiftSurfaceName   = m_crustalThicknessData->getSurfaceName();

   if (m_HCuMap == nullptr){
      throw std::invalid_argument( "The initial crustal thickness map maps cannot be retreived by the interface input" );
   }
   else if (m_HLMuMap == nullptr) {
      throw std::invalid_argument( "The initial lithospheric mantle thickness map maps cannot be retreived by the interface input" );
   }
   else if ( m_continentalCrustRatio < 0 ) {
      throw std::invalid_argument( "The continental crust ratio (which defines the lower and upper continental crust) provided by the interface input is negative" );
   }
   else if ( m_oceanicCrustRatio < 0 ) {
      throw std::invalid_argument( "The oceanic crust ratio (which defines the lower and upper oceanic crust) provided by the interface input is negative" );
   }

}

//------------------------------------------------------------//
void InterfaceInput::loadCTCRiftingHistoryIoTblData(){
   loadSnapshots();
   loadRiftingEvents();
   analyseRiftingHistory();
}

//------------------------------------------------------------//
void InterfaceInput::loadRiftingEvents(){

   if (m_snapshots.size() != m_crustalThicknessRiftingHistoryData.size()){
      throw std::runtime_error( "The number of snpashots (" + std::to_string( m_snapshots.size() )
         + ") differ from the number of rifting events (" + std::to_string( m_crustalThicknessRiftingHistoryData.size() ) + ")" );
   }
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "      #" << std::setw( 15 ) << "Snapshot" << std::setw( 20 ) << "Tectonic Context";
   for (size_t i = 0; i < m_snapshots.size(); i++) {
      const double age = m_snapshots[i];
      const std::shared_ptr<const CrustalThicknessRiftingHistoryData> data = m_crustalThicknessRiftingHistoryData[i];
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "      #" << std::setw( 15 ) << age << "Ma" << std::setw( 20 ) << data->getTectonicFlagName();;
      GridMap const * const deltaSLMap( data->getMap( Interface::DeltaSL ));
      GridMap const * const hBuMap    ( data->getMap( Interface::HBu     ));
      m_riftingEvents[age] = std::shared_ptr<CrustalThickness::RiftingEvent>(
         new CrustalThickness::RiftingEvent( data->getTectonicFlag(), deltaSLMap, hBuMap )
      );
   }
}

//------------------------------------------------------------//
void InterfaceInput::analyseRiftingHistory(){
   analyseRiftingHistoryStartAge();
   analyseRiftingHistoryEndAge();
   LogHandler( LogHandler::INFO_SEVERITY ) << "      #"
                                           << std::setw( 15 ) << "Snapshot"
                                           << std::setw( 20 ) << "Tectonic Context"
                                           << std::setw( 10 ) << "Rift ID"
                                           << std::setw( 8  ) << "Start"
                                           << std::setw( 8  ) << "End";
   LogHandler( LogHandler::INFO_SEVERITY ) << "      #"
                                           << std::setw( 15 ) << "[Ma]"
                                           << std::setw( 20 ) << "[]"
                                           << std::setw( 10 ) << "[]"
                                           << std::setw( 8  ) << "[Ma]"
                                           << std::setw( 8  ) << "[Ma]";
   bool atLeastOneActiveEvent   = false;
   bool atLeastOnePassiveEvent  = false;
   bool atLeastOneFlexuralEvent = false;
   for (size_t i = 0; i < m_snapshots.size(); i++){
      const std::shared_ptr<const CrustalThickness::RiftingEvent> event = m_riftingEvents[m_snapshots[i]];
      const double start = m_riftingEvents[m_snapshots[i]]->getStartRiftAge();
      const double end   = m_riftingEvents[m_snapshots[i]]->getEndRiftAge();
      LogHandler( LogHandler::INFO_SEVERITY ) << "      #"
                                              << std::setw( 15 ) << m_snapshots[i]
                                              << std::setw( 20 ) << m_crustalThicknessRiftingHistoryData[i]->getTectonicFlagName()
                                              << std::setw( 10 ) << event->getRiftId()
                                              << std::setw( 8  ) << start
                                              << std::setw( 8  ) << end;
      // check that the timing is valid
      if (start  != DataAccess::Interface::DefaultUndefinedScalarValue
         and end != DataAccess::Interface::DefaultUndefinedScalarValue
         and start <= end){
         throw  std::invalid_argument( "The start of the rifting event " + std::to_string( start )
            + "Ma is anterior or equal to its end " + std::to_string( end ) + "Ma" );
      }
      // if this is flexural check that there was no active or passive event before
      if (event->getTectonicFlag() == DataAccess::Interface::FLEXURAL_BASIN){
         if (atLeastOneActiveEvent){
            throw std::invalid_argument( "An active rifting event is defined after a flexural event" );
         }
         else if (atLeastOnePassiveEvent){
            throw std::invalid_argument( "A passive margin event is defined after a flexural event" );
         }
         else if (not atLeastOneFlexuralEvent){
            atLeastOneFlexuralEvent = true;
         }
      }
      else if (event->getTectonicFlag() == DataAccess::Interface::ACTIVE_RIFTING){
         if (not atLeastOneActiveEvent){
            atLeastOneActiveEvent = true;
         }
      }
      else if (event->getTectonicFlag() == DataAccess::Interface::PASSIVE_MARGIN) {
         if (not atLeastOnePassiveEvent){
            atLeastOnePassiveEvent = true;
         }
      }
   }
   LogHandler( LogHandler::INFO_SEVERITY ) << "      #";
   LogHandler( LogHandler::INFO_SEVERITY ) << "      #" << "Flexural age " << m_flexuralAge;

   // check that there is at least one active rifting event defined
   if (not atLeastOneActiveEvent){
      throw std::invalid_argument( "There is no active rifting event defined in the rifting history" );
   }
   // check that there is at least one flexural event defined
   else if (not atLeastOneFlexuralEvent){
      throw std::invalid_argument( "There is no flexural event defined in the rifting history" );
   }

}

//------------------------------------------------------------//
void InterfaceInput::analyseRiftingHistoryStartAge(){
   unsigned int id = 0;
   double start = DataAccess::Interface::DefaultUndefinedScalarValue;
   bool isFirstActive = true;
   bool isFirstFlexural = true;
   std::shared_ptr<CrustalThickness::RiftingEvent> previousEvent = nullptr;
   assert( m_snapshots.size() > 0 );
   for (size_t i = 1; i <= m_snapshots.size(); i++){
      size_t index = m_snapshots.size() - i;
      const double age = m_snapshots[index];
      const std::shared_ptr<CrustalThickness::RiftingEvent> event = m_riftingEvents[age];
      ///1. If we are in Active Rifting we we check if this is the first Active Rifting event of the rift
      ///      and if it is the first global Active Rifting (which defines the first rift to be computed)
      if ( event->getTectonicFlag() == DataAccess::Interface::ACTIVE_RIFTING ){
         if ( isFirstActive ){
            m_firstRiftAge = age;
            isFirstActive = false;
         }
         // if there is no previous event (first rifting) or if the next event is passive
         //    then defined the start age to be the age of the current event
         if ( previousEvent == nullptr
              or previousEvent->getTectonicFlag() == DataAccess::Interface::PASSIVE_MARGIN ){
            start = age;
            id++;
         }
      }
      ///2. Else if we are in Flexural Basin we check if this is the first Flexural Basin event
      else if ( event->getTectonicFlag() == DataAccess::Interface::FLEXURAL_BASIN ){
         // if this is the first flexural event we set the last computation age
         //    and the start age of the event is the same as the start age of the previous event
         if (isFirstFlexural){
            m_flexuralAge = age;
            isFirstFlexural = false;
         }
         // else this is not the first flexural event and there are no computations to be done at this age
         //    we set the start age of the event to undefined as it is not part of a rift
         else{
            id = (unsigned int)DataAccess::Interface::DefaultUndefinedMapValueInteger;
            start = DataAccess::Interface::DefaultUndefinedScalarValue;
         }
      }
      event->setStartRiftAge( start );
      event->setRiftId( id );
      previousEvent = event;
   }
}

//------------------------------------------------------------//
void InterfaceInput::analyseRiftingHistoryEndAge(){
   double end = DataAccess::Interface::DefaultUndefinedScalarValue;
   std::shared_ptr<CrustalThickness::RiftingEvent> nextEvent = nullptr;
   std::vector<std::shared_ptr<CrustalThickness::RiftingEvent>> eventsToUpdate;
   assert( m_snapshots.size() > 0 );
   for (size_t i = 0; i < m_snapshots.size(); i++){
      const double age = m_snapshots[i];
      const std::shared_ptr<CrustalThickness::RiftingEvent> event = m_riftingEvents[age];

      ///1. If we are in Passive Margin we look for the next Active Rifting and update the end ages
      ///      according to the end age of this Active Rifting
      if (event->getTectonicFlag() == DataAccess::Interface::PASSIVE_MARGIN ){
         eventsToUpdate.push_back(event);
         // if there is a next event find the first active event of the rift
         if ( i < m_snapshots.size() - 1){
            nextEvent = m_riftingEvents[m_snapshots[i + 1]];
            // first active event is found, the end age of the rift is its age
            if (nextEvent->getTectonicFlag() == DataAccess::Interface::ACTIVE_RIFTING){
               end = age;
               std::for_each( eventsToUpdate.begin(), eventsToUpdate.end(),
                  [&]( std::shared_ptr<CrustalThickness::RiftingEvent> item ){ item->CrustalThickness::RiftingEvent::setEndRiftAge( end ); } );
               eventsToUpdate.clear();
            }
            else{
               // continue until we find the next Active Rifting
               eventsToUpdate.push_back( nextEvent );
            }
         }
         // else there is no next event so we do not know when is the first active event of the rift
         else{
            end = DataAccess::Interface::DefaultUndefinedScalarValue;
            std::for_each( eventsToUpdate.begin(), eventsToUpdate.end(),
               [&]( std::shared_ptr<CrustalThickness::RiftingEvent> item ){ item->CrustalThickness::RiftingEvent::setEndRiftAge( end ); } );
            eventsToUpdate.clear();
         }
      }

      /// 2. Else if we are in Flexural Basin we look for the nature of the next event
      else if (event->getTectonicFlag() == DataAccess::Interface::FLEXURAL_BASIN){
         // if there is a next event find its nature
         if (i < m_snapshots.size() - 1){
            nextEvent = m_riftingEvents[m_snapshots[i + 1]];
            // if the next event is a passive then the current event is the first flexural event
            //    and its end age is the one of the next active event
            if (nextEvent->getTectonicFlag() == DataAccess::Interface::PASSIVE_MARGIN){
               eventsToUpdate.push_back( event );
            }
            // else if the next event is a active then the current event is the first flexural event
            //    and its end age is the one of this active event
            else if (nextEvent->getTectonicFlag() == DataAccess::Interface::ACTIVE_RIFTING){
               end = age;
               event->setEndRiftAge( end );
            }
            // else the next event is also flexural so the current event is not the first flexural event
            //    and its end age is undefined
            else{
               event->setEndRiftAge( DataAccess::Interface::DefaultUndefinedScalarValue );
            }
         }
      }

      /// 3. Else we are in Active Rifting then the end age is the same as the one in the previous event
      else{
         event->setEndRiftAge( end );
      }
   }
}

//------------------------------------------------------------//
double InterfaceInput::getRiftingStartAge( const double age ) const{
   auto iterator = m_riftingEvents.find(age);
   double riftinStartAge = DataAccess::Interface::DefaultUndefinedScalarValue;
   if (iterator != m_riftingEvents.end()){
      if (iterator->second != nullptr){
         riftinStartAge = iterator->second->getStartRiftAge();
      }
      else{
         throw std::runtime_error( "The rifting event defined at " + std::to_string( age ) + "Ma is corrupted" );
      }
   }
   else{
      throw std::runtime_error( "There is no rifting event defined at " + std::to_string( age ) + "Ma" );
   }
   return riftinStartAge;
}

//------------------------------------------------------------//
double InterfaceInput::getRiftingEndAge( const double age ) const{
   auto iterator = m_riftingEvents.find( age );
   double riftinEndAge = DataAccess::Interface::DefaultUndefinedScalarValue;
   if (iterator != m_riftingEvents.end()){
      if (iterator->second != nullptr){
         riftinEndAge = iterator->second->getEndRiftAge();
      }
      else{
         throw std::runtime_error( "The rifting event defined at " + std::to_string( age ) + "Ma is corrupted" );
      }
   }
   else{
      throw std::runtime_error( "There is no rifting event defined at " + std::to_string( age ) + "Ma" );
   }
   return riftinEndAge;
}

//------------------------------------------------------------//
double InterfaceInput::getRiftId( const double age ) const{
   auto iterator = m_riftingEvents.find( age );
   double riftID = DataAccess::Interface::DefaultUndefinedScalarValue;
   if (iterator != m_riftingEvents.end()){
      if (iterator->second != nullptr){
         riftID = iterator->second->getRiftId();
      }
      else{
         throw std::runtime_error( "The rifting event defined at " + std::to_string( age ) + "Ma is corrupted" );
      }
   }
   else{
      throw std::runtime_error( "There is no rifting event defined at " + std::to_string( age ) + "Ma" );
   }
   return riftID;
}

//------------------------------------------------------------//
const GridMap& InterfaceInput::getHBuMap( const double age ) const{
   auto iterator = m_riftingEvents.find( age );
   GridMap const * map = nullptr;
   if (iterator != m_riftingEvents.end()){
      if (iterator->second != nullptr){
         map = iterator->second->getMaximumOceanicCrustThickness();
      }
      else{
         throw std::runtime_error( "The rifting event defined at " + std::to_string( age ) + "Ma is corrupted" );
      }
   }
   else{
      throw std::runtime_error( "There is no rifting event defined at " + std::to_string( age ) + "Ma" );
   }
   if (map == nullptr){
      // should never happen as it is already tested in RiftingEvents
      throw std::runtime_error( "There is no maximum oceanic crustal thickness defined for the rifting event at " + std::to_string( age ) + "Ma" );
   }
   return *map;
}

//------------------------------------------------------------//
const GridMap& InterfaceInput::getDeltaSLMap( const double age ) const{
   auto iterator = m_riftingEvents.find( age );
   GridMap const * map = nullptr;
   if (iterator != m_riftingEvents.end()){
      if (iterator->second != nullptr){
         map = iterator->second->getSeaLevelAdjustment();
      }
      else{
         throw std::runtime_error( "The rifting event defined at " + std::to_string( age ) + "Ma is corrupted" );
      }
   }
   else{
      throw std::runtime_error( "There is no rifting event defined at " + std::to_string( age ) + "Ma" );
   }
   if (map == nullptr){
      // should never happen as it is already tested in RiftingEvents
      throw std::runtime_error( "There is no sea lvel adjustment defined for the rifting event at " + std::to_string( age ) + "Ma" );
   }
   return *map;
}

//------------------------------------------------------------//
void InterfaceInput::loadSnapshots() {

   m_snapshots.clear();
   m_snapshots = m_crustalThicknessData->getSnapshots();

}

//------------------------------------------------------------//
void InterfaceInput::loadDerivedPropertyManager(){
   if (m_derivedManager == nullptr){
      GeoPhysics::ProjectHandle* projectHandle = dynamic_cast<GeoPhysics::ProjectHandle*>(m_crustalThicknessData->getProjectHandle());
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
   const Interface::MantleFormation * formationMantle = projectHandle->getMantleFormation();
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
   LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #Bot mantle surface     "     << m_botOfMantle->getName();

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
      m_pressureBasement           = m_derivedManager->getSurfaceProperty( pressureProperty, currentSnapshot,    m_bottomOfSedimentSurface );
      m_pressureBasementPresentDay = m_derivedManager->getSurfaceProperty( pressureProperty, presentDaySnapshot, m_bottomOfSedimentSurface );
      // Find the pressure property of the top of sediment
      m_pressureWaterBottom = m_derivedManager->getSurfaceProperty( pressureProperty, currentSnapshot, m_topOfSedimentSurface );
      // Find the pressure property of the bottom of the mantle
      m_pressureMantle             = m_derivedManager->getSurfaceProperty( pressureProperty, currentSnapshot,    m_botOfMantle );
      m_pressureMantleAtPresentDay = m_derivedManager->getSurfaceProperty( pressureProperty, presentDaySnapshot, m_botOfMantle );
      if (m_pressureBasement == nullptr or m_pressureWaterBottom == nullptr){
         throw InputException() << "Could not create surface derived property objects for pressure property";
      }
   }
   catch (...){
      LogHandler( LogHandler::FATAL_SEVERITY ) << "Could not load pressure data for property " << pressureProperty->getName() << " @ snapshot " << snapshotAge;
      throw;
   }

}

//------------------------------------------------------------//
const GridMap& InterfaceInput::getHCuMap() const {
   if (m_HCuMap == nullptr){
      throw std::runtime_error( "Undefined initial crust thickness map" );
   }
   return *m_HCuMap;
}

//------------------------------------------------------------//
const GridMap& InterfaceInput::getHLMuMap() const {
   if (m_HLMuMap == nullptr){
      throw std::runtime_error( "Undefined initial lithospheric mantle thickness map" );
   }
   return *m_HLMuMap;
}

