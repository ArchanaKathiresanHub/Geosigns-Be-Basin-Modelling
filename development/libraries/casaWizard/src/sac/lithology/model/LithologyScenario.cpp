#include "LithologyScenario.h"

#include "model/input/cmbMapReader.h"
#include "model/input/projectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "ConstantsMathematics.h"

namespace
{
static int s_sacLithologyScenarioVersion = 0;
}

namespace casaWizard
{

namespace sac
{

namespace lithology
{

const int defaultReferenceSurface{0};
const int defaultt2zNumberCPUs{1};

LithologyScenario::LithologyScenario(ProjectReader* projectReader) :
   SacScenario{projectReader},
   m_lithofractionManager{},
   m_mapsManager{},
   m_t2zLastSurface{projectReader->lowestSurfaceWithTWTData()},
   m_t2zReferenceSurface{defaultReferenceSurface},
   m_t2zSubSampling{1},
   m_t2zRunOnOriginalProject{false},
   m_t2zNumberCPUs{defaultt2zNumberCPUs}
{}

int LithologyScenario::t2zReferenceSurface() const
{
   return m_t2zReferenceSurface;
}

void LithologyScenario::setT2zReferenceSurface(int refSurface)
{
   m_t2zReferenceSurface = refSurface;
}

int LithologyScenario::t2zLastSurface() const
{
   return m_t2zLastSurface;
}

int LithologyScenario::t2zNumberCPUs() const
{
   return m_t2zNumberCPUs;
}

void LithologyScenario::setT2zNumberCPUs(int t2zNumberCPUs)
{
   m_t2zNumberCPUs = t2zNumberCPUs;
}

int LithologyScenario::t2zSubSampling() const
{
   return m_t2zSubSampling;
}

void LithologyScenario::setT2zSubSampling(int t2zSubSampling)
{
   m_t2zSubSampling = t2zSubSampling;
}

bool LithologyScenario::t2zRunOnOriginalProject() const
{
   return m_t2zRunOnOriginalProject;
}

void LithologyScenario::setT2zRunOnOriginalProject(bool t2zRunOnOriginalProject)
{
   m_t2zRunOnOriginalProject = t2zRunOnOriginalProject;
}

LithofractionManager& LithologyScenario::lithofractionManager()
{
   return m_lithofractionManager;
}

const LithofractionManager& LithologyScenario::lithofractionManager() const
{
   return m_lithofractionManager;
}

void LithologyScenario::writeToFile(ScenarioWriter& writer) const
{
   SacScenario::writeToFile(writer);
   writer.writeValue("SACLithologyScenarioVersion", s_sacLithologyScenarioVersion);

   writer.writeValue("referenceSurface", m_t2zReferenceSurface);
   writer.writeValue("t2zSubSampling", m_t2zSubSampling);
   writer.writeValue("t2zRunOnOriginalProject", m_t2zRunOnOriginalProject);
   writer.writeValue("t2zNumberOfCPUs", m_t2zNumberCPUs);

   m_mapsManager.writeToFile(writer);
   m_lithofractionManager.writeToFile(writer);
}

void LithologyScenario::readFromFile(const ScenarioReader& reader)
{
   SacScenario::readFromFile(reader);

   int sacScenarioVersion = reader.readInt("SACScenarioVersion");

   //Note that the version variable of this class is SACLithologyScenarioVersion! sacScenarioVersion is the previous one, before refactoring.
   if (sacScenarioVersion > 1)
   {
      m_t2zSubSampling = reader.readInt("t2zSubSampling") > 0 ? reader.readInt("t2zSubSampling") : m_t2zSubSampling;
      m_t2zRunOnOriginalProject = reader.readBool("t2zRunOnOriginalProject");
      m_t2zNumberCPUs = reader.readInt("t2zNumberOfCPUs");
   }

   m_t2zLastSurface = projectReader().lowestSurfaceWithTWTData();
   m_t2zReferenceSurface = reader.readInt("referenceSurface");

   m_lithofractionManager.readFromFile(reader);
   m_mapsManager.readFromFile(reader);
}

void LithologyScenario::clear()
{
   SacScenario::clear();

   m_t2zReferenceSurface = defaultReferenceSurface;
   m_t2zSubSampling = 1;
   m_t2zRunOnOriginalProject = false;
   m_t2zNumberCPUs = defaultt2zNumberCPUs;

   m_lithofractionManager.clear();
}

LithologyMapsManager& LithologyScenario::mapsManager()
{
   return m_mapsManager;
}

const LithologyMapsManager& LithologyScenario::mapsManager() const
{
   return m_mapsManager;
}

void LithologyScenario::updateT2zLastSurface()
{
   m_t2zLastSurface = projectReader().lowestSurfaceWithTWTData();
}

QVector<OptimizedLithofraction> LithologyScenario::getOptimizedLithoFractionsInLayer(const QString& layer) const
{
   const QVector<Lithofraction>& lithofractions = m_lithofractionManager.lithofractions();

   QVector<OptimizedLithofraction> optimizedLithoFractionsInLayer;
   for (const Well* well : calibrationTargetManager().activeAndIncludedWells())
   {
      QVector<OptimizedLithofraction> optimizedLithoFractions = m_lithofractionManager.optimizedInWell(well->id());
      for (OptimizedLithofraction lithofraction : optimizedLithoFractions)
      {
         if (lithofractions[lithofraction.lithofractionId()].layerName() == layer)
         {
            optimizedLithoFractionsInLayer.push_back(lithofraction);
         }
      }
   }

   return optimizedLithoFractionsInLayer;
}

void LithologyScenario::getVisualisationData(QVector<OptimizedLithofraction>& optimizedLithoFractions, QVector<const Well*>& activeAndIncludedWells, const QString& activeLayer) const
{
   optimizedLithoFractions = getOptimizedLithoFractionsInLayer(activeLayer);
   activeAndIncludedWells = calibrationTargetManager().activeAndIncludedWells();

   if (optimizedLithoFractions.size() != activeAndIncludedWells.size())
   {
      return; // In this layer the wells have not been optimized
   }

   if (mapsManager().smartGridding())
   {
      for (int i = activeAndIncludedWells.size() - 1 ; i >= 0; i--)
      {
         if (!wellHasDataInActiveLayer(activeAndIncludedWells[i], activeLayer))
         {
            optimizedLithoFractions.remove(i);
            activeAndIncludedWells.remove(i);
         }
      }
   }
}

bool LithologyScenario::wellHasDataInActiveLayer(const Well* well, const QString& activeLayer) const
{
   // If the well has no data (meaning the wells were imported before the changes to the calibrationTargetCreator)
   // just always show all wells
   if (well->hasDataInLayer().empty())
   {
      return true;
   }

   return well->hasDataInLayer()[projectReader().getLayerID(activeLayer.toStdString())];
}

QVector<int> LithologyScenario::getHighlightedWells(const QVector<int>& selectedWells, const QString& activeLayer)
{
   QVector<int> highlightedWells;
   if (mapsManager().smartGridding())
   {
      // Shift the selected wells to account for wells which are not plotted, since they do
      // not have data in the active layer
      for (int wellIndex : selectedWells)
      {
         int exclusionShift = 0;
         bool excluded = false;
         int excludedWellIndex = 0;
         for (const Well* well : calibrationTargetManager().activeAndIncludedWells())
         {
            if(excludedWellIndex == wellIndex && !wellHasDataInActiveLayer(well, activeLayer))
            {
               excluded = true;
               break;
            }

            if(wellIndex > excludedWellIndex && !wellHasDataInActiveLayer(well, activeLayer))
            {
               exclusionShift++;
            }

            excludedWellIndex++;
         }

         if (!excluded)
         {
            highlightedWells.push_back(wellIndex - exclusionShift);
         }
      }
   }
   else
   {
      highlightedWells = selectedWells;
   }

   return highlightedWells;
}

bool LithologyScenario::getLithologyTypesAndMaps(const QString& layerName, std::vector<VectorVectorMap>& lithologyMaps, QStringList& lithologyTypes)
{
   const int layerID = projectReader().getLayerID(layerName.toStdString());

   CMBMapReader mapReader;
   const bool optimizedLithomapsAvailable = openMaps(mapReader, layerID);
   if (optimizedLithomapsAvailable)
   {
      lithologyMaps = mapReader.getOptimizedLithoMapsInLayer(layerID);
   }
   else
   {
      mapReader.load(project3dPath().toStdString());
      lithologyMaps = mapReader.getInputLithoMapsInLayer(layerID);
   }

   VectorVectorMap depthMap = mapReader.getMapData(projectReader().getDepthGridName(0).toStdString());
   for (VectorVectorMap& lithologyMap : lithologyMaps)
   {
      lithologyMap.setUndefinedValuesBasedOnReferenceMap(depthMap);
   }

   lithologyTypes = projectReader().lithologyTypesForLayer(layerID);

   return optimizedLithomapsAvailable;
}

std::vector<double> LithologyScenario::getLithopercentagesOfClosestWell(const double xInKm, const double yInKm, const QString& activeLayer, int& closestWellID) const
{
   std::vector<double> lithofractionsAtPoint;

   double xMin = 0; double xMax = 1; double yMin = 0; double yMax = 1;
   projectReader().domainRange(xMin, xMax, yMin, yMax);
   double smallestDistance2 = ((xMax - xMin) / 30)*((xMax - xMin) / 30);

   int closestLithofractionIndex = -1;
   int currentLithofractionIndex = 0;

   const QVector<OptimizedLithofraction>& lithofractionsInLayer = getOptimizedLithoFractionsInLayer(activeLayer);
   for (const auto& optimizedLithofraction : lithofractionsInLayer)
   {
      const int wellId = optimizedLithofraction.wellId();
      const auto& currentWell = calibrationTargetManager().well(wellId);
      const double distance2 = (currentWell.x() - xInKm * Utilities::Maths::KilometerToMeter) * (currentWell.x() - xInKm * Utilities::Maths::KilometerToMeter) +
                               (currentWell.y() - yInKm * Utilities::Maths::KilometerToMeter) * (currentWell.y() - yInKm * Utilities::Maths::KilometerToMeter);
      const bool isCloser = distance2 < smallestDistance2;
      if (isCloser)
      {
         smallestDistance2 = distance2;
         closestLithofractionIndex = currentLithofractionIndex;
         closestWellID = wellId;
      }
      currentLithofractionIndex++;
   }

   if (closestLithofractionIndex!= -1)
   {
      lithofractionsAtPoint.push_back(lithofractionsInLayer[closestLithofractionIndex].optimizedPercentageFirstComponent());
      lithofractionsAtPoint.push_back(lithofractionsInLayer[closestLithofractionIndex].optimizedPercentageSecondComponent());
      lithofractionsAtPoint.push_back(lithofractionsInLayer[closestLithofractionIndex].optimizedPercentageThirdComponent());
   }

   return lithofractionsAtPoint;
}

bool LithologyScenario::openMaps(MapReader& mapReader, const int layerID) const
{
   return SacScenario::openMaps(mapReader, QString::number(layerID) + "_percent_1");
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
