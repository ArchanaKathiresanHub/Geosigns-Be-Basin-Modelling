#pragma once

#include <QObject>

namespace casaWizard
{

class ScriptRunController;

namespace sac
{

class SacScenario;
class CaseExtractor;

class DataExtractionController : public QObject
{
   Q_OBJECT

public:
   explicit DataExtractionController(SacScenario& scenario,
                                     ScriptRunController& scriptRunController,
                                     QObject* parent);
   virtual void readOriginalResults();
   virtual void readOptimizedResults(CaseExtractor& caseExtractor);

protected:
   void readCaseData(CaseExtractor& extractor, const QString& message);
   virtual ScriptRunController& scriptRunController();
   virtual SacScenario& scenario();

private:
   SacScenario& scenario_;
   ScriptRunController& scriptRunController_;
};

} // namespace sac

} // namespace casaWizard
