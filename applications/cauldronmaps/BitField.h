#ifndef BITFIELD_H
#define BITFIELD_H

// Class for the manipulation of bits.

class BitField
{
 private:
   /*  Field to store boolean attributes  */
   unsigned int		d_bitfield;

 public:
   BitField (void)
   {
      d_bitfield = 0;
   }

   BitField (const BitField & orig)
   {
      d_bitfield = orig.d_bitfield;
   }

   bool BitValueIsFalse (unsigned int  position)
   {
      return (d_bitfield & position) == 0;
   }

   bool BitValueIsTrue (unsigned int  position)
   {
      return (d_bitfield & position) != 0;
   }

   bool GetBitValue (unsigned int  position)
   {
      return (d_bitfield & position) != 0;
   }

   unsigned int GetBits (unsigned int positions)
   {
      return d_bitfield & positions;
   }

   void SetBitValue (unsigned int  position, bool value)
   {
      if (value)
      {
	 d_bitfield |= position;
      }
      else
      {
	 d_bitfield &= ~position;
      }
   }
};

#endif // BITFIELD_H
