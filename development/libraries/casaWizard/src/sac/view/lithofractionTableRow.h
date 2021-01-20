//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#pragma once

#include <QString>

#include <vector>

class QStringList;
class QTableWidgetItem;
class QWidget;

namespace casaWizard
{

class CustomCheckbox;

namespace sac
{

class Lithofraction;

class LithofractionTableRow
{
public:
  LithofractionTableRow(const Lithofraction& lithofraction, const QStringList& lithoTypes);

  QWidget* firstOptimizationCheckBox() const;
  QWidget* secondOptimizationCheckBox() const;
  std::vector<QTableWidgetItem*> lithoTypeItems() const;
  QTableWidgetItem* firstOptimizationMinPercentage() const;
  QTableWidgetItem* firstOptimizationMaxPercentage() const;
  QTableWidgetItem* secondOptimizationMinFraction() const;
  QTableWidgetItem* secondOptimizationMaxFraction() const;
  const Lithofraction& lithofraction() const;

private:
  QWidget* firstOptimizationCheckBox_;
  QWidget* secondOptimizationCheckBox_;
  std::vector<QTableWidgetItem*> lithoTypeItems_;
  QTableWidgetItem* firstOptimizationMinPercentage_;
  QTableWidgetItem* firstOptimizationMaxPercentage_;
  QTableWidgetItem* secondOptimizationMinFraction_;
  QTableWidgetItem* secondOptimizationMaxFraction_;

  const Lithofraction& lithofraction_;
  const QStringList& lithoTypes_;

  void createLithoTypeItems();
  void createOptimizationCheckBoxes();
  void createOptimizationInputFields();
  void initializeCheckBoxes();
  QWidget* createOptimizationCheckBox();
};

} // namespace sac

} // namespace casaWizard
