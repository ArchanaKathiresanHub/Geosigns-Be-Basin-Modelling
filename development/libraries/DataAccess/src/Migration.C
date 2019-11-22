//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// std library
#include <assert.h>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

// TableIO library
#include "database.h"
#include "cauldronschemafuncs.h"
using namespace database;

// DataAccess library
#include "Formation.h"
#include "ProjectHandle.h"
#include "Reservoir.h"
#include "Snapshot.h"
#include "Migration.h"
#include "Trapper.h"
using namespace DataAccess;
using namespace Interface;

static const char ** ComponentNames = CBMGenerics::ComponentManager::getInstance().getSpeciesNameInputList();

Migration::Migration (ProjectHandle& projectHandle, Record * record) : DAObject (projectHandle, record)
{
   m_sourceSnapshot = 0;
   m_destinationSnapshot = 0;
   m_sourceFormation = 0;
   m_sourceReservoir = 0;
   m_destinationReservoir = 0;
   m_sourceTrapper = 0;
   m_destinationTrapper = 0;
}

Migration::~Migration (void)
{
}

bool Migration::matchesConditions (const string & process, const Formation * sourceFormation,
      const Snapshot * sourceSnapshot, const Reservoir * sourceReservoir, const Trapper * sourceTrapper,
      const Snapshot * destinationSnapshot, const Reservoir * destinationReservoir, const Trapper * destinationTrapper)
{
   if (process != "" && getProcess () != process) return false;
   if (sourceFormation != 0 && getSourceFormation () != sourceFormation) return false;

   if (sourceSnapshot != 0 && getSourceSnapshot () != sourceSnapshot) return false;
   if (sourceReservoir != 0 && getSourceReservoir () != sourceReservoir) return false;
   if (sourceTrapper != 0 && getSourceTrapper () != sourceTrapper) return false;

   if (destinationSnapshot != 0 && getDestinationSnapshot () != destinationSnapshot) return false;
   if (destinationReservoir != 0 && getDestinationReservoir () != destinationReservoir) return false;
   if (destinationTrapper != 0 && getDestinationTrapper () != destinationTrapper) return false;

   return true;
}

const string & Migration::getProcess (void) const
{
   return database::getMigrationProcess (m_record);
}

unsigned int Migration::getSourceTrapId (void) const
{
   return database::getSourceTrapID (m_record);
}

unsigned int Migration::getDestinationTrapId (void) const
{
   return database::getDestinationTrapID (m_record);
}

void Migration::setSourceSnapshot (const Snapshot * snapshot)
{
   m_sourceSnapshot = snapshot;
}

const Snapshot * Migration::getSourceSnapshot (void) const
{
   return m_sourceSnapshot;
}

void Migration::setDestinationSnapshot (const Snapshot * snapshot)
{
   m_destinationSnapshot = snapshot;
}

const Snapshot * Migration::getDestinationSnapshot (void) const
{
   return m_destinationSnapshot;
}

void Migration::setSourceFormation (const Formation * formation)
{
   m_sourceFormation = formation;
}

const Formation * Migration::getSourceFormation (void) const
{
   return m_sourceFormation;
}

void Migration::setSourceReservoir (const Reservoir * reservoir)
{
   m_sourceReservoir = reservoir;
}

const Reservoir * Migration::getSourceReservoir (void) const
{
   return m_sourceReservoir;
}

void Migration::setDestinationReservoir (const Reservoir * reservoir)
{
   m_destinationReservoir = reservoir;
}

const Reservoir * Migration::getDestinationReservoir (void) const
{
   return m_destinationReservoir;
}

void Migration::setSourceTrapper (const Trapper * trapper)
{
   m_sourceTrapper = trapper;
}

const Trapper * Migration::getSourceTrapper (void) const
{
   return m_sourceTrapper;
}

void Migration::setDestinationTrapper (const Trapper * trapper)
{
   m_destinationTrapper = trapper;
}

const Trapper * Migration::getDestinationTrapper (void) const
{
   return m_destinationTrapper;
}

/// return the mass of the given component in this Migration
double Migration::getMass (ComponentId componentId) const
{
   return getMass (ComponentNames[componentId]);
}

/// return the mass of the given component in this Migration
double Migration::getMass (const string & componentName) const
{
   string componentMassName = "Mass";
   componentMassName += componentName;
   return m_record->getValue<double> (componentMassName);
}

void Migration::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}

void Migration::asString (string & str) const
{
   ostringstream buf;

   buf << "Migration:";
   buf << " process = " << getProcess ();
   buf << endl;

   str = buf.str ();
}
