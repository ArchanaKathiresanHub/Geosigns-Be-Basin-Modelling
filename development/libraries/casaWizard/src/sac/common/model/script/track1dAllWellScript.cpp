#include "track1dAllWellScript.h"

#include "model/script/WizardDataToCasaScriptMapper.h"

#include <assert.h>

namespace casaWizard
{

namespace sac
{

Track1DAllWellScript::Track1DAllWellScript(const QString& baseDirectory,
                                           const QVector<double> xCoordinates,
                                           const QVector<double> yCoordinates,
                                           const QStringList properties,
                                           const QString projectFileName) :
  RunScript(baseDirectory),
  xCoordinates_{xCoordinates},
  yCoordinates_{yCoordinates},
  properties_{properties},
  projectFileName_{projectFileName}
{
   for (QString& property : properties_)
   {
      property = wizardDataToCasaScriptMapper::mapName(property);
   }
}

bool Track1DAllWellScript::generateCommands()
{
  addCommand("which", QStringList() << "track1d");

  const int nCoordinates = xCoordinates_.size();
  if (properties_.size() == 0 || nCoordinates == 0 || nCoordinates != yCoordinates_.size())
  {
    return false;
  }

  QString command = "track1d";

  QStringList arguments;
  arguments << "-coordinates";

  arguments << QString::number(xCoordinates_[0],'f') + ","
           + QString::number(yCoordinates_[0],'f');

  if (nCoordinates>1)
  {
    for (int i=1; i<nCoordinates; ++i)
    {
      arguments.last() += "," + QString::number(xCoordinates_[i], 'f')
               + "," + QString::number(yCoordinates_[i], 'f');
    }
  }

  arguments << "-properties" << properties_.join(',')
             << "-age" << "0"
             << "-project" << projectFileName_
             << "-save" << "welldata.csv";

  addCommand(command, arguments);

  return true;
}

}  // namespace sac

}  // namespace casaWizard
