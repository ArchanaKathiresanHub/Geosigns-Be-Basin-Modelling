// Controller for the influential parameter table, handling adding and removing rows
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

namespace ua
{

class InfluentialParameterTable;
class InfluentialParameterManager;

class InfluentialParameterController : public QObject
{
  Q_OBJECT

public:
  InfluentialParameterController(InfluentialParameterTable* table, InfluentialParameterManager& manager, QObject* parent);

private slots:
  void slotPushButtonAddInfluentialParameterClicked();
  void slotPushButtonDelInfluentialParameterClicked();
  void slotArgumentsPopup(const int row);
  void slotChangeInfluentialParameter(const int row, const int selected);

  void slotRefresh();

signals:
  void influentialParametersChanged();
  void removeInfluentialParameter(int row);

private:
  InfluentialParameterTable* table_;
  InfluentialParameterManager& manager_;
};

} // namespace ua

} // namespace casaWizard
