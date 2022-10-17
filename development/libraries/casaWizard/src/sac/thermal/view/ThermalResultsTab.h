// Tab to display the results, as well as the results from the 1D case
#pragma once

#include <QWidget>
#include "view/SacResultsTab.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalResultsTab : public SacResultsTab
{
   Q_OBJECT

public:
   explicit ThermalResultsTab(QWidget* parent = nullptr);

   void updateOptimizedHeatProductionTable(const QStringList& wellName,
                                           const QVector<double>& originalValue,
                                           const QVector<double>& OptimizedValue);

   void setPlotType(const int currentIndex) final;
private:
   void addRow(int& row, QVector<QTableWidgetItem*> items);
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
