//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Tab to display the results, as well as the results from the 1D case
#pragma once

#include <QWidget>

class QLabel;
class QListWidget;
class QComboBox;
class QPushButton;
class QSpinBox;
class QVBoxLayout;
class EmphasisButton;

namespace casaWizard
{

class CustomCheckbox;
class Well;

namespace sac
{

class ActiveWellsTable;
class OptimizationOptions;
class LithofractionVisualisation;

class MapsTab : public QWidget
{
  Q_OBJECT

public:
  explicit MapsTab(QWidget* parent = nullptr);

  QPushButton* createGridsButton() const;
  QPushButton* buttonExportOptimized() const;
  QPushButton* buttonRunOptimized() const;  
  QComboBox* interpolationType() const;
  LithofractionVisualisation* lithofractionVisualisation() const;
  QSpinBox* pValue() const;
  QSpinBox* smoothingRadius() const;
  QComboBox* smoothingType() const;  
  CustomCheckbox* smartGridding() const;
  ActiveWellsTable* activeWellsTable() const;
  int numberOfActiveWells() const;

  void highlightWell(const QString& wellName);
  void disableWellAtIndex(const int index);

private slots:
  void slotInterpolationTypeChange(int interpolationType);
  void slotSmoothingTypeChange(int smoothingType);

private:
  void connectSignalsAndSlots() const;
  void setDefaultGridGenerationOptions();
  void setGridGenerationOptionsLayout();
  void setIdwOptionsLayout();
  void setSmoothingOptionsLayout();
  void setTotalLayout();
  QVBoxLayout* setWellsAndOptionsLayout();

  ActiveWellsTable* activeWellsTable_;
  LithofractionVisualisation* lithofractionVisualisation_;
  EmphasisButton* createGridsButton_;
  QPushButton* buttonExportOptimized_;
  QPushButton* buttonRunOptimized_;  
  QComboBox* interpolationType_;
  QWidget* iwdOptions_;
  QWidget* gridGenerationOptions_;
  QSpinBox* pValue_;
  QWidget* smoothingOptions_;
  QSpinBox* smoothingRadius_;
  QComboBox* smoothingType_;  
  CustomCheckbox* smartGridding_;
};

} // namespace sac

} // namespace casaWizard
