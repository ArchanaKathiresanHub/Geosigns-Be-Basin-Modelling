//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QThread>

class QString;

namespace casaWizard
{
class CalibrationTargetCreator;

class LoadTargetsThread : public QThread
{
  Q_OBJECT
public:
  LoadTargetsThread(CalibrationTargetCreator& targetCreator, QObject* parent = nullptr);
  void run() override;

signals:
  void exceptionThrown(QString message);

private:
  CalibrationTargetCreator& calibrationTargetCreator_;
};

} // namespace casaWizard

