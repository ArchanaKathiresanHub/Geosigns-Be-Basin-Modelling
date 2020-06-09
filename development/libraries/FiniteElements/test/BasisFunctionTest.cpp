//
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//
// Unit tests for FiniteElementMethod::BasisFunction
//

#include "../src/FiniteElementTypes.h"
#include "../src/BasisFunction.h"

#include <gtest/gtest.h>

TEST( BasisFunction, BasisFunctionTest )
{
   double tolerance = 1e-10;
   double stepCount = 5;
   double x;
   double y;
   double z;
   double ppx;
   double ppy;
   double ppz;
   double pmx;
   double pmy;
   double pmz;
   double h;

   FiniteElementMethod::ElementVector basis;

   FiniteElementMethod::BasisFunction  basisFunction;

   // Test basis function at different evaluation point (x,y,z)
   h = 2.0 / static_cast<double> (stepCount - 1);
   x = -1.0;
   for (int i = 1; i <= stepCount; ++i)
   {
      ppx = 0.5 * (1.0 + x);
      pmx = 0.5 * (1.0 - x);
      y = -1.0;
      for (int j = 1; j <= stepCount; ++j)
      {
         ppy = 0.5 * (1.0 + y);
         pmy = 0.5 * (1.0 - y);
         z = -1.0;
         for (int k = 1; k <= stepCount; ++k)
         {
            ppz = 0.5 * (1.0 + z);
            pmz = 0.5 * (1.0 - z);
            basisFunction(x, y, z, basis);
            EXPECT_NEAR(basis(1), pmx * pmy * pmz, tolerance);
            EXPECT_NEAR(basis(2), ppx * pmy * pmz, tolerance);
            EXPECT_NEAR(basis(3), ppx * ppy * pmz, tolerance);
            EXPECT_NEAR(basis(4), pmx * ppy * pmz, tolerance);
	    
            EXPECT_NEAR(basis(5), pmx * pmy * ppz, tolerance);
            EXPECT_NEAR(basis(6), ppx * pmy * ppz, tolerance);
            EXPECT_NEAR(basis(7), ppx * ppy * ppz, tolerance);
            EXPECT_NEAR(basis(8), pmx * ppy * ppz, tolerance);
            z += h;
         }
         y += h;
      }
      x += h;
   }
}

TEST( BasisFunction, GradBasisFunctionTest )
{
   double tolerance = 1e-10;
   double stepCount = 5;
   double x;
   double y;
   double z;
   double ppx;
   double ppy;
   double ppz;
   double pmx;
   double pmy;
   double pmz;
   double h;

   FiniteElementMethod::GradElementVector gradBasis;

   FiniteElementMethod::BasisFunction  basisFunction;

   // Test gradBasis function at different evaluation point (x,y,z)
   h = 2.0 / static_cast<double> (stepCount - 1);
   x = -1.0;
   for (int i = 1; i <= stepCount; ++i)
   {
      ppx = 0.5 * (1.0 + x);
      pmx = 0.5 * (1.0 - x);
      y = -1.0;
      for (int j = 1; j <= stepCount; ++j)
      {
         ppy = 0.5 * (1.0 + y);
         pmy = 0.5 * (1.0 - y);
         z = -1.0;
         for (int k = 1; k <= stepCount; ++k)
         {
            ppz = 0.5 * (1.0 + z);
            pmz = 0.5 * (1.0 - z);
            basisFunction(x, y, z, gradBasis);
    
            EXPECT_NEAR(gradBasis(1, 1), -0.5 * pmy * pmz, tolerance);
            EXPECT_NEAR(gradBasis(1, 2), -0.5 * pmx * pmz, tolerance);
            EXPECT_NEAR(gradBasis(1, 3), -0.5 * pmx * pmy, tolerance);
    
            EXPECT_NEAR(gradBasis(2, 1), 0.5 * pmy * pmz, tolerance);
            EXPECT_NEAR(gradBasis(2, 2), -0.5 * ppx * pmz, tolerance);
            EXPECT_NEAR(gradBasis(2, 3), -0.5 * ppx * pmy, tolerance);

            EXPECT_NEAR(gradBasis(3, 1), 0.5 * ppy * pmz, tolerance);
            EXPECT_NEAR(gradBasis(3, 2), 0.5 * ppx * pmz, tolerance);
            EXPECT_NEAR(gradBasis(3, 3), -0.5 * ppx * ppy, tolerance);
    
            EXPECT_NEAR(gradBasis(4, 1), -0.5 * ppy * pmz, tolerance);
            EXPECT_NEAR(gradBasis(4, 2), 0.5 * pmx * pmz, tolerance);
            EXPECT_NEAR(gradBasis(4, 3), -0.5 * pmx * ppy, tolerance);
    
            EXPECT_NEAR(gradBasis(5, 1), -0.5 * pmy * ppz, tolerance);
            EXPECT_NEAR(gradBasis(5, 2), -0.5 * pmx * ppz, tolerance);
            EXPECT_NEAR(gradBasis(5, 3), 0.5 * pmx * pmy, tolerance);
    
            EXPECT_NEAR(gradBasis(6, 1), 0.5 * pmy * ppz, tolerance);
            EXPECT_NEAR(gradBasis(6, 2), -0.5 * ppx * ppz, tolerance);
            EXPECT_NEAR(gradBasis(6, 3), 0.5 * ppx * pmy, tolerance);
    
            EXPECT_NEAR(gradBasis(7, 1), 0.5 * ppy * ppz, tolerance);
            EXPECT_NEAR(gradBasis(7, 2), 0.5 * ppx * ppz, tolerance);
            EXPECT_NEAR(gradBasis(7, 3), 0.5 * ppx * ppy, tolerance);
    
            EXPECT_NEAR(gradBasis(8, 1), -0.5 * ppy * ppz, tolerance);
            EXPECT_NEAR(gradBasis(8, 2), 0.5 * pmx * ppz, tolerance);
            EXPECT_NEAR(gradBasis(8, 3), 0.5 * pmx * ppy, tolerance);    

            z += h;
         }
         y += h;
      }
      x += h;
   }
}

TEST( BasisFunction, BasisAndGradBasisFunctionTest )
{
   double tolerance = 1e-10;
   double stepCount = 5;
   double x;
   double y;
   double z;
   double ppx;
   double ppy;
   double ppz;
   double pmx;
   double pmy;
   double pmz;
   double h;

   FiniteElementMethod::ElementVector basis;
   FiniteElementMethod::GradElementVector gradBasis;

   FiniteElementMethod::BasisFunction  basisFunction;

   // Test Basis and gradBasis function at different evaluation point (x,y,z)
   h = 2.0 / static_cast<double> (stepCount - 1);
   x = -1.0;
   for (int i = 1; i <= stepCount; ++i)
   {
      ppx = 0.5 * (1.0 + x);
      pmx = 0.5 * (1.0 - x);
      y = -1.0;
      for (int j = 1; j <= stepCount; ++j)
      {
         ppy = 0.5 * (1.0 + y);
         pmy = 0.5 * (1.0 - y);
         z = -1.0;
         for (int k = 1; k <= stepCount; ++k)
         {
            ppz = 0.5 * (1.0 + z);
            pmz = 0.5 * (1.0 - z);
            basisFunction(x, y, z, basis, gradBasis);
    
            EXPECT_NEAR(basis(1), pmx * pmy * pmz, tolerance);
            EXPECT_NEAR(basis(2), ppx * pmy * pmz, tolerance);
            EXPECT_NEAR(basis(3), ppx * ppy * pmz, tolerance);
            EXPECT_NEAR(basis(4), pmx * ppy * pmz, tolerance);
    
            EXPECT_NEAR(basis(5), pmx * pmy * ppz, tolerance);
            EXPECT_NEAR(basis(6), ppx * pmy * ppz, tolerance);
            EXPECT_NEAR(basis(7), ppx * ppy * ppz, tolerance);
            EXPECT_NEAR(basis(8), pmx * ppy * ppz, tolerance);    
    
            EXPECT_NEAR(gradBasis(1, 1), -0.5 * pmy * pmz, tolerance);
            EXPECT_NEAR(gradBasis(1, 2), -0.5 * pmx * pmz, tolerance);
            EXPECT_NEAR(gradBasis(1, 3), -0.5 * pmx * pmy, tolerance);
    
            EXPECT_NEAR(gradBasis(2, 1), 0.5 * pmy * pmz, tolerance);
            EXPECT_NEAR(gradBasis(2, 2), -0.5 * ppx * pmz, tolerance);
            EXPECT_NEAR(gradBasis(2, 3), -0.5 * ppx * pmy, tolerance);

            EXPECT_NEAR(gradBasis(3, 1), 0.5 * ppy * pmz, tolerance);
            EXPECT_NEAR(gradBasis(3, 2), 0.5 * ppx * pmz, tolerance);
            EXPECT_NEAR(gradBasis(3, 3), -0.5 * ppx * ppy, tolerance);
    
            EXPECT_NEAR(gradBasis(4, 1), -0.5 * ppy * pmz, tolerance);
            EXPECT_NEAR(gradBasis(4, 2), 0.5 * pmx * pmz, tolerance);
            EXPECT_NEAR(gradBasis(4, 3), -0.5 * pmx * ppy, tolerance);
    
            EXPECT_NEAR(gradBasis(5, 1), -0.5 * pmy * ppz, tolerance);
            EXPECT_NEAR(gradBasis(5, 2), -0.5 * pmx * ppz, tolerance);
            EXPECT_NEAR(gradBasis(5, 3), 0.5 * pmx * pmy, tolerance);
    
            EXPECT_NEAR(gradBasis(6, 1), 0.5 * pmy * ppz, tolerance);
            EXPECT_NEAR(gradBasis(6, 2), -0.5 * ppx * ppz, tolerance);
            EXPECT_NEAR(gradBasis(6, 3), 0.5 * ppx * pmy, tolerance);
    
            EXPECT_NEAR(gradBasis(7, 1), 0.5 * ppy * ppz, tolerance);
            EXPECT_NEAR(gradBasis(7, 2), 0.5 * ppx * ppz, tolerance);
            EXPECT_NEAR(gradBasis(7, 3), 0.5 * ppx * ppy, tolerance);
    
            EXPECT_NEAR(gradBasis(8, 1), -0.5 * ppy * ppz, tolerance);
            EXPECT_NEAR(gradBasis(8, 2), 0.5 * pmx * ppz, tolerance);
            EXPECT_NEAR(gradBasis(8, 3), 0.5 * pmx * ppy, tolerance);    

            z += h;
         }
         y += h;
      }
      x += h;
   }  
}