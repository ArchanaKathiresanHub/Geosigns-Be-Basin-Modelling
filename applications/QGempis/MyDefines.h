#ifndef _MYDEFINES_H_
#define _MYDEFINES_H_

#include <stdexcept>

#define APPLICATION_NAME "QGempis"
#define ORGANISATION_NAME "Shell"

// #define REMOTE_HOSTNAME "rijkes-n-d99540.linux.shell.com"
#define REMOTE_HOSTNAME "amsdc1-n-s00037.europe.shell.com"

#define PLINK_PROGRAM "d:\\plink.exe"

#define THROW_ERROR(x) \
	{ \
		throw std::runtime_error(x); \
	}
#endif

// Comment out following line to use plink instead
// #define USE_CHILKAT 1
