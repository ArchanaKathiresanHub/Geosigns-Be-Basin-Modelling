//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QObject>
#include <QVector>

class QString;

namespace casaWizard
{

class ScriptRunController;

namespace sac
{

namespace lithology
{

class LithologyScenario;
class T2Ztab;

class LithologyT2Zcontroller : public QObject
{
   Q_OBJECT

public:
   LithologyT2Zcontroller(T2Ztab* t2zTab,
                          LithologyScenario& casaScenario,
                          ScriptRunController& scriptRunController,
                          QObject* parent);

private slots:
   void slotPushButtonSACrunT2ZClicked();
   void slotComboBoxReferenceSurfaceValueChanged(int referenceSurface);
   void slotSpinBoxSubSamplingValueChanged(int subSampling);
   void slotComboBoxProjectSelectionTextChanged(const QString& projectSelection);
   void slotSpinBoxNumberOfCPUsValueChanged(int numberOfCPUs);
   void slotUpdateTabGUI(int tabID);
   void slotComboBoxClusterSelectionTextChanged(const QString& clusterName);
   void slotExportT2ZScenarioClicked();
   void slotExportT2ZMapsToZycorClicked();
   void slotOutputFile();

private:
   bool noProjectAvailable() const;
   bool projectHasNoTWTDataBelowReferenceSurface() const;
   bool referenceSurfaceHasNoTWTData() const;
   bool runCanceled() const;
   bool prepareT2ZWorkSpace();
   void getSourceAndT2zDir();
   void setSubSampling();
   void runDepthConversion();
   void updateSurfaces();
   void updateProjectSelectionOptions();

   T2Ztab* t2zTab_;
   LithologyScenario& casaScenario_;
   ScriptRunController& scriptRunController_;
   QString t2zDir_;
   QString sourceDir_;
   int lineNr_;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
