//
// Copyright (C) 2012 - 2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "SacMapsTabValidator.h"

class QPointF;
class QString;

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalController;
class ThermalGuiTester;

class TCHPMapToolTip;
class ThermalMapPlotOptions;

class ThermalMapsTabValidator : public SacMapsTabValidator
{
public:
   ThermalMapsTabValidator(ThermalController& controller, ThermalGuiTester* tester);

   void validateMapsTab()                     final;
   void validateInitialMapsTabOptions()       final;
   void validateGridding()                    final;
   void validateRun3DOptimization()           final;
   void validateMapOptionsAfterGridding()     final;
   void validateExports()                     final;

   void validateTitleBeforeGridding();
   void validateTitleAfterGridding();
   void validateNumberOfPlottedWells();
   void validateTooltipWithWellsVisible();
   void validateTooltipWhenWellsInvisible();
   void validateMapsTabPlottingOptions();
   void validateWellHighlighting();
   void validateTooltipHasLabelWithText(const QString &labelText, const TCHPMapToolTip *tooltip);
   void validateOptimizedProjectZipExport();
   void validateOptimizedMapZycorExport();

private:
   ThermalController& m_controller;
   ThermalGuiTester* m_tester;

   void createTooltipOnPoint(const QPointF& point);
   TCHPMapToolTip* getTooltipFromMap();
   ThermalMapPlotOptions *getPlotOptions();

   void processDestinationFolderPopup();
   void processCpuPopup();
   void processSubSamplingPopup();
};

} // thermal

} // sac

} // casaWizard
