//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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
    void readOriginalResults();
    void readOptimizedResults();

private:
    void readCaseData(CaseExtractor& extractor, const QString& message);

    ScriptRunController& scriptRunController_;
    SACScenario& scenario_;
};

} // namespace sac

} // namespace casaWizard
