#pragma once

#ifdef DEBUG_BUILD
namespace ComAssertInternal
{
void Com_Assert_IMPL( bool condition, bool fatal, const char *conditionText );
}

#define Com_Assert_Warn( condition ) ComAssertInternal::Com_Assert_IMPL( condition, false, #condition )
#define Com_Assert_Fatal( condition ) ComAssertInternal::Com_Assert_IMPL( condition, true, #condition )
#else // #ifdef DEBUG_BUILD
#define Com_Assert_Warn( condition )
#define Com_Assert_Fatal( condition )
#endif // #else // #ifdef