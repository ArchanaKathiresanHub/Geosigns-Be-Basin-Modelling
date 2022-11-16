#include "model/doeOptionImpl.h"
#include "model/doeOption.h"

#include <BoxBehnken.h>
#include <FactDesign.h>
#include <ParameterBounds.h>
#include <ScreenDesign.h>
#include <Tornado.h>

#include <gtest/gtest.h>
#include <array>

TEST( DoeOptionTest, TestOptions )
{
   const QVector<casaWizard::ua::DoeOption*> options = casaWizard::ua::DoeOption::getDoeOptions();
   QVector<QString> names;
   for (auto option : options)
   {
      names.push_back(option->name());
   }
   EXPECT_TRUE(names.contains("BaseCase"));
   EXPECT_TRUE(names.contains("BoxBehnken"));
   EXPECT_TRUE(names.contains("Tornado"));
   EXPECT_TRUE(names.contains("PlackettBurman"));
   EXPECT_TRUE(names.contains("PlackettBurmanMirror"));
   EXPECT_TRUE(names.contains("FullFactorial"));
   EXPECT_TRUE(names.contains("SpaceFilling"));
   EXPECT_TRUE(names.contains("UserDefined"));
}

TEST( BaseCaseTest, TestBaseCase )
{
   casaWizard::ua::DoeBaseCase baseCase;
   EXPECT_EQ(baseCase.name(),"BaseCase");
   EXPECT_TRUE(baseCase.hasCalculatedDesignPoints());
}

TEST( DoeTornadoTest, TestNCalculatedDesignPointsTornado )
{
   const int numberOfInfluentialParameters = 2;
   const int nDesignPointsExpected = 5;
   casaWizard::ua::DoeTornado tornado;
   tornado.calculateNDesignPoints(numberOfInfluentialParameters);
   const int nDesignPointsActual = tornado.nDesignPoints();
   EXPECT_EQ(nDesignPointsExpected, nDesignPointsActual);

   // Check consistency with SUMlib library
   std::vector<bool> isInfluentialParameterSelected(numberOfInfluentialParameters, true) ;
   SUMlib::Tornado tornadoSUMlib(isInfluentialParameterSelected, (std::size_t) numberOfInfluentialParameters);
   const int nDesignPointsTornadoSUMlibActual = tornadoSUMlib.getNbOfCases(SUMlib::ParameterBounds());
   EXPECT_EQ(nDesignPointsExpected, nDesignPointsTornadoSUMlibActual);
}

TEST( DoeBoxBehnkenTest, TestNCalculatedDesignPointsBoxBehnken )
{
   casaWizard::ua::DoeBoxBehnken boxBehnken;

   const std::array<int, 5> nInfluentialParameterArray = {6,  7,  9,  11,  21};
   const std::array<int, 5> nDesignPointsArrayExpected = {49, 57, 97, 177, 841};

   for (int i = 0; i < nInfluentialParameterArray.size(); ++i)
   {
      const int nDesignPointExpected = nDesignPointsArrayExpected[i];
      boxBehnken.calculateNDesignPoints(nInfluentialParameterArray[i]);
      const int nDesignPointActual = boxBehnken.nDesignPoints();
      EXPECT_EQ(nDesignPointActual, nDesignPointExpected);

      // Check consistency with SUMlib library
      std::vector<bool> isInfluentialParameterSelected(nInfluentialParameterArray[i], true) ;
      std::size_t numberOfInfluentialParameters = nInfluentialParameterArray[i];
      SUMlib::BoxBehnken boxBehnkenSUMlib(isInfluentialParameterSelected, numberOfInfluentialParameters);
      const int nDesignPointsBoxBehnkenSUMlibActual = boxBehnkenSUMlib.getNbOfCases(SUMlib::ParameterBounds());
      EXPECT_EQ(nDesignPointsBoxBehnkenSUMlibActual, nDesignPointExpected);
   }
}

TEST( DoePlackettBurmanTest, TestNCalculatedDesignPointsPlackettBurman )
{
   casaWizard::ua::DoePlackettBurman plackettBurman;
   casaWizard::ua::DoePlackettBurmanMirror plackettBurmanMirror;

   const std::array<int, 5> nInfluentialParameterArray =                     {1, 2, 4,  11, 33};
   const std::array<int, 5> nDesignPointsArrayPlackettBurmanExpected =       {2, 4, 8,  12, 36};
   const std::array<int, 5> nDesignPointsArrayPlackettBurmanMirrorExpected = {2, 4, 16, 24, 72};

   for (int i = 0; i < nInfluentialParameterArray.size(); ++i)
   {
      plackettBurman.calculateNDesignPoints(nInfluentialParameterArray[i]);
      const int nDesignPointsPlackettBurmanActual = plackettBurman.nDesignPoints();
      const int nDesignPointPlackettBurmanExpected = nDesignPointsArrayPlackettBurmanExpected[i];
      EXPECT_EQ(nDesignPointsPlackettBurmanActual, nDesignPointPlackettBurmanExpected);

      plackettBurmanMirror.calculateNDesignPoints(nInfluentialParameterArray[i]);
      const int nDesignPointsPlackettBurmanMirrorActual = plackettBurmanMirror.nDesignPoints();
      const int nDesignPointsPlackettBurmanMirrorExpected = nDesignPointsArrayPlackettBurmanMirrorExpected[i];
      EXPECT_EQ(nDesignPointsPlackettBurmanMirrorActual, nDesignPointsPlackettBurmanMirrorExpected);

      // Check consistency of PlackettBurman with SUMlib library
      std::vector<bool> isInfluentialParameterSelected(nInfluentialParameterArray[i], true) ;
      std::size_t numberOfInfluentialParameters = nInfluentialParameterArray[i];
      SUMlib::ScreenDesign plackettBurmanSUMlib(isInfluentialParameterSelected, numberOfInfluentialParameters, false, false);
      const int nDesignPointsPlackettBurmanSUMlibActual = plackettBurmanSUMlib.getNbOfCases(SUMlib::ParameterBounds());
      EXPECT_EQ(nDesignPointsPlackettBurmanSUMlibActual, nDesignPointPlackettBurmanExpected);

      // Check consistency of PlacketBurmanMirror with SUMlib library
      SUMlib::ScreenDesign plackettBurmanMirrorSUMlib(isInfluentialParameterSelected, numberOfInfluentialParameters, false, true);
      const int nDesignPointsPlackettBurmanMirrorSUMlibActual = plackettBurmanMirrorSUMlib.getNbOfCases(SUMlib::ParameterBounds());
      EXPECT_EQ(nDesignPointsPlackettBurmanMirrorSUMlibActual, nDesignPointsPlackettBurmanMirrorExpected);
   }
}

TEST( DoeFullFactorialTest, TestNCalculatedDesignPointsFullFactorial )
{
   const int numberOfInfluentialParameters = 3;
   const int nDesignPointsExpected = 9;
   casaWizard::ua::DoeFullFactorial fullFactorial;
   fullFactorial.calculateNDesignPoints(numberOfInfluentialParameters);
   const int nDesignPointsActual = fullFactorial.nDesignPoints();
   EXPECT_EQ(nDesignPointsExpected, nDesignPointsActual);

   // Check consistency with SUMlib library
   std::vector<bool> isInfluentialParameterSelected(numberOfInfluentialParameters, true) ;
   SUMlib::FactDesign fullFactorialSUMlib(isInfluentialParameterSelected, (std::size_t) numberOfInfluentialParameters);
   const int nDesignPointsFullFactorialSUMlibActual = fullFactorialSUMlib.getNbOfCases(SUMlib::ParameterBounds());
   EXPECT_EQ(nDesignPointsExpected, nDesignPointsFullFactorialSUMlibActual);
}

TEST( DoeSpaceFillingTest, TestNCalculatedDesignPointsSpaceFilling )
{
   const int numberOfInfluentialParameters = 3;
   const int nDesignPointsExpected = 0;
   casaWizard::ua::DoeSpaceFilling spaceFilling;
   spaceFilling.calculateNDesignPoints(numberOfInfluentialParameters);
   const int nDesignPointsActual = spaceFilling.nDesignPoints();
   EXPECT_EQ(nDesignPointsExpected, nDesignPointsActual);
}

TEST( DoeOptionsTest, TestNArbitraryDesignPointsAllDoeOptions )
{
   casaWizard::ua::DoeTornado tornado;
   tornado.setArbitraryNDesignPoints(10);
   const int nArbitraryDesignPointsTornadoExpected = 0;
   const int nArbitraryDesignPointsTornadoActual = tornado.nDesignPoints();
   EXPECT_EQ(nArbitraryDesignPointsTornadoExpected, nArbitraryDesignPointsTornadoActual);

   casaWizard::ua::DoeBoxBehnken boxBehnken;
   boxBehnken.setArbitraryNDesignPoints(10);
   const int nArbitraryDesignPointsBoxBehnkenExpected = 0;
   const int nArbitraryDesignPointsBoxBehnkenActual = boxBehnken.nDesignPoints();
   EXPECT_EQ(nArbitraryDesignPointsBoxBehnkenExpected, nArbitraryDesignPointsBoxBehnkenActual);

   casaWizard::ua::DoePlackettBurman plackettBurman;
   plackettBurman.setArbitraryNDesignPoints(10);
   const int nArbitraryDesignPointsPlackettBurmanExpected = 0;
   const int nArbitraryDesignPointsPlackettBurmanActual = plackettBurman.nDesignPoints();
   EXPECT_EQ(nArbitraryDesignPointsPlackettBurmanExpected, nArbitraryDesignPointsPlackettBurmanActual);

   casaWizard::ua::DoePlackettBurmanMirror plackettBurmanMirror;
   plackettBurmanMirror.setArbitraryNDesignPoints(10);
   const int nArbitraryDesignPointsPlackettBurmanMirrorExpected = 0;
   const int nArbitraryDesignPointsPlackettBurmanMirrorActual = plackettBurmanMirror.nDesignPoints();
   EXPECT_EQ(nArbitraryDesignPointsPlackettBurmanMirrorExpected, nArbitraryDesignPointsPlackettBurmanMirrorActual);

   casaWizard::ua::DoeFullFactorial fullFactorial;
   fullFactorial.setArbitraryNDesignPoints(10);
   const int nArbitraryDesignPointsFullFactorialExpected = 0;
   const int nArbitraryDesignPointsFullFactorialActual = fullFactorial.nDesignPoints();
   EXPECT_EQ(nArbitraryDesignPointsFullFactorialExpected, nArbitraryDesignPointsFullFactorialActual);

   casaWizard::ua::DoeSpaceFilling spaceFilling;
   spaceFilling.setArbitraryNDesignPoints(10);
   const int nArbitraryDesignPointsSpaceFillingExpected = 10;
   const int nArbitraryDesignPointsSpaceFillingActual = spaceFilling.nDesignPoints();
   EXPECT_EQ(nArbitraryDesignPointsSpaceFillingExpected, nArbitraryDesignPointsSpaceFillingActual);
}
