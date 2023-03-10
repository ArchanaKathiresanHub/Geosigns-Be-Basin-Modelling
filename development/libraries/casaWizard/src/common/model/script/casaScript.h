// Abstract base class for writing a casa script
#pragma once

#include "runScript.h"

class QFile;

namespace casaWizard
{

class CasaScenario;

class CasaScript : public RunScript
{
public:
  CasaScript(const QString& baseDirectory);
  virtual ~CasaScript() override = default;

  virtual const CasaScenario& scenario() const = 0;
  virtual QString scriptFilename() const = 0;
  virtual QString workingDirectory() const = 0;
  virtual QString relativeDirectory() const;
  bool generateCommands() override;
  bool killAsync() const override;
  bool writeScript() const;

protected:  
  virtual bool createStopExecFile() const;
  QString writeApp(int numberOfCPUs, const QString& applicationName) const;
  QString writeBaseProject(const QString& project3dPath) const;
  QString writeRun(const QString& clusterName) const;
  QString writeRunDataDigger() const;
  QString writeLocation(const QString& doe, const bool append = false, const bool noOptimization = false, const bool removeModelFromMemory = false) const;
  QString writeSaveState(const QString& stateFilename) const;
  QString writeLoadState(const QString& stateFilename) const;
  QString writeDoeTextList(const QStringList& doeList) const;

  const QString stopExecFilename_{"stop_exec_scenario"};

private:
  virtual void writeScriptContents(QFile& file) const = 0;
  virtual bool validateScenario() const = 0;
  bool validateBaseScenario() const;  
  void removeStopExecFile() const;

};

} // namespace casaWizard
