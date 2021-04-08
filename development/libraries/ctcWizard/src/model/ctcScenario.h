// Class that contains all data to run a ctc scenario
#pragma once

#include "lithosphereParameter.h"
#include "riftingHistory.h"

#include <QPair>
#include <QString>
#include <QVector>

namespace ctcWizard
{


class CtcScenario
{
public:
  CtcScenario();

  bool isCorrect() const;
  void addLithosphereParameter(const QString& param, const QString& value);
  void clearLithosphereTable();
  void addRiftingHistory(const QString& Age, const QString& hasPWD, const QString& TectonicFlag, const QString& RDA, const QString& RDA_Map, const QString& Basalt_Thickness, const QString& Basalt_Thickness_Map);
  void clearRiftingHistoryTable();

  QString ctcFilePathCTC() const;
  void setCtcFilePathCTC(const QString& ctcFilePathCTC);


  QString project3dPath() const;
  void setProject3dPath(const QString& project3dPath);

  QString numProc() const;
  void setnumProc(const QString& numProc);

  QString runMode() const;
  void setrunMode(const QString& runMode);


  const QVector<LithosphereParameter>& lithosphereParameters() const;
  void setLithosphereParameter(int row, int column, const QString& text);

  const QVector<RiftingHistory>& riftingHistory() const;
  void setRiftingHistory(int row, int column, const QString& text);

  static const QStringList& riftingHistoryOptions();
  static const QStringList& riftingHistoryBasaltMaps();
  static const QStringList& riftingHistoryRDAMaps();

  static QStringList riftingHistoryBasaltMaps_;
  static QStringList riftingHistoryRDAMaps_;

private:
  QString ctcFilePathCTC_="";
  QString project3dPath_="";
  QString numProc_= "1";
  QString runMode_="Decompaction";
  QVector<LithosphereParameter> lithosphereParameters_;
  QVector<RiftingHistory>  riftingHistory_;

  static QStringList riftingHistoryOptions_;

};

} // namespace ctcWizard
