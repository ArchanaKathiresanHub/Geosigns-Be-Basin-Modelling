//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>

// SerialDataAccess
#include "Interface/SerialGrid.h"
#include "Interface/SerialGridMap.h"

// DataAccess
#include "Interface/ObjectFactory.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"

// Datamodel
#include "AbstractSnapshot.h"
#include "MockValidator.h"
#include "MockSnapshot.h"

// CBMGenerics
#include "Polyfunction.h"

// utility
#include "ConstantsNumerical.h"

// prograde
#include "LegacyBasaltThicknessCalculator.h"

//std
#include <map>
#include <utility>
#include <memory>

using namespace DataAccess::Interface;
using namespace DataModel;

typedef DataAccess::Interface::Local2DArray <CBMGenerics::Polyfunction> PolyFunction2DArray;

typedef std::pair<const std::shared_ptr<const DataModel::AbstractSnapshot>,
                  const std::shared_ptr<const DataAccess::Interface::GridMap>> SmartAbstractSnapshotSmartGridMapPair;

typedef std::map<const std::shared_ptr<const DataModel::AbstractSnapshot>,
                 const std::shared_ptr<const DataAccess::Interface::GridMap>,
                 DataModel::AbstractSnapshot::ComparePointers<const std::shared_ptr<const DataModel::AbstractSnapshot>>> SmartAbstractSnapshotVsSmartGridMap;

typedef std::map<const DataModel::AbstractSnapshot* const,
                 const DataAccess::Interface::GridMap* const,
                 DataModel::AbstractSnapshot::ComparePointers<const DataModel::AbstractSnapshot* const>> AbstractSnapshotVsGridMap;

constexpr double minI = 0.0;
constexpr double minJ = 0.0;
constexpr double maxI = 4.0;
constexpr double maxJ = 4.0;
constexpr unsigned int numI = 3;
constexpr unsigned int numJ = 3;
const SerialGrid grid( minI, minJ, maxI, maxJ, numI, numJ );

const std::map<const double, const double> ageVsContCrustThickness{
   { 10,    11000 },
   { 145,   20000 },
   { 332.9, 34000 },
};

constexpr double ages[3] = { 10 , 145 , 332.9 };

/// @brief Clean memory of the given map
void deleteSnapshotVsGridMap(AbstractSnapshotVsGridMap& toDelete) {
   std::for_each(toDelete.begin(), toDelete.end(), [] (std::pair<const DataModel::AbstractSnapshot* const,const GridMap* const>& item) {
      delete item.first;
      delete item.second;
   });
}

//-----------------------------------------------------------------------

TEST( LegacyBasaltThicknessCalculator, constructorIsGivenInvalidInput_ExpectException )
{
   ///////////////////////////////
   ///1. Prepare
   // Define grids and grid-maps
   const SerialGridMap presentDayBasaltThickness( nullptr, 0, &grid, 0    );
   const SerialGridMap crustMeltOnsetMap        ( nullptr, 0, &grid, 5000 );

   // Define inital crust and mantle thicknesses
   constexpr double initialCrustalThickness = 35000;

   // Define valid nodes (all invalid)
   MockValidator validator;
   validator.setIsValid(false);

   // Define object factory
   auto factory = std::shared_ptr<ObjectFactory>(new ObjectFactory());

   // Define crust thickness history and its polyfunction
   PolyFunction2DArray crustThicknessHistory;
   crustThicknessHistory.reallocate(&grid);
   AbstractSnapshotVsGridMap crustThicknesses;
   std::for_each(ageVsContCrustThickness.begin(), ageVsContCrustThickness.end(), [&crustThicknessHistory, &crustThicknesses] (const std::pair<const double,const double>& item) {
      const double age = item.first;
      const double thickness = item.second;
      for (auto i = grid.firstI(); i<=grid.lastI(); i++){
         for (auto j = grid.firstJ(); j<=grid.lastJ(); j++){
            crustThicknessHistory(i, j).AddPoint(age, thickness);
         }
      }
      const GridMap* gridMap = new SerialGridMap( nullptr, 0, &grid, thickness );
      crustThicknesses.insert( std::pair<const AbstractSnapshot*,const GridMap*>(new MockSnapshot(item.first), gridMap) );
   });

   ///////////////////////////////
   ///2. Act
   const AbstractSnapshotVsGridMap emptyCrustThicknessHistory;
   // Define expected exception
   const std::string expectedOutput = "No continental crustal thickness history was provided to the effective crustal thickness calculator (empty data vector)";
   //test if the good exception is thrown
   try{
      const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( emptyCrustThicknessHistory,
                                                                                       crustThicknessHistory,
                                                                                       &presentDayBasaltThickness,
                                                                                       &crustMeltOnsetMap,
                                                                                       initialCrustalThickness,
                                                                                       validator,
                                                                                       &grid,
                                                                                       factory.get() );
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }

   AbstractSnapshotVsGridMap negativeAgeCrustThicknessHistory;
   negativeAgeCrustThicknessHistory.insert(
      std::pair<const AbstractSnapshot*,const GridMap*>(new MockSnapshot(-70), new SerialGridMap( nullptr, 0, &grid, 10 )) );
   const std::string expectedOutput0 = "At least one snapshot of the crustal thickness history has a negative age";
   try{
      const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( negativeAgeCrustThicknessHistory,
                                                                                       crustThicknessHistory,
                                                                                       &presentDayBasaltThickness,
                                                                                       &crustMeltOnsetMap,
                                                                                       initialCrustalThickness,
                                                                                       validator,
                                                                                       &grid,
                                                                                       factory.get() );
      FAIL() << "Expected '" + expectedOutput0 + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput0, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput0 + "' exception";
   }
   deleteSnapshotVsGridMap(negativeAgeCrustThicknessHistory);

   AbstractSnapshotVsGridMap invalidAgeCrustThicknessHistory;
   invalidAgeCrustThicknessHistory.insert(
      std::pair<const AbstractSnapshot*,const GridMap*>(nullptr, new SerialGridMap( nullptr, 0, &grid, 10 )) );
   const std::string expectedOutput1 = "No snapshot was provided to the effective crustal thickness calculator (null pointer)";
   try{
      const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( invalidAgeCrustThicknessHistory,
                                                                                       crustThicknessHistory,
                                                                                       &presentDayBasaltThickness,
                                                                                       &crustMeltOnsetMap,
                                                                                       initialCrustalThickness,
                                                                                       validator,
                                                                                       &grid,
                                                                                       factory.get() );
      FAIL() << "Expected '" + expectedOutput1 + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput1, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput1 + "' exception";
   }
   deleteSnapshotVsGridMap(invalidAgeCrustThicknessHistory);

   AbstractSnapshotVsGridMap invalidMapCrustThicknessHistory;
   invalidMapCrustThicknessHistory.insert( std::pair<const AbstractSnapshot*,const GridMap*>(new MockSnapshot(80), nullptr) );
   const std::string expectedOutput2 = "No continental crustal thickness history was provided to the effective crustal thickness calculator (null pointer)";
   try{
      const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( invalidMapCrustThicknessHistory,
                                                                                       crustThicknessHistory,
                                                                                       &presentDayBasaltThickness,
                                                                                       &crustMeltOnsetMap,
                                                                                       initialCrustalThickness,
                                                                                       validator,
                                                                                       &grid,
                                                                                       factory.get() );
      FAIL() << "Expected '" + expectedOutput2 + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput2, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput2 + "' exception";
   }
   deleteSnapshotVsGridMap(invalidMapCrustThicknessHistory);

   const std::string expectedOutput3 = "The initial crust thickness is negative";
   try{
      const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( crustThicknesses,
                                                                                       crustThicknessHistory,
                                                                                       &presentDayBasaltThickness,
                                                                                       &crustMeltOnsetMap,
                                                                                       -90,
                                                                                       validator,
                                                                                       &grid,
                                                                                       factory.get() );
      FAIL() << "Expected '" + expectedOutput3 + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput3, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput3 + "' exception";
   }

   const std::string expectedOutput4 = "No present day basalt thickness history was provided to the effective crustal thickness calculator (null pointer)";
   try{
      const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( crustThicknesses,
                                                                                       crustThicknessHistory,
                                                                                       nullptr,
                                                                                       &crustMeltOnsetMap,
                                                                                       initialCrustalThickness,
                                                                                       validator,
                                                                                       &grid,
                                                                                       factory.get() );
      FAIL() << "Expected '" + expectedOutput4 + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput4, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput4 + "' exception";
   }

   const std::string expectedOutput5 = "No crustal thickness at melt onset was provided to the effective crustal thickness calculator (null pointer)";
   try{
      const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( crustThicknesses,
                                                                                       crustThicknessHistory,
                                                                                       &presentDayBasaltThickness,
                                                                                       nullptr,
                                                                                       initialCrustalThickness,
                                                                                       validator,
                                                                                       &grid,
                                                                                       factory.get() );
      FAIL() << "Expected '" + expectedOutput5 + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput5, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput5 + "' exception";
   }

   const std::string expectedOutput6 = "No reference grid was provided to the effective crustal thickness calculator (null pointer)";
   try{
      const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( crustThicknesses,
                                                                                       crustThicknessHistory,
                                                                                       &presentDayBasaltThickness,
                                                                                       &crustMeltOnsetMap,
                                                                                       initialCrustalThickness,
                                                                                       validator,
                                                                                       nullptr,
                                                                                       factory.get() );
      FAIL() << "Expected '" + expectedOutput6 + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput6, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput6 + "' exception";
   }

   const std::string expectedOutput7 = "No object factory was provided to the effective crustal thickness calculator (null pointer)";
   try{
      const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( crustThicknesses,
                                                                                       crustThicknessHistory,
                                                                                       &presentDayBasaltThickness,
                                                                                       &crustMeltOnsetMap,
                                                                                       initialCrustalThickness,
                                                                                       validator,
                                                                                       &grid,
                                                                                       nullptr );
      FAIL() << "Expected '" + expectedOutput7 + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput7, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput7 + "' exception";
   }

   deleteSnapshotVsGridMap(crustThicknesses);
}

//-----------------------------------------------------------------------

TEST( LegacyBasaltThicknessCalculator, invalidNodes_expectNDV )
{
   ///////////////////////////////
   ///1. Prepare
   // Define grids and grid-maps
   const SerialGridMap presentDayBasaltThickness( nullptr, 0, &grid, 0    );
   const SerialGridMap crustMeltOnsetMap        ( nullptr, 0, &grid, 5000 );

   // Define inital crust and mantle thicknesses
   constexpr double initialCrustalThickness = 35000;

   // Define valid nodes (all invalid)
   MockValidator validator;
   validator.setIsValid(false);

   // Define object factory
   auto factory = std::shared_ptr<ObjectFactory>(new ObjectFactory());

   // Define crust thickness history and its polyfunction
   PolyFunction2DArray crustThicknessHistory;
   crustThicknessHistory.reallocate(&grid);
   AbstractSnapshotVsGridMap crustThicknesses;
   std::for_each(ageVsContCrustThickness.begin(), ageVsContCrustThickness.end(), [&crustThicknessHistory, &crustThicknesses] (const std::pair<const double,const double>& item) {
      const double age = item.first;
      const double thickness = item.second;
      for (auto i = grid.firstI(); i<=grid.lastI(); i++){
         for (auto j = grid.firstJ(); j<=grid.lastJ(); j++){
            crustThicknessHistory(i, j).AddPoint(age, thickness);
         }
      }
      const GridMap* gridMap = new SerialGridMap( nullptr, 0, &grid, thickness );
      crustThicknesses.insert( std::pair<const AbstractSnapshot*,const GridMap*>(new MockSnapshot(age), gridMap) );
   });

   ///////////////////////////////
   ///2. Act
   // Compute the legacy basalt thicknesses
   const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( crustThicknesses,
                                                                                    crustThicknessHistory,
                                                                                    &presentDayBasaltThickness,
                                                                                    &crustMeltOnsetMap,
                                                                                    initialCrustalThickness,
                                                                                    validator,
                                                                                    &grid,
                                                                                    factory.get() );
   SmartAbstractSnapshotVsSmartGridMap result;
   legacyBasaltThicknessCalculator.compute(result);

   ///////////////////////////////
   ///3. Assert
   unsigned int count = 0;
   // expect three history data (as in continental crustal thickness input)
   EXPECT_EQ(3, result.size());
   std::for_each(result.begin(), result.end(), [&count] (const SmartAbstractSnapshotSmartGridMapPair& item) {
      const auto snapshot = item.first;
      const auto gridMap = item.second;
      // expect grid map and snapshot to be defined
      EXPECT_TRUE(snapshot != nullptr);
      EXPECT_TRUE(gridMap  != nullptr);
      // expect age to be the same as in the continental crust thickness history
      EXPECT_DOUBLE_EQ(ages[count], snapshot->getTime());
      gridMap->retrieveData(true);
      for (auto i = static_cast<unsigned int>(grid.firstI()); i <= static_cast<unsigned int>(grid.lastI()); i++) {
         for (auto j = static_cast<unsigned int>(grid.firstJ()); j <= static_cast<unsigned int>(grid.lastJ()); j++) {
            // expect all nodes unvalid
            EXPECT_DOUBLE_EQ(Utilities::Numerical::CauldronNoDataValue, gridMap->getValue(i,j));
         }
      }
      gridMap->restoreData(true);
      count++;
   });

   ///////////////////////////////
   ///4. Clean
   deleteSnapshotVsGridMap(crustThicknesses);
}

//-----------------------------------------------------------------------

TEST( LegacyBasaltThicknessCalculator, noPresentDayBasalt_ExpectNoBasalt )
{
   ///////////////////////////////
   ///1. Prepare
   // Define grids and grid-maps
   const SerialGridMap presentDayBasaltThickness( nullptr, 0, &grid, 0    );
   const SerialGridMap crustMeltOnsetMap        ( nullptr, 0, &grid, 5000 );

   // Define inital crust and mantle thicknesses
   constexpr double initialCrustalThickness = 35000;

   // Define valid nodes (all valid)
   const MockValidator validator;

   // Define object factory
   auto factory = std::shared_ptr<ObjectFactory>(new ObjectFactory());

   // Define crust thickness history and its polyfunction
   PolyFunction2DArray crustThicknessHistory;
   crustThicknessHistory.reallocate(&grid);
   AbstractSnapshotVsGridMap crustThicknesses;
   std::for_each(ageVsContCrustThickness.begin(), ageVsContCrustThickness.end(), [&crustThicknessHistory, &crustThicknesses] (const std::pair<const double,const double>& item) {
      const double age = item.first;
      const double thickness = item.second;
      for (auto i = grid.firstI(); i<=grid.lastI(); i++){
         for (auto j = grid.firstJ(); j<=grid.lastJ(); j++){
            crustThicknessHistory(i, j).AddPoint(age, thickness);
         }
      }
      const GridMap* gridMap = new SerialGridMap( nullptr, 0, &grid, thickness );
      crustThicknesses.insert( std::pair<const AbstractSnapshot*,const GridMap*>( new MockSnapshot(age), gridMap) );
   });

   ///////////////////////////////
   ///2. Act
   // Compute the legacy basalt thicknesses
   const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( crustThicknesses,
                                                                                    crustThicknessHistory,
                                                                                    &presentDayBasaltThickness,
                                                                                    &crustMeltOnsetMap,
                                                                                    initialCrustalThickness,
                                                                                    validator,
                                                                                    &grid,
                                                                                    factory.get() );
   SmartAbstractSnapshotVsSmartGridMap result;
   legacyBasaltThicknessCalculator.compute(result);

   ///////////////////////////////
   ///3. Assert
   unsigned int count = 0;
   // expect three history data (as in continental crustal thickness input)
   EXPECT_EQ(3, result.size());
   std::for_each(result.begin(), result.end(), [&count] (const SmartAbstractSnapshotSmartGridMapPair& item) {
      const auto snapshot = item.first;
      const auto gridMap = item.second;
      // expect grid map and snapshot to be defined
      EXPECT_TRUE(snapshot != nullptr);
      EXPECT_TRUE(gridMap  != nullptr);
      // expect age to be the same as in the continental crust thickness history
      EXPECT_DOUBLE_EQ(ages[count], snapshot->getTime());
      gridMap->retrieveData(true);
      for (auto i = static_cast<unsigned int>(grid.firstI()); i <= static_cast<unsigned int>(grid.lastI()); i++) {
         for (auto j = static_cast<unsigned int>(grid.firstJ()); j <= static_cast<unsigned int>(grid.lastJ()); j++) {
            // expect no basalt
            EXPECT_DOUBLE_EQ(0, gridMap->getValue(i,j));
         }
      }
      gridMap->restoreData(true);
      count++;
   });

   ///////////////////////////////
   ///4. Clean
   deleteSnapshotVsGridMap(crustThicknesses);
}

//-----------------------------------------------------------------------

TEST( LegacyBasaltThicknessCalculator, onsetIsReached_ExpectBasalt_case1 )
{
   ///////////////////////////////
   ///1. Prepare
   // Define grids and grid-maps
   const SerialGridMap presentDayBasaltThickness( nullptr, 0, &grid, 8000  );
   const SerialGridMap crustMeltOnsetMap        ( nullptr, 0, &grid, 20000 );

   // Define inital crust and mantle thicknesses
   constexpr double initialCrustalThickness = 35000;

   // Define valid nodes (all valid)
   const MockValidator validator;

   // Define object factory
   auto factory = std::shared_ptr<ObjectFactory>(new ObjectFactory());

   // Define crust thickness history and its polyfunction
   PolyFunction2DArray crustThicknessHistory;
   crustThicknessHistory.reallocate(&grid);
   AbstractSnapshotVsGridMap crustThicknesses;
   std::for_each(ageVsContCrustThickness.begin(), ageVsContCrustThickness.end(), [&crustThicknessHistory, &crustThicknesses] (const std::pair<const double,const double>& item) {
      const double age = item.first;
      const double thickness = item.second;
      for (auto i = grid.firstI(); i<=grid.lastI(); i++){
         for (auto j = grid.firstJ(); j<=grid.lastJ(); j++){
            crustThicknessHistory(i, j).AddPoint(age, thickness);
         }
      }
      const GridMap* gridMap = new SerialGridMap( nullptr, 0, &grid, thickness );
      crustThicknesses.insert( std::pair<const AbstractSnapshot*,const GridMap*>( new MockSnapshot(age), gridMap) );
   });

   //Define expected basalt results
   const std::map<const double,const double> ageVsBasaltThickness {
      {10,    8000},
      {145,   0},
      {332.9, 0},
   };

   ///////////////////////////////
   ///2. Act
   // Compute the legacy basalt thicknesses
   const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( crustThicknesses,
                                                                                    crustThicknessHistory,
                                                                                    &presentDayBasaltThickness,
                                                                                    &crustMeltOnsetMap,
                                                                                    initialCrustalThickness,
                                                                                    validator,
                                                                                    &grid,
                                                                                    factory.get() );
   SmartAbstractSnapshotVsSmartGridMap result;
   legacyBasaltThicknessCalculator.compute(result);

   ///////////////////////////////
   ///3. Assert
   unsigned int count = 0;
   // expect three history data (as in continental crustal thickness input)
   EXPECT_EQ(3, result.size());
   std::for_each(result.begin(), result.end(), [&count, &ageVsBasaltThickness] (const SmartAbstractSnapshotSmartGridMapPair& item) {
      const auto snapshot = item.first;
      const auto gridMap = item.second;
      // expect grid map and snapshot to be defined
      EXPECT_TRUE(snapshot != nullptr);
      EXPECT_TRUE(gridMap  != nullptr);
      // expect age to be the same as in the continental crust thickness history
      EXPECT_DOUBLE_EQ(ages[count], snapshot->getTime());
      gridMap->retrieveData(true);
      for (auto i = static_cast<unsigned int>(grid.firstI()); i <= static_cast<unsigned int>(grid.lastI()); i++) {
         for (auto j = static_cast<unsigned int>(grid.firstJ()); j <= static_cast<unsigned int>(grid.lastJ()); j++) {
            // expect no basalt
            EXPECT_DOUBLE_EQ(ageVsBasaltThickness.at(snapshot->getTime()), gridMap->getValue(i,j));
         }
      }
      gridMap->restoreData(true);
      count++;
   });

   ///////////////////////////////
   ///4. Clean
   deleteSnapshotVsGridMap(crustThicknesses);
}

//-----------------------------------------------------------------------

TEST( LegacyBasaltThicknessCalculator, onsetIsReached_ExpectBasalt_case2 )
{
   ///////////////////////////////
   ///1. Prepare
   // Define grids and grid-maps
   const SerialGridMap presentDayBasaltThickness( nullptr, 0, &grid, 8000  );
   const SerialGridMap crustMeltOnsetMap        ( nullptr, 0, &grid, 30000 );

   // Define inital crust and mantle thicknesses
   constexpr double initialCrustalThickness = 35000;

   // Define valid nodes (all valid)
   const MockValidator validator;

   // Define object factory
   auto factory = std::shared_ptr<ObjectFactory>(new ObjectFactory());

   // Define crust thickness history and its polyfunction
   PolyFunction2DArray crustThicknessHistory;
   crustThicknessHistory.reallocate(&grid);
   AbstractSnapshotVsGridMap crustThicknesses;
   std::for_each(ageVsContCrustThickness.begin(), ageVsContCrustThickness.end(), [&crustThicknessHistory, &crustThicknesses] (const std::pair<const double,const double>& item) {
      const double age = item.first;
      const double thickness = item.second;
      for (auto i = grid.firstI(); i<=grid.lastI(); i++){
         for (auto j = grid.firstJ(); j<=grid.lastJ(); j++){
            crustThicknessHistory(i, j).AddPoint(age, thickness);
         }
      }
      const GridMap* gridMap = new SerialGridMap( nullptr, 0, &grid, thickness );
      crustThicknesses.insert( std::pair<const AbstractSnapshot*,const GridMap*>( new MockSnapshot(age), gridMap) );
   });

   //Define expected basalt results
   const std::map<const double,const double> ageVsBasaltThickness {
      {10,    8000},
      {145,   4210.5263157894733},
      {332.9, 0},
   };

   ///////////////////////////////
   ///2. Act
   // Compute the legacy basalt thicknesses
   const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( crustThicknesses,
                                                                                    crustThicknessHistory,
                                                                                    &presentDayBasaltThickness,
                                                                                    &crustMeltOnsetMap,
                                                                                    initialCrustalThickness,
                                                                                    validator,
                                                                                    &grid,
                                                                                    factory.get() );
   SmartAbstractSnapshotVsSmartGridMap result;
   legacyBasaltThicknessCalculator.compute(result);

   ///////////////////////////////
   ///3. Assert
   unsigned int count = 0;
   // expect three history data (as in continental crustal thickness input)
   EXPECT_EQ(3, result.size());
   std::for_each(result.begin(), result.end(), [&count, &ageVsBasaltThickness] (const SmartAbstractSnapshotSmartGridMapPair& item) {
      const auto snapshot = item.first;
      const auto gridMap = item.second;
      // expect grid map and snapshot to be defined
      EXPECT_TRUE(snapshot != nullptr);
      EXPECT_TRUE(gridMap  != nullptr);
      // expect age to be the same as in the continental crust thickness history
      EXPECT_DOUBLE_EQ(ages[count], snapshot->getTime());
      gridMap->retrieveData(true);
      for (auto i = static_cast<unsigned int>(grid.firstI()); i <= static_cast<unsigned int>(grid.lastI()); i++) {
         for (auto j = static_cast<unsigned int>(grid.firstJ()); j <= static_cast<unsigned int>(grid.lastJ()); j++) {
            // expect no basalt
            EXPECT_DOUBLE_EQ(ageVsBasaltThickness.at(snapshot->getTime()), gridMap->getValue(i,j));
         }
      }
      gridMap->restoreData(true);
      count++;
   });

   ///////////////////////////////
   ///4. Clean
   deleteSnapshotVsGridMap(crustThicknesses);
}

//-----------------------------------------------------------------------

TEST( LegacyBasaltThicknessCalculator, onsetIsReached_AndContCrustRebond_ExpectBasalt )
{
   ///////////////////////////////
   ///1. Prepare
   // Define grids and grid-maps
   const SerialGridMap presentDayBasaltThickness( nullptr, 0, &grid, 8000  );
   const SerialGridMap crustMeltOnsetMap        ( nullptr, 0, &grid, 30000 );

   // Define inital crust and mantle thicknesses
   constexpr double initialCrustalThickness = 35000;

   // Define valid nodes (all valid)
   const MockValidator validator;

   // Define object factory
   auto factory = std::shared_ptr<ObjectFactory>(new ObjectFactory());

   // Define crust thickness history and its polyfunction
   const std::map<const double,const double> ageVsContCrustThicknessRebound {
      {10,    22000},
      {145,   20000},
      {332.9, 34000},
   };
   PolyFunction2DArray crustThicknessHistory;
   crustThicknessHistory.reallocate(&grid);
   AbstractSnapshotVsGridMap crustThicknesses;
   std::for_each(ageVsContCrustThicknessRebound.begin(), ageVsContCrustThicknessRebound.end(), [&crustThicknessHistory, &crustThicknesses] (const std::pair<const double,const double>& item) {
      const double age = item.first;
      const double thickness = item.second;
      for (auto i = grid.firstI(); i<=grid.lastI(); i++){
         for (auto j = grid.firstJ(); j<=grid.lastJ(); j++){
            crustThicknessHistory(i, j).AddPoint(age, thickness);
         }
      }
      const GridMap* gridMap = new SerialGridMap( nullptr, 0, &grid, thickness );
      crustThicknesses.insert( std::pair<const AbstractSnapshot*,const GridMap*>( new MockSnapshot(age), gridMap) );
   });

   //Define expected basalt results
   const std::map<const double,const double> ageVsBasaltThickness {
      {10,    10000},
      {145,   10000},
      {332.9, 0},
   };

   ///////////////////////////////
   ///2. Act
   // Compute the legacy basalt thicknesses
   const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( crustThicknesses,
                                                                                    crustThicknessHistory,
                                                                                    &presentDayBasaltThickness,
                                                                                    &crustMeltOnsetMap,
                                                                                    initialCrustalThickness,
                                                                                    validator,
                                                                                    &grid,
                                                                                    factory.get() );
   SmartAbstractSnapshotVsSmartGridMap result;
   legacyBasaltThicknessCalculator.compute(result);

   ///////////////////////////////
   ///3. Assert
   unsigned int count = 0;
   // expect three history data (as in continental crustal thickness input)
   EXPECT_EQ(3, result.size());
   std::for_each(result.begin(), result.end(), [&count, &ageVsBasaltThickness] (const SmartAbstractSnapshotSmartGridMapPair& item) {
      const auto snapshot = item.first;
      const auto gridMap = item.second;
      // expect grid map and snapshot to be defined
      EXPECT_TRUE(snapshot != nullptr);
      EXPECT_TRUE(gridMap  != nullptr);
      // expect age to be the same as in the continental crust thickness history
      EXPECT_DOUBLE_EQ(ages[count], snapshot->getTime());
      gridMap->retrieveData(true);
      for (auto i = static_cast<unsigned int>(grid.firstI()); i <= static_cast<unsigned int>(grid.lastI()); i++) {
         for (auto j = static_cast<unsigned int>(grid.firstJ()); j <= static_cast<unsigned int>(grid.lastJ()); j++) {
            // expect no basalt
            EXPECT_DOUBLE_EQ(ageVsBasaltThickness.at(snapshot->getTime()), gridMap->getValue(i,j));
         }
      }
      gridMap->restoreData(true);
      count++;
   });

   ///////////////////////////////
   ///4. Clean
   deleteSnapshotVsGridMap(crustThicknesses);
}

//-----------------------------------------------------------------------

TEST( LegacyBasaltThicknessCalculator, onsetIsReached_AndEqualToPresentDayContCrustThickness_ExpectNoBasalt )
{
   ///////////////////////////////
   ///1. Prepare
   // Define grids and grid-maps
   const SerialGridMap presentDayBasaltThickness( nullptr, 0, &grid, 8000  );
   const SerialGridMap crustMeltOnsetMap        ( nullptr, 0, &grid, 22000 );

   // Define inital crust and mantle thicknesses
   constexpr double initialCrustalThickness = 35000;

   // Define valid nodes (all valid)
   const MockValidator validator;

   // Define object factory
   auto factory = std::shared_ptr<ObjectFactory>(new ObjectFactory());

   // Define crust thickness history and its polyfunction
   const std::map<const double,const double> ageVsContCrustThicknessRebound {
      {10,    22000},
      {145,   20000},
      {332.9, 34000},
   };
   PolyFunction2DArray crustThicknessHistory;
   crustThicknessHistory.reallocate(&grid);
   AbstractSnapshotVsGridMap crustThicknesses;
   std::for_each(ageVsContCrustThicknessRebound.begin(), ageVsContCrustThicknessRebound.end(), [&crustThicknessHistory, &crustThicknesses] (const std::pair<const double,const double>& item) {
      const double age = item.first;
      const double thickness = item.second;
      for (auto i = grid.firstI(); i<=grid.lastI(); i++){
         for (auto j = grid.firstJ(); j<=grid.lastJ(); j++){
            crustThicknessHistory(i, j).AddPoint(age, thickness);
         }
      }
      const GridMap* gridMap = new SerialGridMap( nullptr, 0, &grid, thickness );
      crustThicknesses.insert( std::pair<const AbstractSnapshot*,const GridMap*>( new MockSnapshot(age), gridMap) );
   });

   ///////////////////////////////
   ///2. Act
   // Compute the legacy basalt thicknesses
   const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( crustThicknesses,
                                                                                    crustThicknessHistory,
                                                                                    &presentDayBasaltThickness,
                                                                                    &crustMeltOnsetMap,
                                                                                    initialCrustalThickness,
                                                                                    validator,
                                                                                    &grid,
                                                                                    factory.get() );
   SmartAbstractSnapshotVsSmartGridMap result;
   legacyBasaltThicknessCalculator.compute(result);

   ///////////////////////////////
   ///3. Assert
   unsigned int count = 0;
   // expect three history data (as in continental crustal thickness input)
   EXPECT_EQ(3, result.size());
   std::for_each(result.begin(), result.end(), [&count] (const SmartAbstractSnapshotSmartGridMapPair& item) {
      const auto snapshot = item.first;
      const auto gridMap = item.second;
      // expect grid map and snapshot to be defined
      EXPECT_TRUE(snapshot != nullptr);
      EXPECT_TRUE(gridMap  != nullptr);
      // expect age to be the same as in the continental crust thickness history
      EXPECT_DOUBLE_EQ(ages[count], snapshot->getTime());
      gridMap->retrieveData(true);
      for (auto i = static_cast<unsigned int>(grid.firstI()); i <= static_cast<unsigned int>(grid.lastI()); i++) {
         for (auto j = static_cast<unsigned int>(grid.firstJ()); j <= static_cast<unsigned int>(grid.lastJ()); j++) {
            // expect no basalt
            EXPECT_DOUBLE_EQ(0, gridMap->getValue(i,j));
         }
      }
      gridMap->restoreData(true);
      count++;
   });

   ///////////////////////////////
   ///4. Clean
   deleteSnapshotVsGridMap(crustThicknesses);
}

//-----------------------------------------------------------------------

TEST( LegacyBasaltThicknessCalculator, meltOnsetIsTooLow_ExpectNoBasalt )
{
   ///////////////////////////////
   ///1. Prepare
   // Define grids and grid-maps
   const SerialGridMap presentDayBasaltThickness( nullptr, 0, &grid, 8000 );
   const SerialGridMap crustMeltOnsetMap        ( nullptr, 0, &grid, 1000 );

   // Define inital crust and mantle thicknesses
   constexpr double initialCrustalThickness = 35000;

   // Define valid nodes (all valid)
   const MockValidator validator;

   // Define object factory
   auto factory = std::shared_ptr<ObjectFactory>(new ObjectFactory());

   // Define crust thickness history and its polyfunction
   PolyFunction2DArray crustThicknessHistory;
   crustThicknessHistory.reallocate(&grid);
   AbstractSnapshotVsGridMap crustThicknesses;
   std::for_each(ageVsContCrustThickness.begin(), ageVsContCrustThickness.end(), [&crustThicknessHistory, &crustThicknesses] (const std::pair<const double,const double>& item) {
      const double age = item.first;
      const double thickness = item.second;
      for (auto i = grid.firstI(); i<=grid.lastI(); i++){
         for (auto j = grid.firstJ(); j<=grid.lastJ(); j++){
            crustThicknessHistory(i, j).AddPoint(age, thickness);
         }
      }
      const GridMap* gridMap = new SerialGridMap( nullptr, 0, &grid, thickness );
      crustThicknesses.insert( std::pair<const AbstractSnapshot*,const GridMap*>( new MockSnapshot(age), gridMap) );
   });

   ///////////////////////////////
   ///2. Act
   // Compute the legacy basalt thicknesses
   const Prograde::LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator( crustThicknesses,
                                                                                    crustThicknessHistory,
                                                                                    &presentDayBasaltThickness,
                                                                                    &crustMeltOnsetMap,
                                                                                    initialCrustalThickness,
                                                                                    validator,
                                                                                    &grid,
                                                                                    factory.get() );
   SmartAbstractSnapshotVsSmartGridMap result;
   legacyBasaltThicknessCalculator.compute(result);

   ///////////////////////////////
   ///3. Assert
   unsigned int count = 0;
   // expect three history data (as in continental crustal thickness input)
   EXPECT_EQ(3, result.size());
   std::for_each(result.begin(), result.end(), [&count] (const SmartAbstractSnapshotSmartGridMapPair& item) {
      const auto snapshot = item.first;
      const auto gridMap = item.second;
      // expect grid map and snapshot to be defined
      EXPECT_TRUE(snapshot != nullptr);
      EXPECT_TRUE(gridMap  != nullptr);
      // expect age to be the same as in the continental crust thickness history
      EXPECT_DOUBLE_EQ(ages[count], snapshot->getTime());
      gridMap->retrieveData(true);
      for (auto i = static_cast<unsigned int>(grid.firstI()); i<= static_cast<unsigned int>(grid.lastI()); i++){
         for (auto j = static_cast<unsigned int>(grid.firstJ()); j<= static_cast<unsigned int>(grid.lastJ()); j++){
            // expect no basalt
            EXPECT_DOUBLE_EQ(0, gridMap->getValue(i,j));
         }
      }
      gridMap->restoreData(true);
      count++;
   });

   ///////////////////////////////
   ///4. Clean
   deleteSnapshotVsGridMap(crustThicknesses);
}