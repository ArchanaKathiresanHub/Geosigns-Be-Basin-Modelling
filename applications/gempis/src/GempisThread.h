#ifndef GempisThread_H
#define GempisThread_H _GempisThread_H

#include <LogFile.h>

#include <qthread.h>
#include <qmutex.h>
#include <qdatetime.h>
#include <vector>
#include <string>


const int BufferSize = 4096;


class GempisThread : public QThread
{
 public:
  GempisThread();
  ~GempisThread();

  void SetCommand( const QString & );     // define log file
  void SetLogFileName( const QString & ); // define command top execute

  bool GetOutputAvailable();
  QString GetNextOutputLine();
  QDateTime GetLastUpdate( void );

  bool GetRunning( void );
  long GetResult( void );

  void KillThread( void );  // kill selected child processes

  void run();               // QThread pure virtual function


 private:
  volatile bool m_running;
  long m_result;
  QString m_logFileName;
  QString m_command;

  // output buffer
  long m_outputLastWrite;
  long m_outputLastRead;
  QMutex m_mutex;
  QStringList m_outputText;
  QDateTime m_lastUpdate;
};


#endif
