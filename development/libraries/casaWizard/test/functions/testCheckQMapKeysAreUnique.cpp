#include "checkQMapKeysAreUnique.h"

#include <gtest/gtest.h>

TEST (TestCheckQMapKeysAreUnique, nonRepeatingMapTest)
{
  QMap<QString, QString> targetMap;

  targetMap.insert("tar1", "a1");
  targetMap.insert("tar2", "a2");
  targetMap.insert("tar3", "a3");

  QVector<bool> boolRepeatedKeyIndices = checkQMapKeysAreUnique(targetMap);

  std::size_t i = 0;
  for (const bool& key : boolRepeatedKeyIndices)
  {
    EXPECT_FALSE(key) << "The map key at index [" << i << "] is repeated." ;
    ++i;
  }
}

TEST (TestCheckQMapKeysAreUnique, repeatingMapTest)
{
  QMap<QString, QString> targetMap;

  targetMap.insert("tar1", "a1");
  targetMap.insert("tar2", "a2");
  targetMap.insertMulti("tar1", "a3");

  bool boolMapHasRepeatedElements = checkQMapKeysAreNotUnique(targetMap);

  EXPECT_TRUE(boolMapHasRepeatedElements) << "No repeated keys found in the map!";
}
