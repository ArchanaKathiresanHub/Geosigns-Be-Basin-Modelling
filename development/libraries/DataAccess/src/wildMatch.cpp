#include "wildMatch.h"

bool WildMatch (const char *wild, const char *string)
{
   // Written by Jack Handy - jakkhandy@hotmail.com
   const char *cp = 0, *mp = 0;

   while ((*string) && (*wild != '*'))
   {
      if ((*wild != *string) && (*wild != '?'))
      {
         return false;
      }
      wild++;
      string++;
   }

   while (*string)
   {
      if (*wild == '*')
      {
         if (!*++wild)
         {
            return true;
         }
         mp = wild;
         cp = string + 1;
      }
      else if ((*wild == *string) || (*wild == '?'))
      {
         wild++;
         string++;
      }
      else
      {
         wild = mp;
         string = cp++;
      }
   }

   while (*wild == '*')
   {
      wild++;
   }
   return !*wild;
}
