#ifndef _CPU_INFO_H_
#define _CPU_INFO_H_

#if defined(_WIN32) || defined (_WIN64)
#include <intrin.h>
typedef unsigned __int32  uint32_t;
#else
#include <stdint.h>
#endif

// This class gets CPU info and the supported instructions both in Windows and Linux

class cpuInfo
{
public:

   cpuInfo();
   int getFamily() const          { return m_family; }
   int getModel() const           { return m_model; }
   int getStepping() const        { return m_stepping; }

   bool supportSse() const        { return m_supportSse; }
   bool supportSse2() const       { return m_supportSse2; }
   bool supportSse3() const       { return m_supportSse3; }
   bool supportSse42() const      { return m_supportSse42; }
   bool supportAvx() const        { return m_supportAvx; }
   bool supportFma3() const       { return m_supportFma3; }

   bool supportAvx2() const       { return m_supportAvx2; }
   bool supportAvx512() const     { return m_supportAvx512; }
   bool supportAvx512Ifma() const { return m_supportAvx512Ifma; }
   bool supportAvx512Er() const   { return m_supportAvx512er; }

   char * getVendor()             { return m_vendorString; }

private:

   bool getBit( unsigned int num, int position );

   void native_cpuid( uint32_t * regs, unsigned eax );

   int  m_family;
   int  m_model;
   int  m_stepping;

   bool m_supportSse;
   bool m_supportSse2;
   bool m_supportSse3;
   bool m_supportSse42;
   bool m_supportAvx;
   bool m_supportFma3;

   bool m_supportAvx2;
   bool m_supportAvx512;
   bool m_supportAvx512Ifma;
   bool m_supportAvx512er;

   char m_vendorString[13];
};

#endif