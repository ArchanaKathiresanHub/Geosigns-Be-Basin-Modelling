#ifndef _CPU_INFO_H_
#define _CPU_INFO_H_

// This class gets the info of the current cpu and the supported instruction sets

class cpuInfo
{
public:

   cpuInfo();
   int getFamily()          { return m_family;}
   int getModel()           { return m_model;}
   int getStepping()        { return m_stepping;}
      
   bool supportSse()        { return m_supportSse; }
   bool supportSse2()       { return m_supportSse2; }
   bool supportSse3()       { return m_supportSse3; }
   bool supportSse42()      { return m_supportSse42; }
   bool supportAvx()        { return m_supportAvx; }
   bool supportFma3( )      { return m_supportFma3; }

   bool supportAvx2()       { return m_supportAvx2; }
   bool supportAvx512()     { return m_supportAvx512; }
   bool supportAvx512Ifma() { return m_supportAvx512Ifma; }
   bool supportAvx512Er( )  { return m_supportAvx512er; }

   char * getVendor()       { return m_vendorString; }

private:

   int getBit( unsigned int num, int position );

#if not defined(_WIN32) && not defined (_WIN64)
   void native_cpuid( unsigned int *eax, unsigned int *ebx,
      unsigned int *ecx, unsigned int *edx );
#endif

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

//Implementation
cpuInfo::cpuInfo( ): m_supportSse( false ),
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
#if not defined(_WIN32) && not defined (_WIN64)
   unsigned int level;
   unsigned int eax;
   unsigned int ebx;
   unsigned int ecx;
   unsigned int edx;

   // Get information from the registers (highest function parameter)
   eax = 0;
   native_cpuid( &eax, &ebx, &ecx, &edx );

   // If CPU is Intel then
   // the first  4 characters are Genu
   // the second 4 characters are ineI
   // the third  4 characters are ntel
   m_vendorString[0] = static_cast<char>( ebx & 0xFF );
   m_vendorString[1] = static_cast<char>( ( ebx >> 8 ) & 0xFF );
   m_vendorString[2] = static_cast<char>( ( ebx >> 16 ) & 0xFF );
   m_vendorString[3] = static_cast<char>( ( ebx >> 24 ) & 0xFF );

   m_vendorString[4] = static_cast<char>( edx & 0xFF );
   m_vendorString[5] = static_cast<char>( ( edx >> 8 ) & 0xFF );
   m_vendorString[6] = static_cast<char>( ( edx >> 16 ) & 0xFF );
   m_vendorString[7] = static_cast<char>( ( edx >> 24 ) & 0xFF );

   m_vendorString[8] = static_cast<char>( ecx & 0xFF );
   m_vendorString[9] = static_cast<char>( ( ecx >> 8 ) & 0xFF );
   m_vendorString[10] = static_cast<char>( ( ecx >> 16 ) & 0xFF );
   m_vendorString[11] = static_cast<char>( ( ecx >> 24 ) & 0xFF );
   m_vendorString[12] = '\0';

   // Get information from the registers (Processor info and feature bits)
   eax = 1;
   native_cpuid( &eax, &ebx, &ecx, &edx );

   // Get information from the registers
   // stepping   eax, bits 0-3
   // model      eax, bits 4-7
   // family     eax, bits 8-11

   // Sse        edx, bit 25
   // Sse2       edx, bit 26
   // Sse3       ecx, bit 0
   // Sse42      ecx, bit 20
   // Avx        ecx, bit 28
   // Fma3       ecx, bit 12

   m_family = ( eax >> 8 ) & 0xF;
   m_model = ( eax >> 4 ) & 0xF;
   m_stepping = (eax)& 0xF;

   m_supportSse = getBit( edx, 25 );
   m_supportSse2 = getBit( edx, 26 );
   m_supportSse3 = getBit( ecx, 0 );
   m_supportSse42 = getBit( ecx, 20 );
   m_supportAvx = getBit( ecx, 28 );
   m_supportFma3 = getBit( ecx, 12 );

   // Get information from the registers (Extended features)
   eax = 7;
   ecx = 0;
   native_cpuid( &eax, &ebx, &ecx, &edx );

   // Get information from the registers
   // avx2       ebx, bit 5
   // avx512f    ebx, bit 16
   // avx512ifma ebx, bit 21
   // avx512er   ebx, bit 27
   // avx512er   ebx, bit 27

   m_supportAvx2 = getBit( ebx, 5 );
   m_supportAvx512 = getBit( ebx, 16 );
   m_supportAvx512Ifma = getBit( ebx, 21 );
   m_supportAvx512er = getBit( ebx, 27 );

#endif
}

int cpuInfo::getBit( unsigned int num, int position )
{
   if ( position >= 0 && position < 32 )
      return ( ( num >> position ) & 1 );
   else
      return 0; // false if the position is out of bounds
}

#if not defined(_WIN32) && not defined (_WIN64)
void cpuInfo::native_cpuid( unsigned int *eax, unsigned int *ebx,
   unsigned int *ecx, unsigned int *edx )
{
   asm volatile( "cpuid"
      : "=a" ( *eax ),
      "=b" ( *ebx ),
      "=c" ( *ecx ),
      "=d" ( *edx )
      : "0" ( *eax ), "2" ( *ecx ) );
}
#endif

#endif