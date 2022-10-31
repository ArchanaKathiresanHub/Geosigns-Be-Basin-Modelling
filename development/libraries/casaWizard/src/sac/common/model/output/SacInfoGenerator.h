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

class SacMapsManager;
class SacScenario;

class SacInfoGenerator : public InfoGenerator
{
public:
   SacInfoGenerator(ProjectReader& projectReader);
   virtual ~SacInfoGenerator() override = default;

   void loadProjectReader(const std::string& projectFileLocation) override;
   void generateInfoTextFile() override;

protected:
   virtual void addMapsGenerationSection();
   virtual const SacMapsManager& mapsManager() const = 0;
   virtual const SacScenario& scenario() const = 0;

   void addGeneralSettingsSection();
   void addInputTabSection();
   void addWellsSection();
   const ProjectReader& projectReader() const;

   std::string doubleToFormattedString(const double inputDouble);
private:
   virtual void addOptimizationOptionsInfo() = 0;
   void addInterpolation();
   void addObjectiveFunction();
   void addRunMode();
   void addSmoothing();
   void addWellInfo(const casaWizard::Well* well);

   ProjectReader& m_projectReader;
};

} // namespace sac

} // namespace casaWizard
