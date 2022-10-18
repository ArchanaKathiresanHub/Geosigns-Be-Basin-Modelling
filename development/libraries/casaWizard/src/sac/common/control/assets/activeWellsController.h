//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Controller to list the active wells. Wells can be excluded for the 2d gridding.
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

class CasaScenario;

namespace sac
{

class ActiveWellsTable;

class ActiveWellsController : public QObject
{
  Q_OBJECT

public:
  ActiveWellsController(ActiveWellsTable* calibTable,
                        CasaScenario& casaScenario,
                        QObject* parent);

public slots:
  void slotClearWellListHighlightSelection();
  void slotSelectAllWells();
  void slotDeselectAllWells();

private slots:
  void slotWellCheckBoxStateChanged(int state, int wellIndex);
  void slotRefresh();

private:
  void setAllsWellsExcludedState(bool excludedState);

  ActiveWellsTable* activeWellsTable_;
  CasaScenario& casaScenario_;
};

}  // namespace sac

}  // namespace casaWizard
