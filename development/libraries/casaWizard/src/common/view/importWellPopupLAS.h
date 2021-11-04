//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "importWellPopup.h"

class QLabel;

namespace casaWizard
{

class CustomCheckbox;


class ImportWellPopupLAS : public ImportWellPopup
{
  Q_OBJECT
public:
  explicit ImportWellPopupLAS(QWidget *parent = nullptr);
  bool correctForElevation() const;
  void updatePropertyTableWithUnits(const QStringList& propertyUserNames, const QStringList& defaultCauldronNames,
                            const QStringList& availableCauldronNames, const QStringList& units,
                            const QVector<double>& conversionFactors);
  QMap<QString, double> getUnitConversions() const;
  void setElevationInfo(const double elevationCorrection, const QString& elevationCorrectionUnit, const double referenceCorrection, const QString& referenceCorrectionUnit);

signals:
  void propertySelectorChanged(const QString, int);

private slots:
  void slotComboboxChanged(const QString& text, int row);

private:
  CustomCheckbox* correctForElevation_;
  QLabel* depthName_;
};

} // namespace casaWizard
