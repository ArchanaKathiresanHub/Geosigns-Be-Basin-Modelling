#include "sacLithologyScenario.h"

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

const int defaultReferenceSurface{0};
const int defaultt2zNumberCPUs{1};

SacLithologyScenario::SacLithologyScenario(ProjectReader* projectReader) :
   SacScenario{projectReader},
   lithofractionManager_{},
   t2zLastSurface_{projectReader->lowestSurfaceWithTWTData()},
   t2zReferenceSurface_{defaultReferenceSurface},
   t2zSubSampling_{1},
   t2zRunOnOriginalProject_{false},
   t2zNumberCPUs_{defaultt2zNumberCPUs}
{}

int SacLithologyScenario::t2zReferenceSurface() const
{
   return t2zReferenceSurface_;
}

void SacLithologyScenario::setT2zReferenceSurface(int refSurface)
{
   t2zReferenceSurface_ = refSurface;
}

int SacLithologyScenario::t2zLastSurface() const
{
   return t2zLastSurface_;
}

int SacLithologyScenario::t2zNumberCPUs() const
{
   return t2zNumberCPUs_;
}

void SacLithologyScenario::setT2zNumberCPUs(int t2zNumberCPUs)
{
   t2zNumberCPUs_ = t2zNumberCPUs;
}

int SacLithologyScenario::t2zSubSampling() const
{
   return t2zSubSampling_;
}

void SacLithologyScenario::setT2zSubSampling(int t2zSubSampling)
{
   t2zSubSampling_ = t2zSubSampling;
}

bool SacLithologyScenario::t2zRunOnOriginalProject() const
{
   return t2zRunOnOriginalProject_;
}

void SacLithologyScenario::setT2zRunOnOriginalProject(bool t2zRunOnOriginalProject)
{
   t2zRunOnOriginalProject_ = t2zRunOnOriginalProject;
}

LithofractionManager& SacLithologyScenario::lithofractionManager()
{
   return lithofractionManager_;
}

const LithofractionManager& SacLithologyScenario::lithofractionManager() const
{
   return lithofractionManager_;
}

void SacLithologyScenario::writeToFile(ScenarioWriter& writer) const
{
   SacScenario::writeToFile(writer);
   writer.writeValue("SACLithologyScenarioVersion", s_sacLithologyScenarioVersion);

   writer.writeValue("referenceSurface", t2zReferenceSurface_);
   writer.writeValue("t2zSubSampling", t2zSubSampling_);
   writer.writeValue("t2zRunOnOriginalProject", t2zRunOnOriginalProject_);
   writer.writeValue("t2zNumberOfCPUs", t2zNumberCPUs_);

   lithofractionManager_.writeToFile(writer);
}

void SacLithologyScenario::readFromFile(const ScenarioReader& reader)
{
   SacScenario::readFromFile(reader);

   int sacScenarioVersion = reader.readInt("SACScenarioVersion");

   //Note that the version variable of this class is SACLithologyScenarioVersion! sacScenarioVersion is the previous one, before refactoring.
   if (sacScenarioVersion > 1)
   {
      t2zSubSampling_ = reader.readInt("t2zSubSampling") > 0 ? reader.readInt("t2zSubSampling") : t2zSubSampling_;
      t2zRunOnOriginalProject_ = reader.readBool("t2zRunOnOriginalProject");
      t2zNumberCPUs_ = reader.readInt("t2zNumberOfCPUs");
   }

   t2zLastSurface_ = projectReader().lowestSurfaceWithTWTData();
   t2zReferenceSurface_ = reader.readInt("referenceSurface");

   lithofractionManager_.readFromFile(reader);
}

void SacLithologyScenario::clear()
{
   SacScenario::clear();

   t2zReferenceSurface_ = defaultReferenceSurface;
   t2zSubSampling_ = 1;
   t2zRunOnOriginalProject_ = false;
   t2zNumberCPUs_ = defaultt2zNumberCPUs;

   lithofractionManager_.clear();
}

void SacLithologyScenario::updateT2zLastSurface()
{
   t2zLastSurface_ = projectReader().lowestSurfaceWithTWTData();
}

QVector<OptimizedLithofraction> SacLithologyScenario::getOptimizedLithoFractionsInLayer(const QString& layer) const
{
   const QVector<Lithofraction>& lithofractions = lithofractionManager_.lithofractions();

   QVector<OptimizedLithofraction> optimizedLithoFractionsInLayer;
   for (const Well* well : calibrationTargetManager().activeAndIncludedWells())
   {
      QVector<OptimizedLithofraction> optimizedLithoFractions = lithofractionManager_.optimizedInWell(well->id());
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

void SacLithologyScenario::getVisualisationData(QVector<OptimizedLithofraction>& optimizedLithoFractions, QVector<const Well*>& activeAndIncludedWells, const QString& activeLayer) const
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

bool SacLithologyScenario::wellHasDataInActiveLayer(const Well* well, const QString& activeLayer) const
{
   // If the well has no data (meaning the wells were imported before the changes to the calibrationTargetCreator)
   // just always show all wells
   if (well->hasDataInLayer().empty())
   {
      return true;
   }

   return well->hasDataInLayer()[projectReader().getLayerID(activeLayer.toStdString())];
}

void SacLithologyScenario::exportOptimizedLithofractionMapsToZycor(const QString& targetPath)
{
   CMBMapReader mapReader;
   mapReader.load((optimizedProjectDirectory() + project3dFilename()).toStdString());
   mapsManager().exportOptimizedLithofractionMapsToZycor(projectReader(), mapReader, targetPath);
}

QVector<int> SacLithologyScenario::getHighlightedWells(const QVector<int>& selectedWells, const QString& activeLayer)
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

bool SacLithologyScenario::getLithologyTypesAndMaps(const QString& layerName, std::vector<VectorVectorMap>& lithologyMaps, QStringList& lithologyTypes)
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

std::vector<double> SacLithologyScenario::getLithopercentagesOfClosestWell(const double xInKm, const double yInKm, const QString& activeLayer, int& closestWellID) const
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

} // namespace sac

} // namespace casaWizard
