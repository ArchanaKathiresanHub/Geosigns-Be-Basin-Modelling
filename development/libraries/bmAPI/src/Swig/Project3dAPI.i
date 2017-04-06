/* File : Project3dAPI.i */
 
%module Project3dAPI

%include "stl.i"
%include "std_except.i"

%include "exception.i"

%exception bmapi::ProjectIoAPI::ProjectIoAPI {
   try {
      $action
   }
   SWIG_CATCH_STDEXCEPT // catch std::exception
   catch (...) { SWIG_exception(SWIG_UnknownError, "Unknown exception"); }
}

%exception bmapi::ProjectIoAPI::saveToProjectFile {
   try {
      $action
   }
   SWIG_CATCH_STDEXCEPT // catch std::exception
   catch (...) { SWIG_exception(SWIG_UnknownError, "Unknown exception"); }
} 

%{
/* Includes the header in the wrapper code */
#include "cauldronschemaAPI.h"

using namespace database;
using namespace bmapi;
%}

/* some output reference types */
namespace Project3dAPI
{
}

%rename(Equals) *::operator ==;

%ignore *::begin;
%ignore *::end;


%template(StringVector)		           std::vector< std::string >;
%template(IntVector)		              std::vector< int >;


%include "cauldronschemaAPI.h"

/* some templates */
