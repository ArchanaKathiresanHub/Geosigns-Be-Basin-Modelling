//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QBrush>

struct LithoTypeBackground
{
  QString pattern;
  QColor color;
};

namespace casaWizard
{

class LithoTypeBackgroundMapping
{
public:
  static void getBackgroundBrush(const QString& lithotypeName, QBrush& brush);
  static void deleteInstance();

private:
  LithoTypeBackgroundMapping();

  QBrush createBrush(QString patternFileName, QColor backgroundColor) const;
  void initializeMapping();
  QJsonArray readJson() const;

  static LithoTypeBackgroundMapping* instance_;
  std::map<QString, QBrush> mapping_;
};

} // namespace casaWizard
