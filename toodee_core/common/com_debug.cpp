#include "com_debug.h"
#include "com_logger.h"

#include <cstdlib>

#ifdef DEBUG_BUILD
#include <stacktrace>

namespace ComAssertInternal
{
void Com_Assert_IMPL( bool condition, bool fatal, const char *conditionText, const char *msgText )
{
	if ( !condition )
	{
		const ComLogLevel logLevel = fatal ? ComLogLevel::Fatal : ComLogLevel::Error;
		const std::stacktrace trace = std::stacktrace::current();
		if ( msgText != nullptr )
		{
			COM_LOG( logLevel, "Assert msg: {}\n", msgText );
		}
		else
		{
			COM_LOG( logLevel, "Asserted condition {} failed.\n", conditionText );
		}
		COM_LOG( logLevel, "Stack trace : \n{}\n", std::to_string( trace ) );
		__debugbreak();
		if ( fatal )
		{
			std::exit( 1 );
		}
	}
}
}
#endif // #ifdef DEBUG_BUILD