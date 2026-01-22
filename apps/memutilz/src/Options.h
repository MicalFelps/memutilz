#ifndef OPTIONS_H
#define OPTIONS_H

#include <qcommandlineoption.h>

namespace Options {
	inline const QCommandLineOption verboseOption{ {"v", "verbose"} };
	inline const QCommandLineOption pidOption{ {"p", "pid"}, "Process ID" };
	inline const QCommandLineOption fileOption{ {"f", "file"}, "Input File" };
}

#endif