#if !defined(INCLUDED_PETSCFIX_H)
#define INCLUDED_PETSCFIX_H

typedef int gid_t;
typedef int uid_t;
typedef int int32_t;
typedef int mode_t;
typedef int pid_t;
#if defined(__cplusplus)
extern "C" {
int getdomainname(char *, int);
double drand48();
void   srand48(long);
#include <stddef.h> 
void *_intel_fast_memcpy(void *,const void *,size_t);
#include <stddef.h> 
void *_intel_fast_memset(void *, int, size_t);
}
#else
#include <stddef.h> 
void *_intel_fast_memcpy(void *,const void *,size_t);
#include <stddef.h> 
void *_intel_fast_memset(void *, int, size_t);
#endif
#endif
