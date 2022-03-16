#ifndef Qt_Utils_h__
#define Qt_Utils_h__

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTextEdit>


namespace qtutils {
	constexpr int MAX_PROCS = 1000;
	QString getTimeStamp(const QString& prefix);
	QTextStream& qStdOut();
	/// <summary>
	/// Put a string between double quotes.
	/// </summary>
	/// <param name="value">Value to be put between double quotes ex: foo</param>
	/// <returns>double quoted string ex: "foo"</returns>
	QString AddDoubleQuotes(QString value);
	QString ExportApplicationPath(void);

	QString IsValidNoOfProcs(QString noOfProcs);
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

#endif // Qt_Utils_h__
