//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TCHPVisualisationController.h"


#include "model/input/cmbMapReader.h"
#include "model/VectorVectorMap.h"
#include "model/ThermalScenario.h"
#include "model/well.h"

#include "view/sharedComponents/customcheckbox.h"
#include "model/functions/sortedByXWellIndices.h"
#include "view/plots/ThermalGrid2DPlot.h"
#include "view/assets/TCHPVisualisation.h"

#include "ConstantsMathematics.h"
#include "ConstantsNumerical.h"

#include "UndefinedValues.h"

#include <QDir>
#include <QComboBox>

namespace casaWizard
{

namespace sac
{

namespace thermal
{

TCHPVisualisationController::TCHPVisualisationController(TCHPVisualisation* TCHPVisualisation,
                                                         ThermalScenario& casaScenario,
                                                         QObject* parent) :
   SacVisualisationController{parent},
   TCHPVisualisation_{TCHPVisualisation},
   scenario_{casaScenario}
{
   connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));
   connect(TCHPVisualisation_->TCHPPlot()->get2DView(), &TCHP2DView::toolTipCreated, [=](const QPointF& point){createToolTip(point);});
}

void TCHPVisualisationController::slotUpdatePlot()
{
   double xMin = 0; double xMax = 1; double yMin = 0; double yMax = 1;
   scenario_.projectReader().domainRange(xMin, xMax, yMin, yMax);

   VectorVectorMap ThermalMap({});
   const bool mapAvailable = scenario_.getThermalMap(ThermalMap);

   ThermalGrid2DPlot* plot = TCHPVisualisation_->TCHPPlot();
   plot->setVisible(true);
   plot->showColorBar();
   plot->get2DView()->updatePlots(ThermalMap.getData());
   plot->get2DView()->updateRange(xMin * Utilities::Maths::MeterToKilometer,
                                  xMax * Utilities::Maths::MeterToKilometer,
                                  yMin * Utilities::Maths::MeterToKilometer,
                                  yMax * Utilities::Maths::MeterToKilometer);
   plot->updateColorBar(2);
   plot->setTitle(QString(mapAvailable ? "" : "INPUT MAP - ") + "Top Crust heat production");
   plot->get2DView()->setToolTipVisible(false);
}

double TCHPVisualisationController::getTCHPAtLocation(const QPointF& point) const
{
   return TCHPVisualisation_->TCHPPlot()->get2DView()->getValue(point);
}

void TCHPVisualisationController::createToolTip(const QPointF& point)
{
   TCHPVisualisation_->hideAllTooltips();
   emit clearWellListHighlightSelection();

   double TCHPAtPoint;
   int closestWellID = -1;
   if (TCHPVisualisation_->wellsVisible()->checkState() == Qt::CheckState::Checked)
   {
      TCHPAtPoint = scenario_.getTCHPOfClosestWell(point.x(), point.y(), closestWellID);
   }
   else
   {
      TCHPAtPoint = getTCHPAtLocation(point);
   }

   if (TCHPIsValid(TCHPAtPoint))
   {
      QString wellName = "";
      if (closestWellID != -1)
      {
         const Well& closestWell = scenario_.calibrationTargetManager().well(closestWellID);
         wellName = closestWell.name();
         TCHPVisualisation_->TCHPPlot()->get2DView()->moveTooltipToDomainLocation(QPointF(closestWell.x() * Utilities::Maths::MeterToKilometer,
                                                                                          closestWell.y() * Utilities::Maths::MeterToKilometer));
         emit wellClicked(wellName);
      }

      TCHPVisualisation_->finalizeTooltip(TCHPAtPoint, wellName);
   }
}

bool TCHPVisualisationController::TCHPIsValid(const double TCHPAtPoint) const
{
   return !Utilities::isValueUndefined(TCHPAtPoint);
}

void TCHPVisualisationController::updateSelectedWells(QVector<int> selectedWells)
{
   TCHPVisualisation_->updateSelectedWells(selectedWells);
}

void TCHPVisualisationController::hideAllTooltips()
{
   TCHPVisualisation_->hideAllTooltips();
}

void TCHPVisualisationController::updateBirdsView()
{
   if (scenario_.project3dPath() == "")
   {
      TCHPVisualisation_->setVisible(false);
      return;
   }
   TCHPVisualisation_->setVisible(true);


   emit clearWellListHighlightSelection();
   QVector<OptimizedTCHP> optimizedTCHPs;
   QVector<const Well*> activeAndIncludedWells;

   scenario_.getVisualisationData(optimizedTCHPs, activeAndIncludedWells);

   const QVector<int> sortedIndices = casaWizard::functions::sortedByXYWellIndices(scenario_.calibrationTargetManager().activeWells());
   const QVector<const Well *> includedWells = scenario_.calibrationTargetManager().activeAndIncludedWells();
   QVector<OptimizedTCHP> sortedOptimizedTCHPs;
   for(int i = 0; i < optimizedTCHPs.size(); ++i)
   {
      for (auto well : includedWells)
      {
         if (well->id() == optimizedTCHPs[sortedIndices[i]].wellId())
         {
            sortedOptimizedTCHPs.push_back(optimizedTCHPs[sortedIndices[i]]);
         }
      }

   }

   TCHPVisualisation_->updateBirdsView(activeAndIncludedWells, sortedOptimizedTCHPs);

   slotUpdatePlot();
   hideAllTooltips();
}

void TCHPVisualisationController::slotRefresh()
{
   updateBirdsView();
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
