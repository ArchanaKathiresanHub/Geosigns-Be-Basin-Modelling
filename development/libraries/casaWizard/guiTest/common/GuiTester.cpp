//
// Copyright (C) 2012 - 2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "GuiTester.h"
#include <string>

namespace casaWizard
{

GuiTester::GuiTester() :
   m_failedSteps{0},
   m_testBuffer(std::cout.rdbuf())
{

}

bool GuiTester::run()
{
   std::cout << "-------------- Starting Automated GUI Test --------------" << std::endl;
   try
   {
      runValidations();
   }
   catch (const std::runtime_error& e)
   {
      std::cout.rdbuf(m_testBuffer); // restore buffer in case it was set to nullptr before the throw

      reportStep(false, e.what());
      std::cout << "* Critical workflow error, cannot continue test" << std::endl;

      std::cout << "--------------- Automated GUI test failed ---------------" << std::endl;
      return EXIT_FAILURE;
   }
   catch ( ... )
   {
      std::cout.rdbuf(m_testBuffer); // restore buffer in case it was set to nullptr before the throw

      std::cout << "* Unknown error occured during the test, cannot continue test" << std::endl;

      std::cout << "--------------- Automated GUI test failed ---------------" << std::endl;
      return EXIT_FAILURE;
   }

   if (m_failedSteps > 0)
   {
      std::cout << " >> " + std::to_string(m_failedSteps) +
                   " step" + (m_failedSteps == 1 ? "" : "s") + " failed!\n";
      std::cout << "--------------- Automated GUI test failed ---------------" << std::endl;
      return EXIT_FAILURE;
   }

   std::cout << "--------------- Automated GUI test passed ---------------" << std::endl;
   return EXIT_SUCCESS;
}

void GuiTester::reportStep( const bool succes, const std::string& message)
{
   std::string statement = (succes ? "\t** SUCCES" : "\t** FAILED TEST STEP" );
   statement.append( (message.empty() ?  "! " : ": " + message) + " **" );

   std::cout << statement << std::endl;
   if(!succes)
   {
      m_failedSteps++;
   }
}

} // namespace casawizard
