//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QVector>
#include <QString>

namespace casaWizard
{

namespace ua
{

class TargetInputInfo {
public:
   TargetInputInfo(QVector<bool> surfaceSelectionStates,
                   bool temperatureTargetsSelected,
                   bool vreTargetsSelected,
                   QString depthInput);

   struct XYName
   {
      XYName():
         x(0.0),
         y(0.0),
         name("")
      {}

      XYName(double x, double y,const QString name = ""):
         x(x),
         y(y),
         name(name)
      {}

      double x;
      double y;
      QString name;
   };


   virtual QVector<XYName> getTargetLocations() const = 0;

   QVector<bool> getSurfaceSelectionStates() const;
   bool getTemperatureTargetsSelected() const;
   bool getVreTargetsSelected() const;
   QString getDepthInput() const;

   void setSurfaceSelectionStates(QVector<bool> states);
   void setTemperatureTargetsSelected(bool state);
   void setVreTargetsSelected(bool state);
   void setDepthInput(QString depthInput);

private:
   QVector<bool> m_surfaceSelectionStates;
   bool m_temperatureTargetsSelected;
   bool m_vreTargetsSelected;
   QString m_depthInput;
};

} // namespace ua

} // namespace casaWizard
