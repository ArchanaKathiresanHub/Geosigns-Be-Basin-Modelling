//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/output/infoGenerator.h"

namespace casaWizard
{

class CMBMapReader;
class ProjectReader;
class Well;

namespace sac
{

class MapsManager;
class SacScenario;

class SacInfoGenerator : public InfoGenerator
{
public:
   SacInfoGenerator(ProjectReader& projectReader);
   virtual ~SacInfoGenerator() override = default;

   void loadProjectReader(const std::string& projectFileLocation) override;

protected:
   virtual void addMapsGenerationSection();
   virtual const MapsManager& mapsManager() const = 0;
   virtual const SacScenario& scenario() const = 0;

   void addGeneralSettingsSection();
   void addInputTabSection();
   void addWellsSection();

   const ProjectReader& projectReader() const;

private:
   void addFormationInfo();
   void addInterpolation();
   void addObjectiveFunction();
   void addRunMode();
   void addSmoothing();
   void addWellInfo(const casaWizard::Well* well);

   std::string doubleToFormattedString(const double inputDouble);

   ProjectReader& m_projectReader;
};

} // namespace sac

} // namespace casaWizard
