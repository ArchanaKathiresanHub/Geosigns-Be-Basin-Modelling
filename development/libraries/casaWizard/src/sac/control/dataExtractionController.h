#pragma once

#include <QObject>

namespace casaWizard
{

class ScriptRunController;

namespace sac
{

class SACScenario;
class CaseExtractor;

class DataExtractionController : public QObject
{
    Q_OBJECT

public:
    explicit DataExtractionController(SACScenario& scenario,
                                      ScriptRunController& scriptRunController,
                                      QObject* parent);
    void readResults();

private:
    void readCaseData(CaseExtractor& extractor, const QString& message);

    SACScenario& scenario_;
    ScriptRunController& scriptRunController_;
};

} // namespace sac

} // namespace casaWizard
