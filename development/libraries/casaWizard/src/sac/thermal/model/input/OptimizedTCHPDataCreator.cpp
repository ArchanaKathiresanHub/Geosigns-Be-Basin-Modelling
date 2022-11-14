#include "OptimizedTCHPDataCreator.h"

#include "model/logger.h"
#include "model/ThermalScenario.h"
#include "model/input/dataFileParser.h"
#include "model/TCHPManager.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

OptimizedTCHPDataCreator::OptimizedTCHPDataCreator(ThermalScenario& scenario, const QString& iterationPath) :
   CaseDataCreator(scenario.calibrationTargetManager(), iterationPath),
   m_TCHPManager{scenario.TCHPmanager()}
{
   m_TCHPManager.clearOptimizedTCHPs();
}

void OptimizedTCHPDataCreator::readCase(const int wellIndex, const int caseIndex)
{
   const QString dataFolder = iterationPath() + "/Case_" + QString::number(caseIndex);
   const QString filePath{dataFolder + "/optimalTopCrustHeatProduction.txt"};

   QVector<double> values;
   try
   {
      values = DataFileParser<double>::readFile(filePath).last();
   }
   catch( const std::exception& e )
   {
      Logger::log() << e.what()
                    << "\nCould not read optimized Heat crust producion for well " << wellIndex << Logger::endl();
      return;
   }

   for (int valueIndex = 0; valueIndex < values.size(); ++valueIndex)
   {
      OptimizedTCHP newOptimal{ wellIndex, values[valueIndex] };
      m_TCHPManager.addOptimizedTCHP(newOptimal);
   }
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
