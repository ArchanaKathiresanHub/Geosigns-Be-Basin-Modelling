#include "model/targetParameterMapCreator.h"
#include "checkQMapKeysAreUnique.h"

#include <gtest/gtest.h>

TEST(TargetParameterMapCreatorTest, testTargetParameterMapsAreUnique)
{
  const QMap<QString, QString> targetMap{casaWizard::targetParameterMapCreator::targetParametersMap()};

  QVector<bool> boolRepeatedKeys = checkQMapKeysAreUnique(targetMap);

  std::size_t i = 0;
  for (const bool& key : boolRepeatedKeys)
  {
    EXPECT_FALSE(key) << "The map key at index [" << i << "] is repeated." ;
    ++i;
  }
}
