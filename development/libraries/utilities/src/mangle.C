#include "stdafx.h"
#include <stdlib.h>

#if !(defined (_WIN32) || defined (_WIN64))
#include <unistd.h>
#endif

#include <iostream>
#define USESTANDARD

#include "mangle.h"

std::string utilities::mangle (const std::string & name)
{
   const std::string NonUsableCharacters = " /*+-[]<>()^=\"|:\\~,.";

#if 0
   cerr << "Mangling " << name << endl;
#endif
   size_t pos = 0;
   std::string mangled = name;
   while ((pos = mangled.find_first_of (NonUsableCharacters, pos)) != std::string::npos)
   {
      mangled[pos] = '_';
#if 0
      cerr << "Pos " << pos << ": Mangled " << name << " into " << mangled << endl;
#endif
   }

   return mangled;
}
