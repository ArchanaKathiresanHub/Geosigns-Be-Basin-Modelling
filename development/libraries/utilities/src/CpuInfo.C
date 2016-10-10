#include "CpuInfo.h"

//Implementation
cpuInfo::cpuInfo() : m_supportSse( false ),
                     m_supportSse2( false ),
                     m_supportSse3( false ),
                     m_supportSse42( false ),
                     m_supportAvx( false ),
                     m_supportFma3( false ),
                     m_supportAvx2( false ),
                     m_supportAvx512( false ),
                     m_supportAvx512Ifma( false ),
                     m_supportAvx512er( false )
{
   // the registers regs[0] = eax, regs[1] = ebx, regs[2] = ecx, regs[3] = edx 
   uint32_t regs[4];

   // Get information from the registers (eax = 0, highest function parameter)
   native_cpuid( regs, 0 );

   // If CPU is Intel then
   // the first  4 characters are Genu
   // the second 4 characters are ineI
   // the third  4 characters are ntel
   m_vendorString[0] = static_cast<char>( regs[1] & 0xFF );
   m_vendorString[1] = static_cast<char>( ( regs[1] >> 8 ) & 0xFF );
   m_vendorString[2] = static_cast<char>( ( regs[1] >> 16 ) & 0xFF );
   m_vendorString[3] = static_cast<char>( ( regs[1] >> 24 ) & 0xFF );

   m_vendorString[4] = static_cast<char>( regs[3] & 0xFF );
   m_vendorString[5] = static_cast<char>( ( regs[3] >> 8 ) & 0xFF );
   m_vendorString[6] = static_cast<char>( ( regs[3] >> 16 ) & 0xFF );
   m_vendorString[7] = static_cast<char>( ( regs[3] >> 24 ) & 0xFF );

   m_vendorString[8] = static_cast<char>( regs[2] & 0xFF );
   m_vendorString[9] = static_cast<char>( ( regs[2] >> 8 ) & 0xFF );
   m_vendorString[10] = static_cast<char>( ( regs[2] >> 16 ) & 0xFF );
   m_vendorString[11] = static_cast<char>( ( regs[2] >> 24 ) & 0xFF );
   m_vendorString[12] = '\0';

   // Get information from the registers (eax = 1, Processor info and feature bits)
   native_cpuid( regs, 1 );

   // Get information from the registers
   // stepping   regs[0], bits 0-3
   // model      regs[0], bits 4-7
   // family     regs[0], bits 8-11

   // Sse        regs[3], bit 25
   // Sse2       regs[3], bit 26
   // Sse3       regs[2], bit 0
   // Sse42      regs[2], bit 20
   // Avx        regs[2], bit 28
   // Fma3       regs[2], bit 12

   m_family = ( regs[0] >> 8 ) & 0xF;
   m_model = ( regs[0] >> 4 ) & 0xF;
   m_stepping = ( regs[0] ) & 0xF;

   m_supportSse = getBit( regs[3], 25 );
   m_supportSse2 = getBit( regs[3], 26 );
   m_supportSse3 = getBit( regs[2], 0 );
   m_supportSse42 = getBit( regs[2], 20 );
   m_supportAvx = getBit( regs[2], 28 );
   m_supportFma3 = getBit( regs[2], 12 );

   // Get information from the registers (eax = 7, Extended features)
   native_cpuid( regs, 7 );

   // Get information from the registers
   // avx2       regs[1], bit 5
   // avx512f    regs[1], bit 16
   // avx512ifma regs[1], bit 21
   // avx512er   regs[1], bit 27
   // avx512er   regs[1], bit 27

   m_supportAvx2 = getBit( regs[1], 5 );
   m_supportAvx512 = getBit( regs[1], 16 );
   m_supportAvx512Ifma = getBit( regs[1], 21 );
   m_supportAvx512er = getBit( regs[1], 27 );
}

bool cpuInfo::getBit( unsigned int num, int position )
{
   if ( position >= 0 && position < 32 )
      return ( ( num >> position ) & 1 );
   else
      return false; // false if the position is out of bounds
}

void cpuInfo::native_cpuid( uint32_t * regs, unsigned eax )
{
#if defined(_WIN32) || defined (_WIN64)
   __cpuid( (int *)regs, (int)eax );
#else
   // call assembly instruction cpuid
   asm volatile( "cpuid"
      : "=a" ( regs[0] ),
      "=b" ( regs[1] ),
      "=c" ( regs[2] ),
      "=d" ( regs[3] )
      : "a" (eax), "c" (0) );
#endif
}