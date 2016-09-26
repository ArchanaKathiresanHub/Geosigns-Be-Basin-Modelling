//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

double GetResidentSetSize ();

long GetPageSize ();

int GetCurrentLimit ();

int GetProcPID ();

/// \brief Returns the number of cores that are online.
long getNumberOfCoresOnline ();

/// \brief Returns the number of bytes in a page.
int getPageSize ();

/// \brief Holds the values from the /proc/self/statm file.
struct StatM
{
   unsigned long size;
   unsigned long resident;
   unsigned long share;
   unsigned long text;
   unsigned long lib;
   unsigned long data;
   unsigned long dt;
};

/// \brief Get the values from the /proc/self/statm file.
void getStatM (StatM& statm);

#endif
