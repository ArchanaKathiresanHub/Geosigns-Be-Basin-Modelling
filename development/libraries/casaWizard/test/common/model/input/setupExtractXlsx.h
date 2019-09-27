#pragma once

#include "model/input/extractWellDataXlsx.h"
#include "wellTestData.h"
#include <gtest/gtest.h>

namespace testCasaThermalWizard
{

class setupExtractXlsx : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  {

  }

  static QVector<QString> extractedWellNames_;
  static QVector<Well> extractedWell_;
  static QVector<WellTargetVariables> extractedWellTargetVariables_;
  static QVector<WellData> extractedWellData_;
};

QVector<QString> setupExtractXlsx::extractedWellNames_;
QVector<Well> setupExtractXlsx::extractedWell_;
QVector<WellTargetVariables> setupExtractXlsx::extractedWellTargetVariables_;
QVector<WellData> setupExtractXlsx::extractedWellData_;

} // namespace testCasaThermalWizard
