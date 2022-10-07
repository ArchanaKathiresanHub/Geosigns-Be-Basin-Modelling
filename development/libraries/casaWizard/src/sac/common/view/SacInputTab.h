//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>
#include <QGridLayout>

class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;
class EmphasisButton;

namespace casaWizard
{

class CalibrationTargetTable;
class ObjectiveFunctionTableSAC;

namespace sac
{

class SacInputTab : public QWidget
{
   Q_OBJECT

public:
   explicit SacInputTab(QWidget* parent = nullptr);

   CalibrationTargetTable* calibrationTargetTable() const;
   ObjectiveFunctionTableSAC* objectiveFunctionTable() const;

   QLineEdit* lineEditProject3D() const;
   const QPushButton* pushSelectProject3D() const;
   QComboBox* comboBoxApplication() const;
   QComboBox* comboBoxCluster() const;
   const QPushButton* pushSelectAllWells() const;
   const QPushButton* pushClearSelection() const;
   const QPushButton* pushRun1DOptimalization() const;
   const QPushButton* buttonRunOriginal1D() const;
   const QPushButton* buttonRunOriginal3D() const;

   void setContentsActive(bool state);

protected:
   void addWidget(QWidget* widget, QString title = "");
   QHBoxLayout* layoutCalibrationOptions() const;
   QVBoxLayout* layoutRunOptions() const;

private:
   QWidget* m_subLayoutActivationWidget;
   QPushButton* m_pushSelectProject3D;
   QLineEdit* m_lineEditProject3D;

   QGridLayout* m_layoutTablesAndOptions;
   QHBoxLayout* m_layoutCalibrationOptions;
   QVBoxLayout* m_layoutRunOptions;
   CalibrationTargetTable* m_calibrationTargetTable;
   ObjectiveFunctionTableSAC* m_objectiveFunctionTable;

   QPushButton* m_pushSelectAllWells;
   QPushButton* m_pushClearSelection;

   QComboBox* m_comboBoxApplication;
   QComboBox* m_comboBoxCluster;

   EmphasisButton* m_pushButtonRun1DOptimalization;
   QPushButton* m_buttonRunOriginal1D;
   QPushButton* m_buttonRunOriginal3D;
};

} // namespace sac

} // namespace casaWizard
