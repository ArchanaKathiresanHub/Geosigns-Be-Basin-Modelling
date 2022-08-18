//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Qt_Utils.h"
#include <QCoreApplication>
#include <QDebug>
#include <QScrollBar>
#include <fstream>
#include <thread>
#include <iostream>
#include <string>

namespace qtutils
{

QString doubleToQString(double d)
{
   return QString::number(d, 'g', 12);
};

QString doubleOneDigitToQString(double d)
{
   return QString::number(d, 'f', 1);
};

QStringList convertToQStringList(const QVector<int>& vec)
{
   QStringList out;
   for( const int i : vec)
   {
      out << QString::number(i);
   }
   return out;
}

QStringList convertToQStringList(const QVector<double>& vec)
{
   QStringList out;
   for( const double d : vec)
   {
      out << qtutils::doubleToQString(d);
   }
   return out;
}

QStringList mergeQStringLists(const QVector<QStringList>& vec, const QString& sublistSeparator)
{
   QStringList out;
   for (const QStringList& l : vec)
   {
      out << l.join(sublistSeparator);
   }
   return out;
}

QString getTimeStamp(const QString& prefix)
{
   const  QDateTime now{ QDateTime::currentDateTime() };
   return prefix + now.toString("yyyyMMdd-HHmmss");
}

QTextStream& qStdOut()
{
   static QTextStream ts(stdout);
   return ts;
}

QString addDoubleQuotes(QString value)
{
   return "\"" + value + "\"";
}

QString escapeSpecialCharacters(QString str)
{
   str = str.replace("\a","\\a")
         .replace("\b","\\b")
         .replace("\f","\\f")
         .replace("\n","\\n")
         .replace("\r","\\r")
         .replace("\t","\\t")
         .replace("\v","\\v");
   return str;
}

QString replaceCharsNotAllowedInExcelTabsBy_(QString str)
{
   //Characters not allowed in excel tab: \ / ? * [ ]
   str = str.replace("\\","_")
         .replace(":","_")
         .replace("/","_")
         .replace("?","_")
         .replace("*","_")
         .replace("[","_")
         .replace("]","_");
   return str;
}

bool isEqual(const QVector<QString>& strVec1, const QVector<QString>& strVec2)
{
   if (strVec1.size() != strVec2.size())
   {
      return false;
   }
   for (int i = 0; i< strVec1.size(); i++)
   {
      if(strVec1[i] != strVec2[i])
      {
         return false;
      }
   }
   return true;
}

std::string replaceFirstOccurrence(
      std::string& s,
      const std::string& toReplace,
      const std::string& replaceWith)
{
   std::size_t pos = s.find(toReplace);
   if (pos == std::string::npos) return s;
   return s.replace(pos, toReplace.length(), replaceWith);
}

QString exportApplicationPath(void)
{
   std::string applicationPath = QCoreApplication::applicationDirPath().toStdString();
   std::size_t index = applicationPath.find("/apps/sss");
   if (index != std::string::npos)
   {
      applicationPath = applicationPath.substr(index);
      // there is superfluous name extension in AMS ssstest.
      index = applicationPath.find("ssstest_rhn");
      if (index != std::string::npos)
      {
         replaceFirstOccurrence(applicationPath, "ssstest_rhn", "ssstest");
      }
   }
   return QString::fromStdString(applicationPath);
}

QString isValidNoOfProcs(QString noOfProcs)
{
   bool validate;
   auto value = noOfProcs.toInt(&validate);
   QString ErrMsg = "Ok";
   ErrMsg = !validate ? " no. of Processors should be an Integer!" : ErrMsg;
   ErrMsg = value < 0 ? " no. of Processors should be a positive Integer!" : ErrMsg;
   ErrMsg = value > MAX_PROCS ? " no. of Processors should be < " + QString::number(MAX_PROCS) + "!" : ErrMsg;

   return ErrMsg;
}

QString getOutpurDirNameFromP3FileName(QString pathToP3File)
{
   QFileInfo info(pathToP3File);
   QStringList strLst = info.fileName().simplified().split(".");
   return info.dir().absolutePath() + "/" + strLst[0] + "_CauldronOutputDir";
}

bool delay(int secs)
{
   QTime dieTime = QTime::currentTime().addSecs(secs);
   while (QTime::currentTime() < dieTime)
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

   return true;
}

void FileLogger::returnToLineNumber(int lineNumber)
{
   lineNr_ = lineNumber;
}

void FileLogger::logFromFile(const QString& logFilePath, QTextEdit* theText) const
{
   QFileInfo checkFile(logFilePath);

   if (checkFile.exists())
   {
      QFile readfile(logFilePath);
      if (!readfile.open(QFile::ReadOnly | QFile::Text))
      {
         qDebug() << "error opening file: " << readfile.errorString();
         return;
      }

      int i = 1;
      QTextStream stream(&readfile);
      while (!stream.atEnd())
      {
         QString line = stream.readLine();
         if (i > lineNr_)
         {
            const_cast<int&>(lineNr_) = i;
#ifdef DEBUG
            qDebug() << "WARNING: logging line no:" << i << ' ' << line;
#endif
            theText->append(line);
            theText->verticalScrollBar()->setSliderPosition(
                     theText->verticalScrollBar()->maximum());
         }
         ++i;
      }
      readfile.close();
   }
   //#ifdef DEBUG
   else
   {
      qDebug() << "WARNING: the log file " << logFilePath << " does not exist, yet!";
   }
   //#endif
   return;
}

void FileLogger::finishupLogging(const QString& logFilePath, QTextEdit* theText) const
{
   QFileInfo checkFile(logFilePath);
   /// if the file is never read and is present
   if (checkFile.exists())
   {
      if (lineNr_ == 0)
      {
         QFile myFile(logFilePath);
         //read the entire file
         //#ifdef DEBUG
         qDebug() << "WARNING: logging the unread file the file " << logFilePath << "!";
         //#endif
         if (!myFile.open(QFile::ReadOnly | QFile::Text))
         {
            qDebug() << "error opening file: " << myFile.errorString();
            return;
         }
         QTextStream textStream(&myFile);
         QString line = textStream.readAll();
         myFile.close();
         theText->append(line);
      }
      //#ifdef DEBUG
      else
      {
         qDebug() << "WARNING: the log file " << logFilePath << " was already read before!";
      }
      //#endif
   }
   else {
      //#ifdef DEBUG
      qDebug() << "WARNING: the log file " << logFilePath << " does not exist for finishing up... retrying!";
      //#endif
      auto ok = readFileWhenCreated(logFilePath, theText);
      if (ok > 0)
      {
         theText->append("failed to load logged report here, check " + logFilePath + " manually, sorry!");
      }
   }

}

int FileLogger::readFileWhenCreated(const QString& logFilePath, QTextEdit* theText) const
{
   QFileInfo checkFile(logFilePath);
   int maxRetryCounter = 0;
   int waitTimeInMs = 100;
   /// safety net, should the file never come to existence...
   while (!checkFile.exists() && maxRetryCounter <= maxRetryCount) {
      std::this_thread::sleep_for(std::chrono::milliseconds(waitTimeInMs));
      maxRetryCounter++;
   }

   if (maxRetryCounter <= maxRetryCount)
   {
      finishupLogging(logFilePath, theText);
   }
   else {
      theText->append("log file wasn't created in " + QString::number(waitTimeInMs*maxRetryCounter*1e-3) + " sec");
      return 1;
   }
   return 0;
}

} //qtutils
