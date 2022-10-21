//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/hdfReadManager.h"

#include "DataMiningProjectHandle.h"
#include "DataMiningObjectFactory.h"
#include "ProjectHandle.h"

#include "ConstantsNumerical.h"

#include <gtest/gtest.h>
#include <memory>

namespace
{
   void checkEqual(const DataExtraction::DoubleMatrix& mat, double value)
   {
      for (const auto& v : mat)
      {
         for (const auto& d : v)
         {
            EXPECT_DOUBLE_EQ(d, value);
         }
      }
   }
}

TEST( hdfReadManager, readFromOutputMap )
{
   std::unique_ptr<DataAccess::Mining::ObjectFactory> objectFactory
         = std::unique_ptr<DataAccess::Mining::ObjectFactory>(new DataAccess::Mining::ObjectFactory);

   std::unique_ptr<DataAccess::Interface::ProjectHandle> projectHandle =
         std::unique_ptr<DataAccess::Interface::ProjectHandle>(OpenCauldronProject( "Project.project3d", objectFactory.get()));

   DataExtraction::HDFReadManager readManager(*projectHandle);

   readManager.openSnapshotFile("Time_0.000000.h5");

   //The "/Temperature/l1" map looks like this for the upper layer:
   // 99999.0	99999.0	  99999.0
   // 77.59483	77.594894  99999.0
   // 77.59501	77.59515	  99999.0

   // with other valid values within the layer. Index origin is upper left.

   {
      DataExtraction::DoubleMatrix mat = readManager.get3dCoordinatePropertyMatrix({{ 0.5, 0.5}},"/Temperature/l1");
      checkEqual(mat,Utilities::Numerical::CauldronNoDataValue);
   }

   { //Locations just outside the valid region should return undefined values:
      DataExtraction::DoubleMatrix mat = readManager.get3dCoordinatePropertyMatrix({{ 1, 1+Utilities::Numerical::DefaultNumericalTolerance}},"/Temperature/l1");
      checkEqual(mat,Utilities::Numerical::CauldronNoDataValue);
   }

   {
   DataExtraction::DoubleMatrix mat = readManager.get3dCoordinatePropertyMatrix({{ 2, 0}},"/Temperature/l1");
   EXPECT_NEAR(mat[0][0],77.595008850097656,Utilities::Numerical::DefaultNumericalTolerance);
   EXPECT_NEAR(mat[0][1],68.2719955444336,Utilities::Numerical::DefaultNumericalTolerance);
   EXPECT_NEAR(mat[0][2],58.38811874389648,Utilities::Numerical::DefaultNumericalTolerance);
   EXPECT_NEAR(mat[0][3],47.70056533813477,Utilities::Numerical::DefaultNumericalTolerance);
   }

   //Values on the edges of the valud region should be extracted correctly:
   {
   DataExtraction::DoubleMatrix mat = readManager.get3dCoordinatePropertyMatrix({{ 2, 1}},"/Temperature/l1");
   EXPECT_NEAR(mat[0][0],77.59515380844879,Utilities::Numerical::DefaultNumericalTolerance);
   EXPECT_NEAR(mat[0][1],68.27204895014191,Utilities::Numerical::DefaultNumericalTolerance);
   EXPECT_NEAR(mat[0][2],58.38804245002746,Utilities::Numerical::DefaultNumericalTolerance);
   EXPECT_NEAR(mat[0][3],47.70033645652771,Utilities::Numerical::DefaultNumericalTolerance);
   }

   {
   DataExtraction::DoubleMatrix mat = readManager.get3dCoordinatePropertyMatrix({{ 1, 1}},"/Temperature/l1");
   EXPECT_NEAR(mat[0][0],77.59489440917969,Utilities::Numerical::DefaultNumericalTolerance);
   EXPECT_NEAR(mat[0][1],68.27183532714844,Utilities::Numerical::DefaultNumericalTolerance);
   EXPECT_NEAR(mat[0][2],58.3879280090332,Utilities::Numerical::DefaultNumericalTolerance);
   EXPECT_NEAR(mat[0][3],47.70046997070313,Utilities::Numerical::DefaultNumericalTolerance);
   }

}
