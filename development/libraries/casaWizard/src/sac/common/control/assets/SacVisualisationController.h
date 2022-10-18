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

namespace sac
{

class SacVisualisationController : public QObject
{
   Q_OBJECT

public:
   SacVisualisationController(QObject* parent);
   virtual ~SacVisualisationController() = default;

   virtual void updateBirdsView() = 0;
   virtual void updateSelectedWells(QVector<int> selectedWells) = 0;
   virtual void hideAllTooltips() = 0;

signals:
   void wellClicked(const QString&);
   void clearWellListHighlightSelection();

private slots:
   virtual void slotRefresh() = 0;

};

} // namespace sac

} // namespace casaWizard
