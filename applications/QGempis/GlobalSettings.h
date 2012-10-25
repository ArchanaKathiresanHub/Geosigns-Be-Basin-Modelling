#ifndef _GLOBAL_SETTINGS_H_
#define _GLOBAL_SETTINGS_H_

#include <QString>

class GlobalSettings
{
public:
	static QString GetUsername();
	static void SetUsername(const QString& s);

	static QString GetPassword();
	static void SetPassword(const QString& s);
};

#endif
