//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "view/calibrationTargetTable.h"

namespace casaWizard
{

namespace ua
{

class CalibrationTargetTableUA : public CalibrationTargetTable
{
public:
   explicit CalibrationTargetTableUA(QWidget* parent = nullptr);
   void updateTable(const QVector<const Well *> wells, const QMap<QString, QSet<int> > &propertyNamesPerWell) override;

private:
   bool addEnabledCheckBox() override;
   int wellXColumn() override;
   int wellYColumn() override;
   int wellNameColumn() override;
};

} // namespace ua

} // namespace casaWizard
