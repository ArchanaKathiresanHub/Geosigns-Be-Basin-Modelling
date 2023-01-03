// Tab to display the results, as well as the results from the 1D case
#pragma once

#include <QWidget>
#include "view/SacResultsTab.h"

namespace casaWizard
{

namespace sac
{

namespace lithology
{

struct Lithofraction;
struct OptimizedLithofraction;

class LithologyResultsTab : public SacResultsTab
{
   Q_OBJECT

public:
   explicit LithologyResultsTab(QWidget* parent = nullptr);

   QButtonGroup* wellPrepOrSAC() const;

   void updateOptimizedLithoTable(const QStringList& layerNameList,
                                  const QVector<QStringList>& lithoNamesVector,
                                  const QVector<QVector<double> >& originalValuesVector,
                                  const QVector<QVector<double> >& optimizedValuesVector);
private:
   void addLithofractionRow(int &row, QVector<QTableWidgetItem*> items, const double diff);
   void setVisibleLithofractionColumn(const bool visible);
   QButtonGroup* wellPrepOrSAC_;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
