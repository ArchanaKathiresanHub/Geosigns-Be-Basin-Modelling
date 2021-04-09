//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

class QString;
class QStringList;

namespace casaWizard
{

class ProjectWriter
{
public:
  ProjectWriter() = default;
  virtual ~ProjectWriter() = default;

  virtual void generateOutputProject(const QString& timeStamp) = 0;
  virtual void setRelevantOutputParameters(const QStringList& activeProperties) = 0;
  virtual void setScaling(int scaleX, int scaleY) = 0;
};

} // namespace casaWizard
