#include "ProgressManager.h"
using namespace utilities;

#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

ProgressManager::ProgressManager()
{
}

ProgressManager::~ProgressManager()
{
}

void ProgressManager::start()
{
   m_timeStarted = WallTime::clock ();
}

std::string ProgressManager::report (const std::string & message)
{
   WallTime::Time clockTime = WallTime::clock ();

   WallTime::Duration executionTime = clockTime - m_timeStarted;

   int len = message.size ();

   ostringstream buf;
   buf << message;
   buf << " ";
   while (len < 80)
   {
      buf << " ";
      ++len;
   }
   buf << "Time = ";
   buf << executionTime.asString ();
   return buf.str();
}
