//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <view/assets/SacMapToolTip.h>

#include <QWidget>

class QLabel;

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class TCHPMapToolTip : public SacMapToolTip
{
  Q_OBJECT
public:
  explicit TCHPMapToolTip(QWidget *parent = nullptr);
  void setTCHP(const double& TCHP, const QString& wellName);

private:
  void setTotalLayout();
  double m_TCHP;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
