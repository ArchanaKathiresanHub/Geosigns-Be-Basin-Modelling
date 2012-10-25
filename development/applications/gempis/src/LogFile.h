#ifndef LogFile_H
#define LogFile_H _LogFile_H

#include <qthread.h>
#include <qmutex.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qtextstream.h>
#include <vector>
#include <string>


class LogFile
{
 public:
  LogFile( const QString & );
  ~LogFile();

  void Reset( void );
  void Append( void );
  void WriteRaw( const char buffer[] );
  void Write( const QString & );
  void Write( int );
  void WriteLine( const QString & );
  void WriteLine( int );

 private:
  QString m_logFileName;
  QFile * m_f;
  //  QTextStream * m_ts;
};


#endif
