//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QTableWidget>

namespace casaWizard
{

namespace ua
{

class PredictionTarget;
class UAResultsTargetsData;

class UAResultsTargetTable : public QTableWidget
{
public:
   UAResultsTargetTable(QWidget *parent = Q_NULLPTR);
   void fillTable(const UAResultsTargetsData& targetsData);

private:
   int m_numDefaultCols;
};

} //ua
} //casaWizard
