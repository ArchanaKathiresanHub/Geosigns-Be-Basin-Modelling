#ifndef _MIGRATION_UTILS_H
#define _MIGRATION_UTILS_H

#include <string>

using std::string;

namespace migration {

namespace utils {

string getProjectBaseName(const string& projectName);

string replaceSpaces(const string& str);

} // namespace utils

} // namespace migration

#endif
