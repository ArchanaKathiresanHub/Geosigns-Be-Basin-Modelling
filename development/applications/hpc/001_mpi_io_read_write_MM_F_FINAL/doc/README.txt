
// Works ok with threads.detach()
// but thre is no synchronization
// and functions encapsulation

// I added file read-synchronization
// CPU time consuming is always done in serial
// but the related to it write statement is detached

// array space is local to each thread
// Works excellent on stampede.tacc.utexas.edu
