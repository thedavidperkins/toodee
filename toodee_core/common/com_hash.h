#pragma once

#include "com_types.h"
#include <functional>

class HashBuilder
{
public:
	template <typename T>
	HashBuilder( T init )
	{
		m_hash = std::hash<T>{}( init );
	}

	template <typename T>
	HashBuilder &Build( const T &val )
	{
		const size_t valHash = std::hash<T>{}( val );
		m_hash = valHash ^ ( m_hash << 1u );
		return *this;
	}

	size_t Get() const { return m_hash; }
	operator size_t() const { return m_hash; }

private:
	size_t m_hash = 0u;
};