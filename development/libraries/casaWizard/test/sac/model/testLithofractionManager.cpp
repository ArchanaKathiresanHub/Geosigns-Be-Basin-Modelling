#include "model/lithofractionManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>

TEST(LithofractionManagerTest, testOptimizedInWell)
{
  casaWizard::sac::LithofractionManager manager;

  casaWizard::sac::OptimizedLithofraction optim1{1, 1, 0, 0};
  casaWizard::sac::OptimizedLithofraction optim2{2, 2, 0, 0};
  casaWizard::sac::OptimizedLithofraction optim3{1, 3, 0, 0};
  manager.addOptimizedLithofraction(optim1);
  manager.addOptimizedLithofraction(optim2);
  manager.addOptimizedLithofraction(optim3);

  const QVector<casaWizard::sac::OptimizedLithofraction> inWell = manager.optimizedInWell(1);

  ASSERT_EQ(inWell.size(), 2);
  EXPECT_EQ(inWell[0].lithofractionId(), 1);
  EXPECT_EQ(inWell[1].lithofractionId(), 3);
}

TEST(LithofractionManagerTest, testWriteToFile)
{
  casaWizard::sac::LithofractionManager manager;

  manager.addLithofraction("Litho1");
  manager.addLithofraction("Litho2");

  manager.setLithofractionFirstComponent(0, 1);
  manager.setLithofractionFirstMinPercentage(0, 2);
  manager.setLithofractionFirstMaxPercentage(0, 3);
  manager.setLithofractionSecondComponent(0, 2);
  manager.setLithofractionSecondMinFraction(0, 0.5);
  manager.setLithofractionSecondMaxFraction(0, 0.6);

  casaWizard::sac::OptimizedLithofraction optim(10, 11, 12, 13);
  manager.addOptimizedLithofraction(optim);

  casaWizard::ScenarioWriter writer{"lithofractionManagerActual.dat"};
  manager.writeToFile(writer);
  writer.close();

  expectFileEq("lithofractionManager.dat", "lithofractionManagerActual.dat");
}

TEST(LithofractionManagerTest, testReadFromFile)
{
  casaWizard::sac::LithofractionManager manager;
  casaWizard::ScenarioReader reader{"lithofractionManager.dat"};

  manager.readFromFile(reader);

  const QVector<casaWizard::sac::Lithofraction>& lithofractions = manager.lithofractions();

  ASSERT_EQ(lithofractions.size(), 2);
  EXPECT_EQ(lithofractions[0].layerName().toStdString(), "Litho1");
  EXPECT_EQ(lithofractions[0].firstComponent(), 1);
  EXPECT_EQ(lithofractions[0].secondComponent(), 2);
  EXPECT_DOUBLE_EQ(lithofractions[0].minPercentageFirstComponent(), 2);
  EXPECT_DOUBLE_EQ(lithofractions[0].maxPercentageFirstComponent(), 3);
  EXPECT_DOUBLE_EQ(lithofractions[0].minFractionSecondComponent(), 0.5);
  EXPECT_DOUBLE_EQ(lithofractions[0].maxFractionSecondComponent(), 0.6);
  EXPECT_EQ(lithofractions[1].layerName().toStdString(), "Litho2");

  const QVector<casaWizard::sac::OptimizedLithofraction>& optimized = manager.optimizedLithofractions();

  ASSERT_EQ(optimized.size(), 1);
  EXPECT_EQ(optimized[0].wellId(), 10);
  EXPECT_EQ(optimized[0].lithofractionId(), 11);
  EXPECT_DOUBLE_EQ(optimized[0].optimizedPercentageFirstComponent(), 12);
  EXPECT_DOUBLE_EQ(optimized[0].optimizedFractionSecondComponent(), 13);
}
