// Controller for the manual design points
#pragma once

#include <QObject>
#include <QStringList>

class QTableWidgetItem;

namespace casaWizard
{

namespace ua
{

class ManualDesignPointManager;
class ManualDesignPointTable;

class ManualDesignPointController : public QObject
{
   Q_OBJECT

public:
   ManualDesignPointController(ManualDesignPointTable* table, ManualDesignPointManager& manager, QObject* parent);

   void updateInfluentialParameters(const int number, const QStringList names);

public slots:
   void removeParameter(const int index);

private slots:
   void slotAddButtonClicked();
   void slotDeleteButtonClicked();
   void slotTableItemChanged(QTableWidgetItem* item);

   void slotRefresh();

signals:
   void designPointsChanged();

private:
   ManualDesignPointTable* m_table;
   ManualDesignPointManager& m_manager;

   QStringList m_headerNames;
};

} // namespace ua

} // namespace casaWizard
