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

  QVector<double> values;
  try
  {
    values = DataFileParser<double>::readFile(filePath).last();
  }
  catch( const std::exception& e )
  {
    Logger::log() << e.what()
                  << "\nDid not read optimized lithofraction for well " << wellIndex << Logger::endl();
    values.clear();
  }

  const QVector<Lithofraction>& lithofractions = lithofractionManager_.lithofractions();

  int lithoFractionIndex = 0;
  for ( int valueIndex = 0; valueIndex < values.size(); valueIndex+=2, lithoFractionIndex++ )
  {
    while (!lithofractions[lithoFractionIndex].doFirstOptimization())
    {
      lithoFractionIndex++;
    }
    OptimizedLithofraction newOptimal
    {
      wellIndex,
      lithoFractionIndex,
      values[valueIndex],
      values[valueIndex + 1]
    };
    // Note that the fraction is present in the file, even if it is not used.

    lithofractionManager_.addOptimizedLithofraction(newOptimal);
  }
}

} // namespace sac

} // namespace casaWizard
