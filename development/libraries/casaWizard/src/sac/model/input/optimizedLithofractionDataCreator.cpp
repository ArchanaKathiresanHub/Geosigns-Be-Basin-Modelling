#include "optimizedLithofractionDataCreator.h"

#include "model/logger.h"
#include "model/sacScenario.h"
#include "model/input/dataFileParser.h"

namespace casaWizard
{

namespace sac
{

OptimizedLithofractionDataCreator::OptimizedLithofractionDataCreator(SACScenario& scenario, const QString& iterationPath) :
  CaseDataCreator(scenario.calibrationTargetManager(), iterationPath),
  lithofractionManager_{scenario.lithofractionManager()}
{
  lithofractionManager_.clearOptimizedLithofractions();
}

void OptimizedLithofractionDataCreator::readCase(const int wellIndex, const int caseIndex)
{
  const QString dataFolder = iterationPath() + "/Case_" + QString::number(caseIndex);
  const QString filePath{dataFolder + "/optimalLithofractions.txt"};

  const DataFileParser<double> parser{filePath};
  QVector<double> values;
  try
  {
    values = parser.rowDominantMatrix().last();
  }
  catch( const std::exception& e )
  {
    Logger::log() << e.what()
                  << "\nDid not read optimized lithofraction for well " << wellIndex << Logger::endl();
    values.clear();
  }

  const int nValues = values.size();

  int valueIndex = 0;
  int layerIndex = 0;
  while (valueIndex < nValues)
  {
    OptimizedLithofraction newOptimal
    {
      wellIndex,
      layerIndex++,
      values[valueIndex++],
      values[valueIndex++]
    };
    // Note that the fraction is present in the file, even if it is not used.

    lithofractionManager_.addOptimizedLithofraction(newOptimal);
  }
}

} // namespace sac

} // namespace casaWizard