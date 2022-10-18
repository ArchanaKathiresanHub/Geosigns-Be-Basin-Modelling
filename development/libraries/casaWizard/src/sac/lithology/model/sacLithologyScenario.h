//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Data class for the SAC Wizard
#pragma once

#include "model/lithofractionManager.h"
#include "model/SacScenario.h"
#include "model/MapsManagerLithology.h"

namespace casaWizard
{

class VectorVectorMap;

namespace sac
{

class SacLithologyScenario : public SacScenario
{
public:
   SacLithologyScenario(ProjectReader* projectReader);

   MapsManagerLithology& mapsManager() override;
   const MapsManagerLithology& mapsManager() const override;

   int t2zReferenceSurface() const;
   void setT2zReferenceSurface(int t2zReferenceSurface);

   int t2zLastSurface() const;
   void updateT2zLastSurface();

   int t2zNumberCPUs() const;
   void setT2zNumberCPUs(int t2zNumberCPUs);

   int t2zSubSampling() const;
   void setT2zSubSampling(int t2zSubSampling);

   bool t2zRunOnOriginalProject() const;
   void setT2zRunOnOriginalProject(bool t2zRunOnOriginalProject);

   LithofractionManager& lithofractionManager();
   const LithofractionManager& lithofractionManager() const;

   void writeToFile(ScenarioWriter& writer) const override;
   void readFromFile(const ScenarioReader& reader) override;
   void clear() override;

   void getVisualisationData(QVector<OptimizedLithofraction>& optimizedLithoFractions,
                             QVector<const Well*>& activeAndIncludedWells,
                             const QString& activeLayer) const;

   QVector<int> getHighlightedWells(const QVector<int>& selectedWells, const QString& activeLayer);
   bool getLithologyTypesAndMaps(const QString& layerName, std::vector<VectorVectorMap>& lithologyMaps, QStringList& lithologyTypes);
   std::vector<double> getLithopercentagesOfClosestWell(const double xInKm, const double yInKm, const QString& activeLayer, int& closestWellID) const;

private:
   QVector<OptimizedLithofraction> getOptimizedLithoFractionsInLayer(const QString& layer) const;
   bool wellHasDataInActiveLayer(const Well* well, const QString& activeLayer) const;

   LithofractionManager m_lithofractionManager;
   MapsManagerLithology m_mapsManager;

   int m_t2zLastSurface;
   int m_t2zReferenceSurface;
   int m_t2zSubSampling;
   bool m_t2zRunOnOriginalProject;
   int m_t2zNumberCPUs;
};

} // namespace sac

} // namespace casaWizard
