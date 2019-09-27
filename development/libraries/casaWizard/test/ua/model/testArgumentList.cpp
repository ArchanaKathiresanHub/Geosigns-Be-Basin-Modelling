#include "model/argumentList.h"

#include <gtest/gtest.h>

TEST( ArgumentListTest, TestGetDoubleArgument )
{
  casaWizard::ua::ArgumentList argList{};
  double valueExpected{123};
  argList.addArgument("", valueExpected);

  double valueActual{argList.getDoubleArgument(0)};

  EXPECT_EQ(valueExpected, valueActual);
}

TEST( ArgumentListTest, TestGetListValue )
{
  casaWizard::ua::ArgumentList argList{};
  QStringList valueExpected{"Test 1", "Test 2"};
  argList.addArgument("", valueExpected);

  QStringList valueActual{argList.getListOptions(0)};

  EXPECT_EQ(valueExpected, valueActual);
}

TEST( ArgumentListTest, TestGetDoubleAndListValue )
{
   casaWizard::ua::ArgumentList argList{};
   QStringList listExpected{"Test 1", "Test 2"};
   double doubleExpected{234};

   argList.addArgument("", listExpected);
   argList.addArgument("", doubleExpected);

   QStringList listActual{argList.getListOptions(0)};
   double doubleActual{argList.getDoubleArgument(1)};

   EXPECT_EQ(listExpected, listActual);
   EXPECT_EQ(doubleExpected, doubleActual);
}

TEST( ArgumentListTest, TestGetValue )
{
   casaWizard::ua::ArgumentList argList{};

   QString listItemExpected{"Test 1"};
   QStringList listExpected{listItemExpected, "Test 2"};
   double doubleExpected{345};

   argList.addArgument("", listExpected);
   argList.addArgument("", doubleExpected);

   QString listItemActual{argList.getSelectedValue(0)};
   double doubleActual{argList.getDoubleArgument(1)};

   EXPECT_EQ(listItemExpected, listItemActual) << " String mismatch. \nExpected: " << listItemExpected.toStdString()
                                               << " \nActual:   " << listItemActual.toStdString();

   EXPECT_EQ(doubleExpected, doubleActual);
}

TEST( ArgumentListTest, TestToString )
{
  casaWizard::ua::ArgumentList argList{};

  argList.addArgument("", 10);
  argList.addArgument("", 20);

  QString stringExpected{"10 20"};
  QString stringActual{argList.toString()};

  EXPECT_EQ(stringExpected, stringActual) << " String mismatch. \nExpected: " << stringExpected.toStdString()
                                          << " \nActual:   " << stringActual.toStdString();
}

TEST( ArgumetListTest, TestToStringWithList )
{
  casaWizard::ua::ArgumentList argList{};
  argList.addArgument("", 10);
  argList.addArgument("", {"Option1", "Option2"});
  argList.setListValues({"Option2"});

  QString stringExpected{"10 \"Option2\""};
  QString stringActual{argList.toString()};

  EXPECT_EQ(stringExpected, stringActual) << " String mismatch. \nExpected: " << stringExpected.toStdString()
                                          << " \nActual:   " << stringActual.toStdString();
}
