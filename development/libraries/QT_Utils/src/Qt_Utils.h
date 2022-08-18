//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTextEdit>
#include <QString>

namespace qtutils {
	constexpr int MAX_PROCS = 1000;
	QString getTimeStamp(const QString& prefix);
	QTextStream& qStdOut();

	/// <summary>
	/// Put a string between double quotes.
	/// </summary>
	/// <param name="value">Value to be put between double quotes ex: foo</param>
	/// <returns>double quoted string ex: "foo"</returns>
   QString addDoubleQuotes(QString value);
   QString exportApplicationPath(void);

   /// <summary>
   ///Escapes the special characters, such that they are written out as text
   QString escapeSpecialCharacters(QString str);
   QString replaceCharsNotAllowedInExcelTabsBy_(QString str);

   QString doubleToQString(double d);
   QString doubleOneDigitToQString(double d);

   QStringList convertToQStringList(const QVector<int>& vec);
   QStringList convertToQStringList(const QVector<double>& vec);
   QStringList mergeQStringLists(const QVector<QStringList>& vec, const QString& sublistSeparator);

   bool isEqual(const QVector<QString>& strVec1, const QVector<QString>& strVec2);

   QString isValidNoOfProcs(QString noOfProcs);
	QString getOutpurDirNameFromP3FileName(QString pathToP3File);
	bool delay(int secs);

	class FileLogger
	{
	public:
		/// <summary>
		/// giving more flexibility to revisit logged lines
		/// </summary>
		/// <param name="lineNumber"></param>
		void returnToLineNumber(int lineNumber=0);
		void logFromFile(const QString& logFilePath, QTextEdit* theText) const;
		void finishupLogging(const QString& logFilePath, QTextEdit* theText=nullptr) const;
		/// Sometimes the WLM returns thread even before the log files is closed for reading, next.
		/// Hence we wait for 100 ms before trying again
		int readFileWhenCreated(const QString& logFilePath, QTextEdit* theText) const;
	private:
		int lineNr_ = 0;
		const int maxRetryCount = 600;// approx 1 min
	};
}
