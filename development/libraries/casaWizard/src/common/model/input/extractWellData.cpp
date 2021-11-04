//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "extractWellData.h"
#include "model/wellData.h"

namespace casaWizard
{

ExtractWellData::ExtractWellData() :
  wellData_{new WellData{}},
  calibrationTargetVariableMaps_{}
{
  setCalibrationTargetVariableMaps();
}

ExtractWellData::~ExtractWellData()
{

}

double ExtractWellData::xCoord() const
{
  return wellData_->xCoord_;
}

double ExtractWellData::yCoord() const
{
  return wellData_->yCoord_;
}

const QString& ExtractWellData::metaData() const
{
  return wellData_->metaData_;
}

const QString&ExtractWellData::wellName() const
{
  return wellData_->wellName_;
}

QVector<double> ExtractWellData::depth() const
{
  return wellData_->depth_;
}

unsigned int ExtractWellData::nCalibrationTargetVars() const
{
  return wellData_->nCalibrationTargetVars_;
}

QVector<QString> ExtractWellData::calibrationTargetVarsUserName() const
{
  return wellData_->calibrationTargetVarsUserName_;
}

QVector<QString> ExtractWellData::calibrationTargetVarsCauldronName() const
{
  return wellData_->calibrationTargetVarsCauldronName_;
}

QVector<unsigned int> ExtractWellData::nDataPerTargetVar() const
{
  return wellData_->nDataPerTargetVar_;
}

QVector<double> ExtractWellData::calibrationTargetValues() const
{
  return wellData_->calibrationTargetValues_;
}

QVector<double> ExtractWellData::calibrationTargetStdDeviation() const
{
  return wellData_->calibrationTargetStdDeviation_;
}

QMap<QString, QString> ExtractWellData::calibrationTargetVariableMaps() const
{
  return calibrationTargetVariableMaps_;
}

QVector<QString> ExtractWellData::units() const
{
  return wellData_->units_;
}

void ExtractWellData::setCalibrationTargetVariableMaps()
{
  calibrationTargetVariableMaps_.insert("TwoWayTime", "TwoWayTime");
  calibrationTargetVariableMaps_.insert("Two way time", "TwoWayTime");
  calibrationTargetVariableMaps_.insert("TWTT", "TwoWayTime");
  calibrationTargetVariableMaps_.insert("SAC-TWTT_from_DT", "TWT_FROM_DT");
  calibrationTargetVariableMaps_.insert("SAC-TWTT_from_DT_from_VP", "TWT_FROM_DT");
  calibrationTargetVariableMaps_.insert("BulkDensity", "BulkDensity");
  calibrationTargetVariableMaps_.insert("Density", "BulkDensity");
  calibrationTargetVariableMaps_.insert("SonicSlowness", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("DT", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("DTfromVP", "DT_FROM_VP");
  calibrationTargetVariableMaps_.insert("Temperature", "Temperature");
  calibrationTargetVariableMaps_.insert("Vr", "VRe");
  calibrationTargetVariableMaps_.insert("Vre", "VRe");
  calibrationTargetVariableMaps_.insert("VRe", "VRe");
  calibrationTargetVariableMaps_.insert("GammaRay", "GammaRay");
  calibrationTargetVariableMaps_.insert("GR", "GammaRay");
  calibrationTargetVariableMaps_.insert("Pressure", "Pressure");
  calibrationTargetVariableMaps_.insert("Velocity", "Velocity");
  calibrationTargetVariableMaps_.insert("VP", "Velocity");
  calibrationTargetVariableMaps_.insert("GMUA_update_bgg_31_velocity_global_vo__vel", "TWTT");
  calibrationTargetVariableMaps_.insert("RHOB", "BulkDensity");
  calibrationTargetVariableMaps_.insert("Den", "BulkDensity");
  calibrationTargetVariableMaps_.insert("DEN_E", "BulkDensity");
  calibrationTargetVariableMaps_.insert("DEN", "BulkDensity");
  calibrationTargetVariableMaps_.insert("den", "BulkDensity");
  calibrationTargetVariableMaps_.insert("RHOB_GP", "BulkDensity");
  calibrationTargetVariableMaps_.insert("TRDE_00J", "GammaRay");
  calibrationTargetVariableMaps_.insert("DEN_E_00Q", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("DT_00J", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("Dt", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("DTL_00J", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("GRfinal", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("Somalia_VINT_Reg_model_v1_lkey_Vint_vo__Vint", "Velocity");
  calibrationTargetVariableMaps_.insert("SONI", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("DT_GP", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("DTC", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("DTfinal", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("DTC_E", "SonicSlowness");
  calibrationTargetVariableMaps_.insert("TRGR", "GammaRay");
}

void ExtractWellData::mapTargetVarNames()
{
  for (QString& calTargetVar : wellData_->calibrationTargetVarsUserName_)
  {
    wellData_->calibrationTargetVarsCauldronName_.push_back(calibrationTargetVariableMaps_.value(calTargetVar, QString("Unknown")));
  }
}

} // namespace casaWizard
