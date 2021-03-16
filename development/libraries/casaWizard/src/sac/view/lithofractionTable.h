//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Table for setting the lithofractions as influential parameters
#pragma once

#include <QTableWidget>
#include <QWidget>

namespace casaWizard
{

class CustomCheckbox;
class ProjectReader;

namespace sac
{

class Lithofraction;
class LithofractionTableRow;

class LithofractionTable : public QTableWidget
{
  Q_OBJECT

public:
  explicit LithofractionTable(QWidget* parent = nullptr);

  void disableProhibitedComponents(const int row);
  void updateLithoPercentagesAndFractions(const QVector<Lithofraction>& lithofractions, int row);
  void updateTable(const QVector<Lithofraction>& lithofractions, const ProjectReader& projectReader);

  static CustomCheckbox* getCheckBoxFromWidget(QWidget* widget);
  static QString percentageToQString(const double d);
  static QString fractionToQString(const double d);

signals:
  void layersFromProject();
  void firstOptimizationChanged(int, int);
  void secondOptimizationChanged(int, int);

private:
  void addRow(const int row, const Lithofraction& lithofraction, const ProjectReader& projectReader, const QStringList& layerNames);
  void addItemsToTable(const int row, const casaWizard::sac::LithofractionTableRow& tableRow);
  void connectCheckBoxSignals(const int row, const LithofractionTableRow tableRow);
  void checkOptimizations(const int row);
  void checkNumberOfLithotypes(const int row);
  bool lessThanThreeLithoTypes(const int row) const;
  bool lessThanTwoLithoTypes(const int row) const;
  QStringList obtainLithoTypes(const ProjectReader& projectReader, const Lithofraction& lithofraction, const QStringList& layerNames);
  void resizeColumnsToContents();
  void setFieldsEnabled(const int row, const std::vector<int>& fields, const bool enabled);
  void setHeaderResizeModes();
  void setHeaderTitles();
  void setHeaderAlignment();
  void setHelpToolTips();
  void setSecondCheckBoxEnabled(const int row, const bool enabled);
  void setTableHeader();  
  void stretchColumns();
};

} // namespace sac

} // namespace casaWizard
