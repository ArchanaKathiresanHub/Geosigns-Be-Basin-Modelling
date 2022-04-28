#include "model/SurfaceToDepthConverter.h"
#include "model/input/cmbMapReader.h"
#include "model/input/cmbProjectReader.h"

#include "ConstantsNumerical.h"

#include <gtest/gtest.h>

using namespace casaWizard;

TEST(SurfaceToDepthConverterTest, testSurfaceToDepthConverter)
{
   const QString project3dPath{"./surfaceToDepthConverterTestInputs/Project.project3d"};

   CMBProjectReader projectReader;
   projectReader.load(project3dPath);

   CMBMapReader mapReader;
   mapReader.load(project3dPath.toStdString());

   SurfaceToDepthConverter converter(projectReader,mapReader);

   EXPECT_EQ(converter.getDepth(0,0,"invalidSurface"),Utilities::Numerical::IbsNoDataValue);

   //Invalid location
   EXPECT_EQ(converter.getDepth(-10e6,-10e6,"ResTop"),Utilities::Numerical::IbsNoDataValue);

   //Read from map
   double z3 = converter.getDepth(0,0,"ResTop");
   EXPECT_EQ(z3,3000);

   //Read depth value in stratIOtbl.
   EXPECT_EQ(converter.getDepth(0,0,"SRTop"),6000);

   //Read from layer with thickness
   double z5 = converter.getDepth(0,0,"SealTop");
   EXPECT_EQ(z3,z5+20);
}
