#include "SimulationDetails.h"

#include "database.h"
#include "cauldronschemafuncs.h"

DataAccess::Interface::SimulationDetails::SimulationDetails (ProjectHandle * projectHandle, database::Record * record) : DAObject (projectHandle, record) {

   m_simulatorName = database::getSimulatorName ( m_record );
   m_simulatorMode = database::getSimulatorMode ( m_record );
   m_commandLineParams = database::getSimulatorCommandLineParameters ( m_record );
}

DataAccess::Interface::SimulationDetails::~SimulationDetails () {
   // nothing to do.
}

int DataAccess::Interface::SimulationDetails::getSimulationSequenceNumber () const {
   return database::getSimulationSequenceNumber ( m_record );
}

const std::string& DataAccess::Interface::SimulationDetails::getSimulatorName () const {
   return m_simulatorName;
}

const std::string& DataAccess::Interface::SimulationDetails::getSimulatorMode () const {
   return m_simulatorMode;
}

int DataAccess::Interface::SimulationDetails::getNumberOfCores () const {
   return database::getNumberOfCores ( m_record );
}

const std::string& DataAccess::Interface::SimulationDetails::getSimulatorCommandLineParameters () const {
   return m_commandLineParams;
}

bool DataAccess::Interface::SimulationDetailsComparison::operator ()( const SimulationDetails* left, const SimulationDetails* right ) const {
   return left->getSimulationSequenceNumber () < right->getSimulationSequenceNumber ();
}
