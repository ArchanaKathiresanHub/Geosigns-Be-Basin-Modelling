#include "influentialParameterManager.h"

#include "influentialParameter.h"
#include "model/input/projectReader.h"
#include "model/logger.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include <assert.h>

namespace casaWizard
{

namespace ua
{

InfluentialParameterManager::InfluentialParameterManager(const ProjectReader& projectReader) :
   m_version{1},
   m_options{InfluentialParameter::list()},
   m_influentialParameters(),
   m_isUsedInCorrelation{},
   m_projectReader{projectReader}
{
}

InfluentialParameterManager::~InfluentialParameterManager()
{
   for (InfluentialParameter* influentialParameter : m_influentialParameters)
   {
      delete influentialParameter;
   }
}

QVector<InfluentialParameter*> InfluentialParameterManager::influentialParameters() const
{
   return m_influentialParameters;
}

int InfluentialParameterManager::totalNumberOfInfluentialParameters() const
{
   int nInfluentialParameters = 0;

   for (const InfluentialParameter* influentialParameter : m_influentialParameters)
   {
      nInfluentialParameters += influentialParameter->totalNumberOfParameters();
   }

   return nInfluentialParameters;
}

QStringList InfluentialParameterManager::options() const
{
   return m_options;
}

void InfluentialParameterManager::set(const int row, const int selected)
{
   delete m_influentialParameters[row];
   try
   {
      m_influentialParameters[row] = InfluentialParameter::createFromIndex(selected);
      m_influentialParameters[row]->fillArguments(m_projectReader);
   }
   catch (std::runtime_error e)
   {
      Logger::log() << e.what() << Logger::endl();
   }
}

void InfluentialParameterManager::setArguments(int index, const QVector<double>& valueArguments, const QStringList& listArguments)
{
   m_influentialParameters[index]->arguments().setDoubleArguments(valueArguments);
   m_influentialParameters[index]->arguments().setListValues(listArguments);
}

void InfluentialParameterManager::add(const int index)
{
   try
   {
      InfluentialParameter* newInfluentialParameter = InfluentialParameter::createFromIndex(index);
      newInfluentialParameter->fillArguments(m_projectReader);
      m_influentialParameters.append(newInfluentialParameter);
      m_isUsedInCorrelation.push_back(true);
   }
   catch (std::runtime_error e)
   {
      Logger::log() << e.what() << Logger::endl();
   }
}

void InfluentialParameterManager::remove(int index)
{
   m_influentialParameters.remove(index);
   m_isUsedInCorrelation.remove(index);
}


QVector<bool> InfluentialParameterManager::isUsedInCorrelation() const
{
   return m_isUsedInCorrelation;
}

void InfluentialParameterManager::setIsUsedInCorrelation(const QVector<bool>& isInfluentialParameterUsedInCorrelation)
{
   m_isUsedInCorrelation = isInfluentialParameterUsedInCorrelation;
}

QVector<double> InfluentialParameterManager::optimalParameterValues() const
{
   QVector<double> optimalValues;
   for (const InfluentialParameter* influentialParameter : m_influentialParameters)
   {
      optimalValues.append(influentialParameter->optimalValue());
   }
   return optimalValues;
}

QStringList InfluentialParameterManager::labelNameList() const
{
   QStringList names;
   for (const InfluentialParameter* influentialParameter : m_influentialParameters)
   {
      names << influentialParameter->labelName();
   }
   return names;
}

QStringList InfluentialParameterManager::nameList() const
{
   QStringList names;
   for (const InfluentialParameter* influentialParameter : m_influentialParameters)
   {
      names << influentialParameter->name();
   }
   return names;
}

void InfluentialParameterManager::storeMCMCOutputInParameters(const QVector<QVector<double>>& mcmcInfluentialParameterMatrix)
{
   const int nInfluentialParameters = totalNumberOfInfluentialParameters();

   QVector<double> influentialParametersBestMCData;
   if (mcmcInfluentialParameterMatrix.isEmpty() ||
       mcmcInfluentialParameterMatrix.size() != nInfluentialParameters)
   {
      Logger::log() << "Mismatch in mcmc influential parameter matrix size and number of influential parameters. Could not store output." << Logger::endl();
      return;
   }

   int matrixIdx(0);
   for (InfluentialParameter* parameter : m_influentialParameters)
   {
      int numSubParameters = parameter->totalNumberOfParameters();
      assert(numSubParameters == 1); //Storing mcmc results for influential parameters with multiple sub-parameters is currently not supported.
      if (numSubParameters == 1)
      {
         parameter->setMCMCResult(mcmcInfluentialParameterMatrix.at(matrixIdx));
      }
      matrixIdx += numSubParameters;
   }
}

void InfluentialParameterManager::writeToFile(ScenarioWriter& writer) const
{
   writer.writeValue("InfluentialParameterManagerVersion", m_version);
   writer.writeValue("influentialParameter", m_influentialParameters);
}

void InfluentialParameterManager::readFromFile(const ScenarioReader& reader)
{
   clear();
   m_influentialParameters = reader.readAndCreateVector<InfluentialParameter>("influentialParameter");
   m_isUsedInCorrelation = QVector<bool>(m_influentialParameters.size(), true);
}

void InfluentialParameterManager::clear()
{
   for (InfluentialParameter* influentialParameter : m_influentialParameters)
   {
      delete influentialParameter;
   }
   m_influentialParameters.clear();
   m_isUsedInCorrelation.clear();
}

} // namespace ua

} // namespace casaWizard
