#include "utils.h"

namespace migration {

namespace utils {

string getProjectBaseName(const string& projectName)
{
   string result = projectName;
   string::size_type dotPos = result.rfind (".project");
   if (dotPos != string::npos)
      result.erase(dotPos, string::npos);
   return result;
}

string replaceSpaces(const string& str) {
   string result = str;
   for (unsigned int i = 0; i < result.length(); ++i)
      if (result[i] == ' ') result[i] = '_';
   return result;
}

} // namespace utils

} // namespace migration
