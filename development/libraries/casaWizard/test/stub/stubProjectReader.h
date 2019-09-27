#pragma once
#include "model/input/projectReader.h"

namespace casaWizard
{

class StubProjectReader : public ProjectReader
{
public:
    void load(const QString& /*projectFile*/) override
    {
    }

    QStringList layerNames() const override
    {
        return QStringList("Layer");
    }

    QStringList lithologyNames() const override
    {
        return QStringList();
    }

    QStringList mapNames() const override
    {
        return QStringList();
    }

    QStringList lithologyTypesForLayer(const int /*layerIndex*/) const override
    {
        return QStringList();
    }

    double heatProductionRate() const override
    {
        return 0.0;
    }

    double initialLithosphericMantleThickness() const override
    {
        return 0.0;
    }

    double equilibriumOceanicLithosphereThickness() const override
    {
        return 0.0;
    }

    QVector<double> agesFromMajorSnapshots() const override
    {
        return {0, 23, 66, 100, 201, 252, 255, 299};
    }
};

} // namespace casaWizard
