//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#pragma once

#include <string>
#include <map>

#include "datadriller.h"

namespace DataAccess
{
  namespace Interface
  {
    class ProjectHandle;
    class Property;
  }
}

namespace database
{
  class Record;
}

namespace AbstractDerivedProperties
{
  class FormationPropertyCalculator;
}

namespace GeoPhysics
{
  class GeoPhysicsFormation;
  class CompoundLithology;
}

namespace DataExtraction
{
  class DerivedPropertyDriller
  {
  public:
    DerivedPropertyDriller( const DataDriller* dataDriller );
    bool run(double& value);
    void setRecord(database::Record* record);
    void setSnapshot(const DataAccess::Interface::Snapshot *snapshot);

  private:
    database::Record* m_record;
    const DataAccess::Interface::Snapshot* m_snapshot;
    std::map<std::string, double> m_obtainedProperties;
    const DataDriller* m_dataDriller;

    double run(const std::string& alternativePropertyName);

    std::vector<std::string> defineDependentProperties(const std::string &propertyName) const;
    void obtainDependentProperties(const std::vector<std::string>& dependentProperties);
    double obtainValue(const std::string& propertyName);
    double calculateProperty(const std::string &propertyName) const;
    AbstractDerivedProperties::FormationPropertyCalculator *initializeCalculator(const std::string &propertyName) const;
    const GeoPhysics::CompoundLithology* getLithology(const unsigned int i, const unsigned int j, const GeoPhysics::GeoPhysicsFormation* formation) const;
  };
}
