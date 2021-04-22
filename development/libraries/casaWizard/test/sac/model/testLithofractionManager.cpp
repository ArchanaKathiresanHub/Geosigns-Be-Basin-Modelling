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

  manager.addLithofraction("Litho1", true, false);
  manager.addLithofraction("Litho2", true, false);

  manager.setLithofractionFirstComponent(0, 1);
  manager.setLithofractionFirstMinPercentage(0, 2);
  manager.setLithofractionFirstMaxPercentage(0, 3);
  manager.setLithofractionSecondComponent(0, 2);
  manager.setLithofractionSecondMinFraction(0, 0.5);
  manager.setLithofractionSecondMaxFraction(0, 0.6);

  manager.addOptimizedLithofraction(casaWizard::sac::OptimizedLithofraction(10,0,12,13));
  manager.addOptimizedLithofraction(casaWizard::sac::OptimizedLithofraction(11,1,14,15));

  casaWizard::ScenarioWriter writer{"lithofractionManagerActual.dat"};
  manager.writeToFile(writer);
  writer.close();

  expectFileEq("lithofractionManager.dat", "lithofractionManagerActual.dat");
}

void checkVersion0Reading(const QVector<casaWizard::sac::Lithofraction>& lithofractions,
                          const QVector<casaWizard::sac::OptimizedLithofraction>& optimized)
{
  ASSERT_EQ(lithofractions.size(), 2);
  EXPECT_EQ(lithofractions[0].layerName().toStdString(), "Litho1");
  EXPECT_EQ(lithofractions[0].firstComponent(), 1);
  EXPECT_EQ(lithofractions[0].secondComponent(), 2);
  EXPECT_DOUBLE_EQ(lithofractions[0].minPercentageFirstComponent(), 2);
  EXPECT_DOUBLE_EQ(lithofractions[0].maxPercentageFirstComponent(), 3);
  EXPECT_DOUBLE_EQ(lithofractions[0].minFractionSecondComponent(), 0.5);
  EXPECT_DOUBLE_EQ(lithofractions[0].maxFractionSecondComponent(), 0.6);
  EXPECT_EQ(lithofractions[1].layerName().toStdString(), "Litho2");

  ASSERT_EQ(optimized.size(), 2);
  EXPECT_EQ(optimized[0].wellId(), 10);
  EXPECT_EQ(optimized[0].lithofractionId(), 0);
  EXPECT_DOUBLE_EQ(optimized[0].optimizedPercentageFirstComponent(), 12);
  EXPECT_DOUBLE_EQ(optimized[0].optimizedFractionSecondComponent(), 13);

  EXPECT_EQ(optimized[1].wellId(), 11);
  EXPECT_EQ(optimized[1].lithofractionId(), 1);
  EXPECT_DOUBLE_EQ(optimized[1].optimizedPercentageFirstComponent(), 14);
  EXPECT_DOUBLE_EQ(optimized[1].optimizedFractionSecondComponent(), 15);
}

void checkVersion1Reading(const QVector<casaWizard::sac::Lithofraction>& lithofractions )
{
  EXPECT_TRUE(lithofractions[0].doFirstOptimization());
  EXPECT_FALSE(lithofractions[0].doSecondOptimization());
  EXPECT_TRUE(lithofractions[1].doFirstOptimization());
  EXPECT_FALSE(lithofractions[1].doSecondOptimization());
}

TEST(LithofractionManagerTest, testReadFromFile)
{
  casaWizard::sac::LithofractionManager manager;
  casaWizard::ScenarioReader reader{"lithofractionManager.dat"};

  manager.readFromFile(reader);

  const QVector<casaWizard::sac::Lithofraction>& lithofractions = manager.lithofractions();
  const QVector<casaWizard::sac::OptimizedLithofraction>& optimized = manager.optimizedLithofractions();

  checkVersion0Reading(lithofractions, optimized);
  checkVersion1Reading(lithofractions);
}

TEST(LithofractionManagerTest, testReadFromFileVersion0)
{
  casaWizard::sac::LithofractionManager manager;
  casaWizard::ScenarioReader reader{"lithofractionManagerVersion0.dat"};

  manager.readFromFile(reader);

  const QVector<casaWizard::sac::Lithofraction>& lithofractions = manager.lithofractions();
  const QVector<casaWizard::sac::OptimizedLithofraction>& optimized = manager.optimizedLithofractions();

  checkVersion0Reading(lithofractions, optimized);
}
