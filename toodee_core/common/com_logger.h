#pragma once

#include "com_types.h"

#include <print>

enum class ComLogLevel : byte
{
	Verbose,
	Info,
	Warning,
	Error,
	Fatal,

	Count
};

namespace ComLogInternal
{
inline const char *LogLevelToString( ComLogLevel logLevel )
{
	switch ( logLevel )
	{
	case ComLogLevel::Verbose:
		return "Verbose";
	case ComLogLevel::Info:
		return "Info";
	case ComLogLevel::Warning:
		return "Warning";
	case ComLogLevel::Error:
		return "Error";
	case ComLogLevel::Fatal:
		return "Fatal";
	}

	return "UNKNOWN";
}
}
#define COM_LOG( logLevel, fmt, ... ) std::print( "[{}]: " fmt, ComLogInternal::LogLevelToString( logLevel ), __VA_ARGS__ )

#define COM_LOG_VERBOSE( fmt, ... ) COM_LOG( ComLogLevel::Verbose, fmt, __VA_ARGS__ )
#define COM_LOG_INFO( fmt, ... ) COM_LOG( ComLogLevel::Info, fmt, __VA_ARGS__ )
#define COM_LOG_WARNING( fmt, ... ) COM_LOG( ComLogLevel::Warning, fmt, __VA_ARGS__ )
#define COM_LOG_ERROR( fmt, ... ) COM_LOG( ComLogLevel::Error, fmt, __VA_ARGS__ )
#define COM_LOG_FATAL( fmt, ... ) COM_LOG( ComLogLevel::Fatal, fmt, __VA_ARGS__ )