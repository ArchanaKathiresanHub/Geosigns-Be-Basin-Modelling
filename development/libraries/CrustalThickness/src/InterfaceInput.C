//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "InterfaceInput.h"

// CrustalThickness library
#include "LinearFunction.h"
#include "RiftingEvent.h"

// DataAccess library
#include "Interface/Interface.h"
#include "Interface/CrustalThicknessData.h"
#include "Interface/CrustalThicknessInterface.h"
#include "Interface/CrustalThicknessRiftingHistoryData.h"
#include "Interface/Formation.h"
#include "Interface/CrustFormation.h"
#include "Interface/MantleFormation.h"

// GeoPhysics library
#include "GeoPhysicsProjectHandle.h"

// DataMining library
#include "CauldronDomain.h"
#include "DataMiner.h"
#include "ElementPosition.h"
#include "InterpolatedPropertyValues.h"
#include "DataMiningProjectHandle.h"

// DataModel library
#include "AbstractProperty.h"

// DerivedProperties library
#include "DerivedPropertyManager.h"

// utility
#include "NumericFunctions.h"
#include "LogHandler.h"
#include "StringHandler.h"
#include "ConstantsNumerical.h"

// Geophysics library
#include "GeoPhysicsProjectHandle.h"

using DataAccess::Interface::FLEXURAL_BASIN;
using DataAccess::Interface::ACTIVE_RIFTING;
using DataAccess::Interface::PASSIVE_MARGIN;
using Utilities::Numerical::IbsNoDataValue;
using Utilities::Numerical::UnsignedIntNoDataValue;
using DataAccess::Interface::MINOR;
using DataAccess::Interface::MAJOR;

const string InterfaceInput::s_ctcConfigurationFile = "InterfaceData.cfg";

//------------------------------------------------------------//
InterfaceInput::InterfaceInput( const std::shared_ptr< const CrustalThicknessData>                            crustalThicknessData,
                                const std::vector<std::shared_ptr<const CrustalThicknessRiftingHistoryData>>& crustalThicknessRiftingHistoryData ) :
   m_crustalThicknessData              ( crustalThicknessData               ),
   m_crustalThicknessRiftingHistoryData( crustalThicknessRiftingHistoryData ),
   m_smoothRadius              (0  ),
   m_flexuralAge               (0.0),
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
      throw std::invalid_argument( "Basin_Error: No crustal thickness data provided to the CTC" );
   }
   else if (m_crustalThicknessRiftingHistoryData.empty()){
      throw std::invalid_argument( "Basin_Error: No crustal thickness rifting history data provided to the CTC" );
   }
   else{
      for (std::size_t i = 0; i < m_crustalThicknessRiftingHistoryData.size(); i++){
         if (m_crustalThicknessRiftingHistoryData[i] == nullptr){
            throw std::invalid_argument( "Basin_Error: The crustal thickness rifting event data number " +
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
void InterfaceInput::loadInputData() {
   ///1. Load configuration file
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "loading CTC configuration file";
   m_constants.loadConfigurationFileCtc( s_ctcConfigurationFile );
   ///2. Load parameters input data
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "loading user input data from CTC Parameters Table";
   loadCTCIoTblData();
   ///3. Load history input data
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "loading user input data from CTC Rifting History Table";
   loadCTCRiftingHistoryIoTblData();
}

//------------------------------------------------------------//
void InterfaceInput::loadSurfaceDepthHistoryMask( GeoPhysics::ProjectHandle * projectHandle ) {
   if (m_snapshots.empty()) {
      throw std::invalid_argument("Basin_Error: Could not retrieve surface depth history because the snapshots were not loaded");
   }
   else {
      std::for_each( m_snapshots.begin(), m_snapshots.end(), [&](const double age) {
         m_hasSurfaceDepthHistory[age] = projectHandle->hasSurfaceDepthHistory( age );
      } );
   }
}

//------------------------------------------------------------//
void InterfaceInput::loadCTCIoTblData() {

   //UI inputs
   m_HCuMap       = m_crustalThicknessData->getMap (DataAccess::Interface::HCuIni );
   m_HLMuMap      = m_crustalThicknessData->getMap (DataAccess::Interface::HLMuIni);
   const int smoothingRadius = m_crustalThicknessData->getFilterHalfWidth();
   if (smoothingRadius < 0){
      throw std::invalid_argument( "Basin_Error: The smoothing radius is set to a negative value" );
   }
   else{
      m_smoothRadius = static_cast<unsigned int>(smoothingRadius);
   }

   //Debug R&D project file inputs
   m_continentalCrustRatio = m_crustalThicknessData->getUpperLowerContinentalCrustRatio();
   m_oceanicCrustRatio     = m_crustalThicknessData->getUpperLowerOceanicCrustRatio();
   m_baseRiftSurfaceName   = m_crustalThicknessData->getSurfaceName();

   if (m_HCuMap == nullptr){
      throw std::invalid_argument( "Basin_Error: The initial crustal thickness map maps cannot be retreived by the interface input" );
   }
   else if (m_HLMuMap == nullptr) {
      throw std::invalid_argument( "Basin_Error: The initial lithospheric mantle thickness map maps cannot be retreived by the interface input" );
   }
   else if ( m_continentalCrustRatio < 0 ) {
      throw std::invalid_argument( "Basin_Error: The continental crust ratio (which defines the lower and upper continental crust) provided by the interface input is negative" );
   }
   else if ( m_oceanicCrustRatio < 0 ) {
      throw std::invalid_argument( "Basin_Error: The oceanic crust ratio (which defines the lower and upper oceanic crust) provided by the interface input is negative" );
   }

}

//------------------------------------------------------------//
void InterfaceInput::loadCTCRiftingHistoryIoTblData(){
   loadRiftingEvents();
   analyseRiftingHistory();
}

//------------------------------------------------------------//
void InterfaceInput::loadRiftingEvents(){

   if (m_snapshots.size() != m_crustalThicknessRiftingHistoryData.size()){
      throw std::runtime_error( "Basin_Error: The number of snpashots (" + std::to_string( m_snapshots.size() )
         + ") differ from the number of rifting events (" + std::to_string( m_crustalThicknessRiftingHistoryData.size() ) + ")" );
   }
   LogHandler( LogHandler::DEBUG_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << std::setw( 15 ) << "Snapshot" << std::setw( 20 ) << "Tectonic Context";
   size_t index = 0;
   std::for_each( m_snapshots.rbegin(), m_snapshots.rend(), [&]( const double age ) {
      const std::shared_ptr<const CrustalThicknessRiftingHistoryData> data = m_crustalThicknessRiftingHistoryData[index];
      LogHandler( LogHandler::DEBUG_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << std::setw( 15 ) << age << "Ma" << std::setw( 20 ) << data->getTectonicFlagName();;
      GridMap const * const deltaSLMap( data->getMap( DataAccess::Interface::DeltaSL ) );
      GridMap const * const hBuMap( data->getMap( DataAccess::Interface::HBu ) );
      m_riftingEvents[age] = std::shared_ptr<CrustalThickness::RiftingEvent>(
         new CrustalThickness::RiftingEvent( data->getTectonicFlag(), deltaSLMap, hBuMap )
      );
      index++;
   } );
}

//------------------------------------------------------------//
void InterfaceInput::analyseRiftingHistory(){
   analyzeRiftingHistoryCalculationMask();
   analyseRiftingHistoryStartAge ();
   analyseRiftingHistoryEndAge();
   printRiftingHistory();
   checkRiftingHistory();
}

//------------------------------------------------------------//
void InterfaceInput::analyzeRiftingHistoryCalculationMask() {
   bool firstFlexuralEventFound = false;
   assert( m_snapshots.size() > 0 );
   for (size_t i = 0; i < m_snapshots.size(); i++) {
      bool mask = false;
      const double age = m_snapshots[i];
      const std::shared_ptr<CrustalThickness::RiftingEvent> event = m_riftingEvents[age];
      // RULE_ID #2 The first Flexural event (0Ma by default) is a calculation age, and must have an SDH according to RULE_ID #1
      if (event->getTectonicFlag() == FLEXURAL_BASIN and not firstFlexuralEventFound) {
         firstFlexuralEventFound = true;
         mask = true;
      }
      // RULE_ID #3 An Active event with a SDH (according to RULE_ID #1) which follows another Active or Passive event is a calculation age
      //    if this is not the basement age (according to RULE_ID #5)
      else if (event->getTectonicFlag() == ACTIVE_RIFTING and m_hasSurfaceDepthHistory.at(age) and i!=0) {
         const double prevAge = m_snapshots[i-1];
         const std::shared_ptr<CrustalThickness::RiftingEvent> prevEvent = m_riftingEvents[prevAge];
         if (prevEvent->getTectonicFlag() == ACTIVE_RIFTING or prevEvent->getTectonicFlag() == PASSIVE_MARGIN) {
            mask = true;
         }
      }
      // RULE_ID #4 A Passive event, Active event but without SDH,
      // and a Flexural event which is not the first flexural event can never be a calculation age
      else {
         mask = false;
      }
      event->setCalculationMask( mask );
   }
}

//------------------------------------------------------------//
void InterfaceInput::analyseRiftingHistoryStartAge(){
   unsigned int id = 0;
   double start = IbsNoDataValue, age = IbsNoDataValue;
   bool firstFlexuralEventFound = false;
   assert( m_snapshots.size() > 0 );
   for (size_t i = 0; i < m_snapshots.size(); i++) {
      age = m_snapshots[i];
      const std::shared_ptr<CrustalThickness::RiftingEvent> event = m_riftingEvents[age];

      if (event->getTectonicFlag() == FLEXURAL_BASIN and not firstFlexuralEventFound) {
         firstFlexuralEventFound = true;
      }
      // RULE_ID #6 An Active event with a SDH is a starting rifting age
      // by default we assume that the basement has an SDH of 0 if not user defined
      else if (event->getTectonicFlag() == ACTIVE_RIFTING and not firstFlexuralEventFound) {
         if (m_hasSurfaceDepthHistory.at( age ) and i!=0) {
            //first set the rift age and id to the one of the ending rift
            event->setStartRiftAge( start );
            event->setRiftId( id );
            //then update the age and id for the other beggining rift
            start = age;
            id++;
            continue;
         }
         // the first event is always the start of the first rift
         else if (i == 0) {
            start = age;
            id++;
         }
         else {
            assert( i > 0 );
            const double prevAge = m_snapshots[i - 1];
            const std::shared_ptr<CrustalThickness::RiftingEvent> prevEvent = m_riftingEvents[prevAge];
            // RULE_ID #15 Error when there are no SDH defined at the beginning of a rifting event
            if (prevEvent->getTectonicFlag() == PASSIVE_MARGIN) {
               throw std::invalid_argument( "Basin_Error: The begining of rift ID " + std::to_string( id ) +
                  " at age " + std::to_string( age ) + " does not have any surface depth history associated" );
            }
         }
      }
      // after the first flexural event, there is no more rift
      else if (firstFlexuralEventFound) {
         start = IbsNoDataValue;
         id    = UnsignedIntNoDataValue;
      }
      event->setStartRiftAge( start );
      event->setRiftId      ( id    );
   }
}

//------------------------------------------------------------//
void InterfaceInput::analyseRiftingHistoryEndAge(){
   double end = IbsNoDataValue;
   bool firstFlexuralEventFound = false;
   std::shared_ptr<CrustalThickness::RiftingEvent> nextEvent = nullptr;
   std::vector<std::shared_ptr<CrustalThickness::RiftingEvent>> eventsToUpdate;
   assert( m_snapshots.size() > 0 );
   for (size_t i = 0; i < m_snapshots.size(); i++){
      const double age = m_snapshots[i];
      const std::shared_ptr<CrustalThickness::RiftingEvent> event = m_riftingEvents[age];
      // RULE_ID #8 An Active event with a SDH which follows another Active event is an End Age
      const bool activeEnd   = event->getTectonicFlag() == ACTIVE_RIFTING and m_hasSurfaceDepthHistory.at(age) and i != 0;
      // RULE_ID #9 A Passive event with a previous event being Active is an End Age
      const bool passiveEnd  = event->getTectonicFlag() == PASSIVE_MARGIN and i != 0;
      // RULE_ID #10 The first Flexural event (0Ma by default), if it follows an Active event, is an End Age
      const bool flexuralEnd = event->getTectonicFlag() == FLEXURAL_BASIN and not firstFlexuralEventFound and i!= 0;
      if (activeEnd or passiveEnd or flexuralEnd) {
         const double prevAge = m_snapshots[i - 1];
         const std::shared_ptr<CrustalThickness::RiftingEvent> prevEvent = m_riftingEvents[prevAge];
         eventsToUpdate.push_back( event );
         // if the previous event was active, then we found the end of the rift
         if (prevEvent->getTectonicFlag() == ACTIVE_RIFTING) {
            end = age;
            std::for_each( eventsToUpdate.begin(), eventsToUpdate.end(),
               [&]( std::shared_ptr<CrustalThickness::RiftingEvent> item ) { item->setEndRiftAge( end ); } );
            eventsToUpdate.clear();
         }
         // if the previous event was passive, then the end age stays the same
         else if (prevEvent->getTectonicFlag() == PASSIVE_MARGIN) {
            std::for_each( eventsToUpdate.begin(), eventsToUpdate.end(),
               [&]( std::shared_ptr<CrustalThickness::RiftingEvent> item ) { item->setEndRiftAge( end ); } );
            eventsToUpdate.clear();
         }
      }
      // after the first flexural event, there is no more rift
      else if (firstFlexuralEventFound) {
         event->setEndRiftAge( IbsNoDataValue );
      }
      // Else, we need to update the events when we will know their end ages
      else{
         eventsToUpdate.push_back(event);
      }

      // Sets the flexural age to the first flexural event
      if (flexuralEnd) {
         m_flexuralAge = age;
      }
      firstFlexuralEventFound = firstFlexuralEventFound or flexuralEnd;
   }
}

//------------------------------------------------------------//
void InterfaceInput::checkRiftingHistory() const {

   bool atLeastOneActiveEvent   = false;
   bool atLeastOnePassiveEvent  = false;
   bool atLeastOneFlexuralEvent = false;
   const double basementAge = m_snapshots[0];

   // RULE_ID #13 The first deposited formation (at basement age) is not an active event
   if (m_riftingEvents.at(basementAge)->getTectonicFlag() != ACTIVE_RIFTING) {
      throw std::invalid_argument( "Basin_Error: The first rifting event is not an active rifting" );
   }

   std::shared_ptr<CrustalThickness::RiftingEvent> prevEvent = nullptr;
   for (size_t i = 0; i < m_snapshots.size(); i++) {
      const std::shared_ptr<const CrustalThickness::RiftingEvent> event = m_riftingEvents.at(m_snapshots[i]);
      if (i != 0) prevEvent = m_riftingEvents.at(m_snapshots[i - 1]);
      const double start = event->getStartRiftAge();
      const double end = event->getEndRiftAge();
      // check that the timing is valid
      if (   start != IbsNoDataValue
         and end   != IbsNoDataValue
         and start <= end) {
         // this should not happen as it is already checked when we set the ages
         throw  std::invalid_argument( "Basin_Error: The start of the rifting event " + std::to_string( start )
            + "Ma is anterior or equal to its end " + std::to_string( end ) + "Ma" );
      }
      // if this is flexural check that there was no active or passive event before
      if (event->getTectonicFlag() == FLEXURAL_BASIN and not atLeastOneFlexuralEvent) {
         atLeastOneFlexuralEvent = true;
         // RULE_ID #11 Error when the first flexural event doesn't have an SDH
         if (not m_hasSurfaceDepthHistory.at( m_snapshots[i] )) {
            throw std::invalid_argument( "Basin_Error: There is no surface depth history defined for the first flexural event" );
         }
      }
      else if (event->getTectonicFlag() == ACTIVE_RIFTING) {
         // RULE_ID #14 There are post Flexural Active / Passive events
         if (atLeastOneFlexuralEvent) {
            throw std::invalid_argument( "Basin_Error: An active rifting event is defined after a flexural event" );
         }
         else if (not atLeastOneActiveEvent) {
            atLeastOneActiveEvent = true;
         }
      }
      else if (event->getTectonicFlag() == PASSIVE_MARGIN) {
         // RULE_ID #14 There are post Flexural Active / Passive events
         if (atLeastOneFlexuralEvent) {
            throw std::invalid_argument( "Basin_Error: A passive margin event is defined after a flexural event" );
         }
         else if (not atLeastOnePassiveEvent) {
            atLeastOnePassiveEvent = true;
         }
      }
      // check that the maximum basalt thickness is constant within one rift (only if the value is constant, we do not check for maps)
      // RULE_ID #11 Only one Maximum Basalt Thickness value can be allowed in one rift
      if (prevEvent != nullptr and prevEvent->getRiftId() == event->getRiftId() and prevEvent->getRiftId() != UnsignedIntNoDataValue) {
         if (prevEvent->getMaximumOceanicCrustThickness()->getConstantValue() != event->getMaximumOceanicCrustThickness()->getConstantValue()) {
            throw std::invalid_argument( "Basin_Error: Only one Maximum Oceanic Thickness value can be allowed within a rift" );
         }
      }

   }

   // check that there is at least one active rifting event defined
   if (not atLeastOneActiveEvent) {
      // RULE_ID #16 There is no rift defined (no active event), in principle RULE ID #13 will catch this error before
      throw std::invalid_argument( "Basin_Error: There is no active rifting event defined in the rifting history" );
   }
   // check that there is at least one flexural event defined
   else if (not atLeastOneFlexuralEvent) {
      // RULE_ID #17 Error when there is no Flexural event
      throw std::invalid_argument( "Basin_Error: There is no flexural event defined in the rifting history, at least the present day event has to be set to flexural" );
   }
}

//------------------------------------------------------------//
void InterfaceInput::printRiftingHistory() const {
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP )
      << std::setw( 15 ) << "Snapshot"
      << std::setw( 20 ) << "Tectonic Context"
      << std::setw( 10 ) << "Rift ID"
      << std::setw( 8  ) << "Start"
      << std::setw( 8  ) << "End"
      << std::setw( 8  ) << "SDH";
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP )
      << std::setw( 15 ) << "[Ma]"
      << std::setw( 20 ) << "[]"
      << std::setw( 10 ) << "[]"
      << std::setw( 8  ) << "[Ma]"
      << std::setw( 8  ) << "[Ma]"
      << std::setw( 8  ) << "[]";

   unsigned int index = 0;
   std::for_each( m_snapshots.rbegin(), m_snapshots.rend(), [&]( const double age ) {

      const std::shared_ptr<const CrustalThickness::RiftingEvent> event = m_riftingEvents.at( age );
      const double start        = event->getStartRiftAge();
      const double end          = event->getEndRiftAge();
      const unsigned int riftId = event->getRiftId();

      std::string startString  = (start  == IbsNoDataValue         ? "NA" : std::to_string( start  ));
      std::string endString    = (end    == IbsNoDataValue         ? "NA" : std::to_string( end    ));

      //remove useless 0 (60.000000 --> 60.)
      startString.erase ( startString.find_last_not_of ( '0' ) + 1, std::string::npos );
      endString.erase   ( endString.find_last_not_of   ( '0' ) + 1, std::string::npos );

      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP )
         << std::setw( 15 ) << age
         << std::setw( 20 ) << m_crustalThicknessRiftingHistoryData[index]->getTectonicFlagName()
         << std::setw( 10 ) << (riftId == UnsignedIntNoDataValue ? "NA" : std::to_string( riftId ))
         << std::setw( 8  ) << startString
         << std::setw( 8  ) << endString
         << std::setw( 8  ) << m_hasSurfaceDepthHistory.at( age );
      index++;

   } );

   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "";
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "Flexural age " << m_flexuralAge;
}

//------------------------------------------------------------//
bool InterfaceInput::getRiftingCalculationMask( const double age ) const {
   auto event = getRiftEvent( age );
   return event->getCalculationMask();
}

//------------------------------------------------------------//
double InterfaceInput::getRiftingStartAge( const double age ) const{
   auto event = getRiftEvent( age );
   return event->getStartRiftAge();
}

//------------------------------------------------------------//
double InterfaceInput::getRiftingEndAge( const double age ) const{
   auto event = getRiftEvent( age );
   return event->getEndRiftAge();
}

//------------------------------------------------------------//
double InterfaceInput::getRiftId( const double age ) const{
   auto event = getRiftEvent( age );
   return event->getRiftId();
}

//------------------------------------------------------------//
const GridMap& InterfaceInput::getHBuMap( const double age ) const{
   auto event = getRiftEvent( age );
   GridMap const * map = event->getMaximumOceanicCrustThickness();
   if (map == nullptr){
      // should never happen as it is already tested in RiftingEvents
      throw std::runtime_error( "Basin_Error: There is no maximum oceanic crustal thickness defined for the rifting event at " + std::to_string( age ) + "Ma" );
   }
   return *map;
}

//------------------------------------------------------------//
const GridMap& InterfaceInput::getDeltaSLMap( const double age ) const{
   auto event = getRiftEvent( age );
   GridMap const * map = event->getSeaLevelAdjustment();
   if (map == nullptr){
      // should never happen as it is already tested in RiftingEvents
      throw std::runtime_error( "Basin_Error: There is no sea level adjustment defined for the rifting event at " + std::to_string( age ) + "Ma" );
   }
   return *map;
}

//------------------------------------------------------------//
void InterfaceInput::loadSnapshots() {

   m_snapshots.clear();
   m_snapshots = m_crustalThicknessData->getSnapshots();
   std::sort( m_snapshots.begin(), m_snapshots.end(), std::greater<double>() );

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

   const DataAccess::Interface::Snapshot * currentSnapshot = projectHandle->findSnapshot( snapshotAge, MINOR | MAJOR );
   if (currentSnapshot == nullptr){
      throw InputException() << "Could not retrieve snapshot " << snapshotAge;
   }

   // - SEDIMENTS -
   //1. Find the bottom of the sediment
   //1.1 If the base of the rift is the base of the stratigraphy (ie. no crust in the stratigraphy)
   if (baseSurfaceName == "") {
      const DataAccess::Interface::CrustFormation * formationCrust  = projectHandle->getCrustFormation();
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
   DataAccess::Interface::FormationList * myFormations = projectHandle->getFormations( currentSnapshot, true );
   const DataAccess::Interface::Formation * formationWB = (*myFormations)[0]; // find Water bottom
   if (formationWB == nullptr) {
      throw InputException() << "Could not find the Water bottom formation at the age " << currentSnapshot->getTime();
   }
   else{
      m_topOfSedimentSurface = formationWB->getTopSurface();
   }


   // - MANTLE -
   const DataAccess::Interface::MantleFormation * formationMantle = projectHandle->getMantleFormation();
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
   const DataAccess::Interface::Snapshot * currentSnapshot = projectHandle->findSnapshot( snapshotAge, MINOR | MAJOR );
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
   const DataAccess::Interface::Snapshot * currentSnapshot    = projectHandle->findSnapshot( snapshotAge, MINOR | MAJOR );
   const DataAccess::Interface::Snapshot * presentDaySnapshot = projectHandle->findSnapshot( 0.0,         MINOR | MAJOR );
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
      throw std::runtime_error( "Basin_Error: Undefined initial crust thickness map" );
   }
   return *m_HCuMap;
}

//------------------------------------------------------------//
const GridMap& InterfaceInput::getHLMuMap() const {
   if (m_HLMuMap == nullptr){
      throw std::runtime_error( "Basin_Error: Undefined initial lithospheric mantle thickness map" );
   }
   return *m_HLMuMap;
}

//------------------------------------------------------------//
std::shared_ptr<const CrustalThickness::RiftingEvent> InterfaceInput::getRiftEvent( const double age ) const {
   auto iterator = m_riftingEvents.find( age );
   if (iterator != m_riftingEvents.end()) {
      if (iterator->second != nullptr) {
         return iterator->second;
      }
      else {
         throw std::runtime_error( "Basin_Error: The rifting event defined at " + std::to_string( age ) + "Ma is corrupted" );
      }
   }
   else {
      throw std::runtime_error( "Basin_Error: There is no rifting event defined at " + std::to_string( age ) + "Ma" );
   };
}
