#ifndef _PROCESS_H
#define _PROCESS_H
int processCmd (char * cmd, char ** args, bool wait = true);
void waitForChildren (int numChildren = 0);
int getNumProcessors (void);
void calculateMaxNumChildren (void);
void setMaxNumChildren (int numChildren);
char * findExecutable (char * name);
bool childHasFinished (int pid, int & exitValue);
void killChild (int pid);
#endif
