#include "model/input/cmbMapReader.h"
#include "model/well.h"
#include "model/wellValidator.h"

#include <gtest/gtest.h>

using namespace casaWizard;

TEST(WellValidatorTest, testInvalidWell)
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
  invalidWells.push_back(Well(0, "Name", 179000, 603500));
  invalidWells.push_back(Well(0, "Name", 179125, 603500));
  invalidWells.push_back(Well(0, "Name", 179000, 603625));
  invalidWells.push_back(Well(0, "Name", 179125, 603625));
  invalidWells.push_back(Well(0, "Name", 1, 1));

  for (const Well& well : invalidWells)
  {
    EXPECT_FALSE(validator.isValid(well, "MAP-27105"));
  }
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
  const Well validWell(0, "Name", 179250, 603750);

  EXPECT_TRUE(validator.isValid(validWell, "MAP-27105"));
}
