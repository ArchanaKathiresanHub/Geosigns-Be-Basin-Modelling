#include "model/vpToDTConverter.h"

#include <gtest/gtest.h>

#include "ConstantsNumerical.h"

using namespace casaWizard;

TEST(VPToDTConverterTest, TestConvert)
{
  VPToDTConverter converter;
  std::vector<double> DT = converter.convertToDT({1500, 1300, 300});

  std::vector<double> DTExpected = {1e6 / 1500, 1e6 / 1300, 1e6 / 300};

  for (unsigned int i = 0; i < DT.size(); i++)
  {
    EXPECT_DOUBLE_EQ(DT[i], DTExpected[i]);
  }
}

TEST(VPToDTConverterTest, TestConvertInvalidValues)
{
  VPToDTConverter converter;
  std::vector<double> DT = converter.convertToDT({0.0, -1, Utilities::Numerical::CauldronNoDataValue});

  std::vector<double> DTExpected = {Utilities::Numerical::CauldronNoDataValue,
                                    Utilities::Numerical::CauldronNoDataValue,
                                    Utilities::Numerical::CauldronNoDataValue};

  for (unsigned int i = 0; i < DT.size(); i++)
  {
    EXPECT_DOUBLE_EQ(DT[i], DTExpected[i]);
  }
}


//   return Utilities::Maths::SecondToMicroSecond * (1.0 / velocity);
