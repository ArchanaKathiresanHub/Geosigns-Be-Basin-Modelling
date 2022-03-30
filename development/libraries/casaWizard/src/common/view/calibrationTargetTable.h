#pragma once

#include "model/well.h"
#include <QTableWidget>

namespace casaWizard
{

class CalibrationTargetTable : public QTableWidget
{
  Q_OBJECT

public:
  explicit CalibrationTargetTable(QWidget* parent = nullptr);

  virtual void updateTable(const QVector<const Well *> wells, const QMap<QString, QSet<int>>& propertyNamesPerWell);

  void selectAllWells();
  void clearWellSelection();

  QVector<int> getWellSelection() const;

signals:
  void checkBoxChanged(int, int);
  void activePropertyCheckBoxChanged(int, int, QString);

protected:
  void updateTableBase(const QVector<const Well*> wells, const QMap<QString, QSet<int> >& propertyNamesPerWell);

private:
  virtual bool addEnabledCheckBox();
  virtual int wellNameColumn();
  virtual int wellXColumn();
  virtual int wellYColumn();
};

}  // namespace casaWizard
