//
// Copyright (C) 2012 - 2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <iostream>
#include <sstream>
#include <ostream>
#include <QObject>

class QCheckBox;

namespace casaWizard
{

class GuiTester
{

public:
   GuiTester();
   virtual bool run();

public:
   std::streambuf* m_testBuffer;
   std::ostringstream m_logStream; //stream for the normal sac messages
   void reportStep( const bool succes, const std::string& message);

protected:
   int m_failedSteps;

private:
   virtual void runValidations() = 0;
   virtual void validateLoadingSaves() = 0;
};

} // namespace casaWizard
