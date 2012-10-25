#ifndef _BITFIELD_H
#define _BITFIELD_H

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
#endif // sgi

#include <assert.h>

// Class for the manipulation of bits.

class BitField
{
 private:
   /*  Field to store boolean attributes  */
   unsigned int	m_values;

 public:
   BitField (void)
   {
      clearAll ();
   }

   void clearAll (void)
   {
      m_values = 0;
   }

   unsigned int getField (unsigned int position) const
   {
      const int fields[] =
      {
	 1 <<  0, 1 <<  1, 1 <<  2, 1 <<  3, 1 <<  4, 1 <<  5, 1 <<  6, 1 <<  7, 1 <<  8, 1 <<  9,
	 1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15, 1 << 16, 1 << 17, 1 << 18, 1 << 19,
	 1 << 20, 1 << 21, 1 << 22, 1 << 23, 1 << 24, 1 << 25, 1 << 26, 1 << 27, 1 << 28, 1 << 29,
	 1 << 30, 1 << 31
      };

      assert (position < 32);

      return fields[position];
   }

   bool valueIsFalse (unsigned int position) const
   {
      return (m_values & getField (position)) == 0;
   }

   bool valueIsTrue (unsigned int position) const
   {
      return (m_values & getField (position)) != 0;
   }

   bool getValue (unsigned int position) const
   {
      return (m_values & getField (position)) != 0;
   }

   void setValue (unsigned int position, bool value)
   {
      if (value)
      {
	 m_values |= getField (position);
      }
      else
      {
	 m_values &= ~getField (position);
      }
   }
};

#endif // _BITFIELD_H


