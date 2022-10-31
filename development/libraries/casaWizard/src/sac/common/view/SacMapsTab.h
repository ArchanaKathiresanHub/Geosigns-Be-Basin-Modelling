//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Tab to display the results, as well as the results from the 1D case
#pragma once

#include <QWidget>

class QComboBox;
class QPushButton;
class QSpinBox;
class QVBoxLayout;
class EmphasisButton;

namespace casaWizard
{

namespace sac
{

class SacMapsVisualisation;
class ActiveWellsTable;

class SacMapsTab : public QWidget
{
   Q_OBJECT

public:
   SacMapsTab(QWidget* parent = nullptr);
   virtual ~SacMapsTab() = default;

   virtual SacMapsVisualisation* mapsVisualisation() const = 0;

   QPushButton* createGridsButton() const;
   QSpinBox* pValue() const;
   QSpinBox* smoothingRadius() const;
   QComboBox* smoothingType() const;
   QPushButton* buttonExportOptimized() const;
   QPushButton* buttonExportOptimizedToZycor() const;
   QPushButton* buttonRunOptimized() const;
   QComboBox* interpolationType() const;

   QPushButton* pushSelectAllWells() const;
   QPushButton* pushClearWellSelection() const;

   void highlightWell(const QString& wellName);
   void disableWellAtIndex(const int index);

   ActiveWellsTable* activeWellsTable() const;
   int numberOfActiveWells() const;

protected:
   QWidget* idwOptions() const;
   QWidget* gridGenerationOptions() const;
   QWidget* smoothingOptions() const;

   void setIdwOptionsLayout();
   void setDefaultGridGenerationOptions();
   QVBoxLayout* setWellsAndOptionsLayout();
   void setSmoothingOptionsLayout();
   virtual void setGridGenerationOptionsLayout() = 0;

private slots:
   void slotInterpolationTypeChange(int interpolationType);
   void slotSmoothingTypeChange(int smoothingType);

private:
   void connectSignalsAndSlots();

   ActiveWellsTable* m_activeWellsTable;
   EmphasisButton* m_createGridsButton;
   QPushButton* m_buttonExportOptimized;
   QPushButton* m_buttonExportOptimizedToZycor;
   QPushButton* m_buttonRunOptimized;
   QComboBox* m_interpolationType;
   QWidget* m_idwOptions;
   QWidget* m_gridGenerationOptions;
   QSpinBox* m_pValue;
   QWidget* m_smoothingOptions;
   QSpinBox* m_smoothingRadius;
   QComboBox* m_smoothingType;
   QPushButton* m_pushSelectAllWells;
   QPushButton* m_pushClearWellSelection;
};

} // namespace sac

} // namespace casaWizard
