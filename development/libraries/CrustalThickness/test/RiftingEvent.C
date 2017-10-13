//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/RiftingEvent.h"

// std library
#include <string>
#include <exception>

// SerialDataAccess library
#include "../../SerialDataAccess/src/Interface/SerialGrid.h"
#include "../../SerialDataAccess/src/Interface/SerialGridMap.h"

// DataAccess library
#include "../../DataAccess/src/Interface/Interface.h"

#include <gtest/gtest.h>

using CrustalThickness::RiftingEvent;

DataAccess::Interface::SerialGrid grid( 0, 0, 1, 1, 2, 2 );
DataAccess::Interface::SerialGridMap seaLevelAdjustment    ( 0, 0, &grid, 500,  1 );
DataAccess::Interface::SerialGridMap maximumBasaltThickness( 0, 0, &grid, 8000, 1 );

TEST( RiftingEvent, constructor ){
   //1. Test that the constructor throws an exception if the sea level adjustement map is a nullptr
   std::string expectedOutput = "No sea level adjustement is provided for the current rifting event";
   //test if the exception is thrown
   EXPECT_THROW( RiftingEvent event( DataAccess::Interface::ACTIVE_RIFTING, nullptr, &maximumBasaltThickness ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      RiftingEvent event( DataAccess::Interface::ACTIVE_RIFTING, nullptr, &maximumBasaltThickness );
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }

   //2. Test that the constructor throws an exception if the maximum basalt thickness map is a nullptr
   expectedOutput = "No maximum oceanic crustal thickness is provided for the current rifting event";
   //test if the exception is thrown
   EXPECT_THROW( RiftingEvent event( DataAccess::Interface::ACTIVE_RIFTING, &seaLevelAdjustment, nullptr ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      RiftingEvent event( DataAccess::Interface::ACTIVE_RIFTING, &seaLevelAdjustment, nullptr );
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }

   //3. Test default values
   RiftingEvent event( DataAccess::Interface::ACTIVE_RIFTING, &seaLevelAdjustment, &maximumBasaltThickness );
   EXPECT_EQ( DataAccess::Interface::ACTIVE_RIFTING             , event.getTectonicFlag()                                     );
   EXPECT_EQ( 500                                               , event.getSeaLevelAdjustment()->getConstantValue()           );
   EXPECT_EQ( 8000                                              , event.getMaximumOceanicCrustThickness()->getConstantValue() );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedScalarValue, event.getStartRiftAge()                                     );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedScalarValue, event.getEndRiftAge()                                       );
   EXPECT_EQ( 0                                                 , event.getRiftId()                                           );
   EXPECT_EQ( false                                             , event.getCalculationMask()                                  );
}

TEST( RiftingEvent, star_age ){
   RiftingEvent event( DataAccess::Interface::ACTIVE_RIFTING, &seaLevelAdjustment, &maximumBasaltThickness );
   //1. Test that the class throws an exception if the start age is <0 and not a NDV
   std::string expectedOutput = "Cannot set a start rift age for the rifting event to a negative value";
   //test if the exception is thrown
   EXPECT_THROW(event.setStartRiftAge( -10 ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      event.setStartRiftAge( -10 );
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }

   //2. Test that the class throws an exception if the start age is =0 and not a NDV
   expectedOutput = "Cannot set a start rift age for the rifting event to present day 0Ma";
   //test if the exception is thrown
   EXPECT_THROW( event.setStartRiftAge( 0 ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      event.setStartRiftAge( 0 );
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }


   //3. Test that the class throws an exception if the start age is >= end age and both are not NDV
   expectedOutput = "Cannot set a start rift age for the rifting event to " +std::to_string( 10.0 ) +
      "Ma because it is posterior to its end rift age " + std::to_string( 20.0 ) + "Ma";
   event.setEndRiftAge( 20 );
   //test if the exception is thrown
   EXPECT_THROW( event.setStartRiftAge( 10 ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      event.setStartRiftAge( 10 );
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }


   //4. Test that the good values are assigned when we set the start age to NDV
   event.setStartRiftAge( DataAccess::Interface::DefaultUndefinedScalarValue );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedScalarValue, event.getStartRiftAge() );

   //5. Test that the good values are assigned when we set the start age to not an NDV value and end age is NDV
   event.setEndRiftAge( DataAccess::Interface::DefaultUndefinedScalarValue );
   event.setStartRiftAge( 20 );
   EXPECT_EQ( 20, event.getStartRiftAge() );

   //6. Test that the good values are assigned when we set the start age to not an NDV value and end age is not NDV
   event.setEndRiftAge( 10 );
   event.setStartRiftAge( 30 );
   EXPECT_EQ( 30, event.getStartRiftAge() );

}

TEST( RiftingEvent, end_age ){
   RiftingEvent event( DataAccess::Interface::ACTIVE_RIFTING, &seaLevelAdjustment, &maximumBasaltThickness );
   //1. Test that the class throws an exception if the end age is <0 and not a NDV
   std::string expectedOutput = "Cannot set an end rift age for the rifting event to a negative value";
   //test if the exception is thrown
   EXPECT_THROW( event.setEndRiftAge( -10 ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      event.setEndRiftAge( -10 );
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }

   //2. Test that the class throws an exception if the start age is >= end age and both are not NDV
   expectedOutput = "Cannot set an end rift age for the rifting event to " + std::to_string( 30.0 ) +
      "Ma because it is anterior to its start rift age " + std::to_string( 20.0 ) + "Ma";
   event.setStartRiftAge( 20 );
   //test if the exception is thrown
   EXPECT_THROW( event.setEndRiftAge( 30 ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      event.setEndRiftAge( 30 );
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( expectedOutput, std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected '" + expectedOutput + "' exception";
   }


   //4. Test that the good values are assigned when we set the end age to NDV
   event.setEndRiftAge( DataAccess::Interface::DefaultUndefinedScalarValue );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedScalarValue, event.getEndRiftAge() );

   //5. Test that the good values are assigned when we set the end age to not an NDV value and start age is NDV
   event.setStartRiftAge( DataAccess::Interface::DefaultUndefinedScalarValue );
   event.setEndRiftAge( 20 );
   EXPECT_EQ( 20, event.getEndRiftAge() );

   //6. Test that the good values are assigned when we set the end age to not an NDV value and start age is not NDV
   event.setStartRiftAge( 30 );
   event.setEndRiftAge( 10 );
   EXPECT_EQ( 10, event.getEndRiftAge() );
}

TEST( RiftingEvent, rifting_id ){
   RiftingEvent event( DataAccess::Interface::ACTIVE_RIFTING, &seaLevelAdjustment, &maximumBasaltThickness );
   event.setRiftId( 10 );
   EXPECT_EQ( 10, event.getRiftId() );
}

TEST( RiftingEvent, calculation_mask ) {
   RiftingEvent event( DataAccess::Interface::ACTIVE_RIFTING, &seaLevelAdjustment, &maximumBasaltThickness );
   event.setCalculationMask( true );
   EXPECT_EQ( true, event.getCalculationMask() );
}