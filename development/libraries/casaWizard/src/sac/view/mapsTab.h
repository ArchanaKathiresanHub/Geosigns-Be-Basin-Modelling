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

  void updateBirdsView(const QVector<const Well*> wells);

  QPushButton* createGridsButton() const;
  QComboBox* interpolationType() const;
  LithofractionVisualisation* lithofractionVisualisation() const;
  QSpinBox* pValue() const;
  QSpinBox* smoothingRadius() const;
  QComboBox* smoothingType() const;
  QSpinBox* threads() const;
  ActiveWellsTable* activeWellsTable() const;

private slots:
  void slotInterpolationTypeChange(int interpolationType);
  void slotSmoothingTypeChange(int smoothingType);

private:
  void connectSignalsAndSlots() const;
  void setDefaultGridGenerationOptions();
  void setGridGenerationOptionsLayout();
  void setIdwOptionsLayout();
  void setSmoothingOptionsLayout();
  void setTotalLayout(QVBoxLayout* wellsAndOptions);
  QVBoxLayout* setWellsAndOptionsLayout();

  EmphasisButton* createGridsButton_;
  QComboBox* interpolationType_;
  QWidget* iwdOptions_;
  QWidget* gridGenerationOptions_;
  LithofractionVisualisation* lithofractionVisualisation_;
  QSpinBox* pValue_;
  QWidget* smoothingOptions_;
  QSpinBox* smoothingRadius_;
  QComboBox* smoothingType_;
  QSpinBox* threads_;
  ActiveWellsTable* activeWellsTable_;
};

} // namespace sac

} // namespace casaWizard