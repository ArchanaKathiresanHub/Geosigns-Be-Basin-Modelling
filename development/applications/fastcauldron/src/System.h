#ifndef _SYSTEM_H_
#define _SYSTEM_H_

double GetResidentSetSize();

long GetPageSize();

int GetCurrentLimit();

int GetProcPID();

/// \brief Returns the number of cores that are online.
long getNumberOrCoresOnline ();


/// \brief Holds the values from the /proc/self/statm file.
struct StatM {
   unsigned long size;
   unsigned long resident;
   unsigned long share;
   unsigned long text;
   unsigned long lib;
   unsigned long data;
   unsigned long dt;
};

/// \brief Get the values from the /proc/self/statm file.
void getStatM ( StatM& statm );

#endif
