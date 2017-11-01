//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file GoogleTestMacros.h
/// @brief This file is a unit testing utility which contains user defined macros for goole tests

#ifndef UTILITIES_GOOGLETESTMACROS_H
#define UTILITIES_GOOGLETESTMACROS_H

// google tests library
#include <gtest/gtest.h>


/// @brief Test that the provided statement returns the expected exception
/// @details Both the type and the content of the exception are tested
/// @param[in] statement The statement to test, shoud throw the same exception (type and content)
///    than expectedException to pass the test
/// @pram[in] expectedException The expected exception which should be thrown by the statement
#define EXPECT_EXCEPTION_EQ(statement, expectedException)                               \
{                                                                                       \
                                                                                        \
   try{                                                                                 \
      statement;                                                                        \
      FAIL() << "Expected '" + std::string(expectedException.what()) + "' exception";   \
   }                                                                                    \
   catch (const decltype(expectedException)& ex) {                                      \
      EXPECT_EQ( std::string( expectedException.what() ), std::string( ex.what() ) );   \
   }                                                                                    \
   catch (...) {                                                                        \
      FAIL() << "Expected '" + std::string( expectedException.what() ) + "' exception"; \
   }                                                                                    \
                                                                                        \
}

#endif //UTILITIES_GOOGLETESTMACROS_H
