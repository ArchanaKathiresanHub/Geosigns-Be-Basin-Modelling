// Model to manage the manual design points
// Design points that are marked as completed can not be deleted
#pragma once

#include "model/writable.h"

#include <QVector>

namespace casaWizard
{

namespace ua
{

class ManualDesignPointManager : public Writable
{
public:
   explicit ManualDesignPointManager();

   void addDesignPoint();
   bool addDesignPoint(const QVector<double> newPoint);
   void removeDesignPoint(const int index);

   void addInfluentialParameter(const int amount = 1);
   void removeInfluentialParameter(const int index);

   int numberOfPoints() const;
   int numberOfParameters() const;

   void completeAll();
   void setParameterValue(const int designPoint, const int parameterIndex, const double value);

   int numberOfCasesToRun() const;

   QVector<double> getDesignPoint(const int index) const;
   QVector<QVector<double>> parameters() const;
   QVector<bool> completed() const;

   void writeToFile(ScenarioWriter& writer) const override;
   void readFromFile(const ScenarioReader& reader) override;
   void clear() override;

private:
   ManualDesignPointManager(const ManualDesignPointManager&) = delete;
   ManualDesignPointManager& operator=(ManualDesignPointManager) = delete;

   bool pointTooCloseToExistingPoints(const QVector<double> point) const;

   QVector<QVector<double>> influentialParameterValues_;
   QVector<bool> m_completed;
   int numberOfParameters_;
};

} // namespace ua

} // namespace casaWizard
