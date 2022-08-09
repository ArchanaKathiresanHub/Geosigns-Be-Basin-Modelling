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
class CalibrationTargetManager;

namespace ua
{
class TargetImportDialogAscii;
class PredictionTargetManager;

class TargetImportPredictionTargetsController : public QObject
{
    Q_OBJECT

public:
    TargetImportPredictionTargetsController(const CalibrationTargetManager& calibrationTargetManager,
                                            PredictionTargetManager& predictionTargetManager,
                                            QObject* parent = nullptr);
    ~TargetImportPredictionTargetsController();
    struct XYLocations {
        XYLocations():
            x(0.0),
            y(0.0)
        {}

        XYLocations(double x, double y, const QString name = ""):
            x(x),
            y(y),
            name(name)
        {}

        double x;
        double y;
        QString name;
    };

private slots:
    void slotImportPredictionTargets();
    void slotClearAndWritePredictionTargets();
    void slotImportAccepted();
    bool slotPushSelectFileClicked();

private:
   const CalibrationTargetManager& m_calibrationTargetManager;
   PredictionTargetManager& m_predictionTargetManager;
   TargetImportDialogAscii* m_targetImportDialogAscii;
};

} // namespace ua

} // namespace casaWizard
