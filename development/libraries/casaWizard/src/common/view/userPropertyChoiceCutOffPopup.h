//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "userPropertyChoicePopup.h"

#include <QMap>
#include <QString>

namespace casaWizard
{

typedef QMap<QString,QPair<double, double>> PropertiesWithCutOffRanges;

class UserPropertyChoiceCutOffPopup : public UserPropertyChoicePopup
{
  Q_OBJECT
public:
  explicit UserPropertyChoiceCutOffPopup(QWidget* parent = nullptr);
  void updateTable(const QStringList& propertyUserNames) override;

  PropertiesWithCutOffRanges getPropertiesWithCutOffRanges() const;

private:
  void fillDefaultsMap();

  PropertiesWithCutOffRanges defaultsMap_;
};

} // namespace casaWizard

