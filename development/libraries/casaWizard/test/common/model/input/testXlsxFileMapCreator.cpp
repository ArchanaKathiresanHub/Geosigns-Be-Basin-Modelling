#include "model/input/extractWellDataXlsx.h"
#include "checkQMapKeysAreUnique.h"

#include <gtest/gtest.h>

TEST(TargetXlsxFileMapCreatorTest, testXlsxFileMapsAreUnique)
{
  const casaWizard::ExtractWellDataXlsx myObject;
  const QMap<QString, QString> targetMap{myObject.calibrationTargetVariableMaps()};

  QVector<bool> boolRepeatedKeys = checkQMapKeysAreUnique(targetMap);

  std::size_t i = 0;
  for (const bool& key : boolRepeatedKeys)
  {
    EXPECT_FALSE(key) << "The map key at index [" << i << "] is repeated." ;
    ++i;
  }
}
