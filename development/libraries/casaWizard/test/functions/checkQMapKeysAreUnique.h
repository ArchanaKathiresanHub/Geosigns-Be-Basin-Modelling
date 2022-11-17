// Function to assert that there are no repeating keys in the map (having multiple repeated keys is possible in QMap)

#pragma once

#include <gtest/gtest.h>
#include <QMap>
#include <QString>
#include <QVector>

template<class T>
QVector<bool> checkQMapKeysAreUnique(const QMap<QString, T>& targetMap)
{
  typename QMap<QString, T>::const_iterator it = targetMap.constBegin();
  QVector<bool> boolRepeatedKeys;

  while (it != targetMap.end() - 1)
  {
    bool boolContainsRepeated = false;
    const QString key_Actual = it.key();

    typename QMap<QString, T>::const_iterator it_j = it + 1;
    while (it_j != targetMap.end() && !boolContainsRepeated)
    {
      const QString key_NotExpected = it_j.key();
      if (key_NotExpected == key_Actual)
      {
        boolContainsRepeated = true;
      }

      ++it_j;
    }

    boolRepeatedKeys.push_back(boolContainsRepeated);

    ++it;
  }

  return boolRepeatedKeys;
}

template<class T>
bool checkQMapKeysAreNotUnique(const QMap<QString, T>& targetMap)
{
  typename QMap<QString, T>::const_iterator it = targetMap.constBegin();
  bool boolContainsRepeated;

  boolContainsRepeated = false;
  while (it != targetMap.end() - 1 && !boolContainsRepeated)
  {
    const QString key_Actual = it.key();

    typename QMap<QString, T>::const_iterator it_j = it + 1;
    while (it_j != targetMap.end() && !boolContainsRepeated)
    {
      const QString key_NotExpected = it_j.key();
      if (key_NotExpected == key_Actual)
      {
        boolContainsRepeated = true;
      }

      ++it_j;
    }

    ++it;
  }

  return boolContainsRepeated;
}
