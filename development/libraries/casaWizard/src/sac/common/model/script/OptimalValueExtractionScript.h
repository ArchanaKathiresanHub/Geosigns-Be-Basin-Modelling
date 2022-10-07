// Script to run casa to extract the optimized lithofraction for each case
#pragma once

#include "model/script/runScript.h"

#include <QStringList>
#include <QFile>

namespace casaWizard
{

namespace sac
{

class OptimalValueExtractionScript : public RunScript
{
public:
   explicit OptimalValueExtractionScript(const QString& baseDirectory, QString scriptFileName = "optimizationScript.casa");
   bool generateCommands() override;
   void addCase(const QString& runFolder);
   bool scriptShouldCancelWhenFailureIsEncountered() const override;

protected:
   virtual void writeScript(QFile* file) = 0;


private:
   void createScriptInFolder(const QString& folderName);

   const QString scriptFilename_;
   QStringList runFolders_;
};

} // namespace sac

} // namespace casaWizard
