#ifndef GlobalData_H
#define GlobalData_H


#include <qstring.h>
#include <qstringlist.h>


// a static class for saving data
class GlobalData
{
 public:
  static void SetPath( const QString & );
  static const QString & GetPath( void );

 private:
  static QString s_path;
};

#endif











