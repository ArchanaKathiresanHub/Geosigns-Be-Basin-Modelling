#ifndef __H5_STRUCT_TYPES__
#define __H5_STRUCT_TYPES__

#include "DllExport.h"
#define MAX_STRING_SIZE 200

#ifdef powf
#undef powf
#endif
#include <iostream>
using namespace std;

//
// Fixed String structure 
//
class SERIALHDF5_DLL_EXPORT FixedString
{
public:
   // ctor / dtor
   FixedString () { str[0] = '\0'; }
   FixedString (const char* s) { str[0] = '\0'; safeCopy (str, s); }

   // public methods 
   static int getStructSize ()
   {
      return sizeof (FixedString);
   }

   int getStringSize () const
   {
      return MAX_STRING_SIZE;
   }

   FixedString& operator= (const char *s);
   
   ostream& write (ostream& os) const
   {
      return os << str;
   }

   istream& read (istream &is)
   {
      is.getline (str, MAX_STRING_SIZE, '\0');
      return is;
   }

   char* stringValue (void) { return str; }
   
   // operators
   char operator[] (int index)
   {
      return str[index];
   }

   char* operator* (void)
   {
      return str;
   }

   // needs to be public for H5 compound types
   char str [MAX_STRING_SIZE];

private:
   void safeCopy   (char *s1, const char*s2);
   bool stringOkay (const char *s);  
   void strError   (ostream &os);
};

static ostream& operator<< (ostream &os, const FixedString &str)
{
   return str.write (os);
}

static istream& operator>> (istream &is, FixedString &str)
{
   return str.read (is);
}

//
// Grid Structure
//

typedef struct SERIALHDF5_DLL_EXPORT GridStruct
{
   FixedString gridName;
   
   static int getStructSize ()
   {
      return sizeof (GridStruct);
   }

} GridStruct;

//
// Stratigraphy Row Structure
//
typedef struct SERIALHDF5_DLL_EXPORT StratRowStruct
{
   // set default values for row
   StratRowStruct () 
   : DepoAge           (0),
     TwoWayTime        (0),
     DepoSequence      (0),
     SourceRock        (0),
     MobileLayer       (0)
   {}

   ~StratRowStruct () {}
   
   float      DepoAge; // double in Cauldron but float in project file
   GridStruct DepthGrid;
   float      TwoWayTime;
   int        DepoSequence;
   GridStruct ThicknessGrid;
   GridStruct PercentGrid1;
   GridStruct PercentGrid2;
   GridStruct PercentGrid3;
   int        SourceRock;
   int        MobileLayer;
   
   FixedString SurfaceName;
   FixedString LayerName;
   FixedString LithoType1;
   FixedString LithoType2;
   FixedString LithoType3;
   FixedString Percent1;
   FixedString Percent2;
   FixedString Percent3;
   FixedString MixModel;
   FixedString SeisVelocityModel;
   FixedString FluidType;

   static int getStructSize (void) 
   {
      return sizeof (StratRowStruct);
   }
 
} StratRowStruct;

#endif


