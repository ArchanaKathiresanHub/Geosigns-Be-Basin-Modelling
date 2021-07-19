#include "model/input/cmbMapReader.h"
#include "model/well.h"
#include "model/wellValidator.h"

#include <gtest/gtest.h>

using namespace casaWizard;

TEST(WellValidatorTest, testInvalidLocationWell)
{
  CMBMapReader mapReader;
  mapReader.load("TestWellValidator.project3d");
  WellValidator validator(mapReader);

  // Nodes (X = invalid, O = valid):
  // (179000, 603500)             (179250, 603500)
  //        X                            O
  //
  // (179000, 603750)             (179250, 603750)
  //        O                            O
  std::vector<Well> invalidWells;
  CalibrationTarget target;
  invalidWells.push_back(Well(0, "Name", 179000, 603500, true, false, {target}));
  invalidWells.push_back(Well(0, "Name", 179125, 603500, true, false, {target}));
  invalidWells.push_back(Well(0, "Name", 179000, 603625, true, false, {target}));
  invalidWells.push_back(Well(0, "Name", 179125, 603625, true, false, {target}));
  invalidWells.push_back(Well(0, "Name", 1, 1, true, false, {target}));

  for (const Well& well : invalidWells)
  {
    EXPECT_EQ(validator.wellState(well, "MAP-27105"), WellState::invalidLocation);
  }
}

TEST(WellValidatorTest, testInvalidDataWell)
{
  CMBMapReader mapReader;
  mapReader.load("TestWellValidator.project3d");
  WellValidator validator(mapReader);

  std::vector<Well> invalidWells;
  CalibrationTarget target;
  Well well(0, "Name", 179250, 603750, true, false);
  EXPECT_EQ(validator.wellState(well, "MAP-27105"), WellState::invalidData);
}

TEST(WellValidatorTest, testValidWell)
{
  CMBMapReader mapReader;
  mapReader.load("TestWellValidator.project3d");
  WellValidator validator(mapReader);

  // Nodes (X = invalid, O = valid):
  // (179000, 603500)             (179250, 603500)
  //        X                            O
  //
  // (179000, 603750)             (179250, 603750)
  //        O                            O
  const Well validWell(0, "Name", 179250, 603750, true, false, {CalibrationTarget()});

  EXPECT_EQ(validator.wellState(validWell, "MAP-27105"), WellState::valid);
}
