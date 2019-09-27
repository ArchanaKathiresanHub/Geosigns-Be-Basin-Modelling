// Class for running track1d on multiple wells in a 3d case
// It runs all wells and all properties in one run
#pragma once

#include "model/script/runScript.h"

#include <QStringList>
namespace casaWizard
{

namespace sac
{

class Track1DAllWellScript : public RunScript
{
public:
  explicit Track1DAllWellScript(const QString& baseDirectory,
                                const QVector<double> xCoordinates,
                                const QVector<double> yCoordinates,
                                const QStringList properties,
                                const QString projectFileName);
  bool generateCommands() override;

private:
  const QVector<double> xCoordinates_;
  const QVector<double> yCoordinates_;
  const QStringList properties_;
  const QString projectFileName_;
};



}  // namespace sac

}  // namespace casaWizard
