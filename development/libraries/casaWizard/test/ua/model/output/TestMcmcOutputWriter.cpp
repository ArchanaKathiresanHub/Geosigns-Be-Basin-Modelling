//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "expectFileEq.h"
#include "model/output/McmcOutputWriter.h"
#include "model/output/McmcTargetExportData.h"

#include <gtest/gtest.h>

using namespace casaWizard;
using namespace ua;

void writeAndCompare(QString expectedFile, QString actualFile, const McmcTargetExportData& data)
{
   McmcOutputWriter::writeToFile(actualFile,data);
   expectFileEq(expectedFile.toStdString(), actualFile.toStdString());

   if (QFileInfo::exists(actualFile))
   {
      QFile file (actualFile);
      file.remove();
   }
}

TEST(McmcOutput, testNoWrite )
{
   McmcTargetExportData exportData;

   const QString outFile{"fileShouldNotBeThere.csv"};
   McmcOutputWriter::writeToFile(outFile,exportData);
   EXPECT_FALSE(QFileInfo::exists(outFile));

   //If the target output property name is not in allpropnames, output should not be written (exportData is incorrectly constructed).
   McmcSingleTargetExportData t1;
   t1.targetOutputs["p2"] = TargetDataSingleProperty();
   exportData.targetData["t"] = t1;

   McmcOutputWriter::writeToFile(outFile,exportData);
   EXPECT_FALSE(QFileInfo::exists(outFile));

   //If all propNames only has a another target, output is not written
   exportData.allPropNames.insert("p1");
   McmcOutputWriter::writeToFile(outFile,exportData);
   EXPECT_FALSE(QFileInfo::exists(outFile));

   //If the property name of one property is in allpropnames, but another one is not, the output data is incorrectly constructed and output is not written:
   McmcSingleTargetExportData t2;
   t2.targetOutputs["p1"] = TargetDataSingleProperty();
   exportData.targetData["h1"] =t2;

   McmcOutputWriter::writeToFile(outFile,exportData);
   EXPECT_FALSE(QFileInfo::exists(outFile));

   //Cleanup, in case one of the tests failed:
   if (QFileInfo::exists(outFile))
   {
      QFile file (outFile);
      file.remove();
   }

}

TEST(McmcOutput, testNormalWrite )
{
   McmcTargetExportData exportData;
   exportData.allPropNames.insert("p1");

   McmcSingleTargetExportData singleTargetData;
   singleTargetData.targetOutputs["p1"] = TargetDataSingleProperty();
   exportData.targetData["h1"] =singleTargetData;

   QString expectedFile{"McmcOutputExpected.csv"};
   QString actualFile{"McmcOutputActual.csv"};

   writeAndCompare(expectedFile,actualFile,exportData);

   //Test one target with a single property and another one with two properties.
   exportData.allPropNames.insert("p2");
   singleTargetData.targetOutputs["p2"] = TargetDataSingleProperty();
   exportData.targetData["h2"] =singleTargetData;

   expectedFile = "McmcOutputExpected2Prop.csv";

   writeAndCompare(expectedFile,actualFile,exportData);
}

TEST(McmcOutput, testSpecialCharacters )
{
   McmcTargetExportData exportData;
   exportData.allPropNames.insert("^\a\b\f\r\v\54\t\n^%//");

   McmcSingleTargetExportData singleTargetData;
   singleTargetData.targetOutputs["^\a\b\f\r\v\54\t\n^%//"] = TargetDataSingleProperty();
   singleTargetData.locationName = "^\a\b\f\r\v\54\t\n^%//";
   exportData.targetData["^\a\b\f\r\v\54\t\n^%//"] = singleTargetData;

   const QString expectedFile{"McmcOutputExpectedSpecialCharacters.csv"};
   const QString actualFile{"McmcOutputActual.csv"};

   writeAndCompare(expectedFile,actualFile,exportData);
}
