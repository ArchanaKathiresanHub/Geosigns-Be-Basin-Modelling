#include "ctcScenario.h"

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>
#include <QTextStream>

#include <numeric>

namespace ctcWizard
{
QStringList CtcScenario::riftingHistoryOptions_ = QStringList() << "Active Rifting" << "Passive Margin" << "Flexural Basin";
QStringList CtcScenario::riftingHistoryRDAMaps_ = QStringList() << "";
QStringList CtcScenario::riftingHistoryBasaltMaps_ = QStringList() << "";

CtcScenario::CtcScenario() :
  ctcFilePathCTC_(""),
  project3dPath_(""),
  numProc_(""),
  runMode_(""),
  lithosphereParameters_(),
  riftingHistory_{}
{
    addLithosphereParameter("Initial Crust Thickness [m]","");
    addLithosphereParameter("Initial Mantle Thickness [m]","");
    addLithosphereParameter("Smoothing Radius [number of cells]","");
}

bool CtcScenario::isCorrect() const
{
  if(project3dPath_.isEmpty())
  {
    QMessageBox messageBox;
    messageBox.setText("Please provide the path to the project 3D file");
    messageBox.exec();
    return false;
  }
  return true;
}

void CtcScenario::addLithosphereParameter(const QString& param, const QString& value)
{
  LithosphereParameter newLithosphereParameter;
  newLithosphereParameter.param = param;
  newLithosphereParameter.value = value;
  lithosphereParameters_.append(newLithosphereParameter);
}

void CtcScenario::clearLithosphereTable()
{
    lithosphereParameters_.clear();
}

void CtcScenario::clearRiftingHistoryTable()
{
    riftingHistory_.clear();
}

void CtcScenario::addRiftingHistory(const QString& Age, const QString& hasPWD, const QString& TectonicFlag, const QString& RDA, const QString& RDA_Map, const QString& Basalt_Thickness, const QString& Basalt_Thickness_Map)
{
  RiftingHistory target;
  target.Age = Age;
  target.HasPWD = hasPWD;
  target.TectonicFlag = TectonicFlag;
  target.RDA = RDA;
  target.RDA_Map = RDA_Map;
  target.Basalt_Thickness = Basalt_Thickness;
  target.Basalt_Thickness_Map = Basalt_Thickness_Map;
  riftingHistory_.append(target);
}


QString CtcScenario::ctcFilePathCTC() const
{
  return ctcFilePathCTC_;
}

void CtcScenario::setCtcFilePathCTC(const QString& ctcFilePathCTC)
{
  ctcFilePathCTC_ = ctcFilePathCTC;
}



QString CtcScenario::project3dPath() const
{
  return project3dPath_;
}

void CtcScenario::setProject3dPath(const QString& project3dPath)
{
  project3dPath_ = project3dPath;
}

QString CtcScenario::numProc() const
{
  return numProc_;
}

void CtcScenario::setnumProc(const QString& numProc)
{
  numProc_ = numProc;
}

QString CtcScenario::runMode() const
{
  return runMode_;
}

void CtcScenario::setrunMode(const QString& runMode)
{
  runMode_ = runMode;
}

const QVector<LithosphereParameter>& CtcScenario::lithosphereParameters() const
{
  return lithosphereParameters_;
}

void CtcScenario::setLithosphereParameter(int row, int column, const QString& text)
{
  switch(column)
  {
    case 0:
      lithosphereParameters_[row].param = text;
      break;
    case 1:
      lithosphereParameters_[row].value = text;
      break;
  }
}

const QVector<RiftingHistory>& CtcScenario::riftingHistory() const
{
  return riftingHistory_;
}

void CtcScenario::setRiftingHistory(int row, int column, const QString& text)
{
  switch(column)
  {
    case 0:
      riftingHistory_[row].Age = text;
      break;
    case 1:
      riftingHistory_[row].HasPWD = text;
      break;
    case 2:
      riftingHistory_[row].TectonicFlag = text;
      break;
    case 3:
      riftingHistory_[row].RDA = text;
      break;
    case 4:
      riftingHistory_[row].RDA_Map = text;
      break;
    case 5:
      riftingHistory_[row].Basalt_Thickness = text;
      break;
    case 6:
      riftingHistory_[row].Basalt_Thickness_Map = text;
      break;
  }
}

const QStringList& CtcScenario::riftingHistoryOptions()
{
  return riftingHistoryOptions_;
}

const QStringList& CtcScenario::riftingHistoryRDAMaps()
{
  return riftingHistoryRDAMaps_;
}

const QStringList& CtcScenario::riftingHistoryBasaltMaps()
{
  return riftingHistoryBasaltMaps_;
}

} // namespace ctcWizard
