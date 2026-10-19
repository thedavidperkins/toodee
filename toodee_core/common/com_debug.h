#pragma once

#pragma warning( disable : 6385 6386 )

#ifdef DEBUG_BUILD
#include "com_types.h"

#include <cstdio>

static constexpr uint kMaxFormattedMsgLength = 1024;

namespace ComAssertInternal
{
void Com_Assert_IMPL( bool condition, bool fatal, const char *conditionText, const char *msgText = nullptr );

template<class... Args>
void Com_Assert_IMPL( bool condition, bool fatal, const char *conditionText, const char *msgFmt, Args&&... args )
{
	char msgFormatted[kMaxFormattedMsgLength];
	std::snprintf( msgFormatted, kMaxFormattedMsgLength, msgFmt, args... );
	Com_Assert_IMPL( condition, fatal, conditionText, msgFormatted );
}
}

#define Com_Assert_Warn( condition ) ComAssertInternal::Com_Assert_IMPL( condition, false, #condition )
#define Com_Assert_Fatal( condition ) ComAssertInternal::Com_Assert_IMPL( condition, true, #condition )
#define Com_AssertMsg_Warn( condition, fmt, ... ) ComAssertInternal::Com_Assert_IMPL( condition, false, #condition, fmt, __VA_ARGS__ )
#define Com_AssertMsg_Fatal( condition, fmt, ... ) ComAssertInternal::Com_Assert_IMPL( condition, true, #condition, fmt, __VA_ARGS__ )

#define TODO Com_AssertMsg_Fatal( false, "Unimplemented TODO at line {}\n", __LINE__ )

#else // #ifdef DEBUG_BUILD
#define Com_Assert_Warn( condition )
#define Com_Assert_Fatal( condition )
#define Com_AssertMsg_Warn( condition, fmt, ... )
#define Com_AssertMsg_Fatal( condition, fmt, ... )

#define TODO
#endif // #else // #ifdef