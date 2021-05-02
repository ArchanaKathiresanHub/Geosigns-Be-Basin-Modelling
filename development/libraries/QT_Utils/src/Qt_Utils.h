#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

namespace qtutils {

	QString getTimeStamp(const QString& prefix);
	QTextStream& qStdOut();
	/// <summary>
	/// Put a string between double quotes.
	/// </summary>
	/// <param name="value">Value to be put between double quotes ex: foo</param>
	/// <returns>double quoted string ex: "foo"</returns>
	QString AddDoubleQuotes(QString value);
}

