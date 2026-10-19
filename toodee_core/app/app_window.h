#pragma once

#include "common/com_types.h"

struct RGFW_window;

class AppWindow
{
public:
	explicit AppWindow( const char *const appName );
	AppWindow( const AppWindow &rhs ) = delete;
	AppWindow( AppWindow &&rhs ) = delete;
	AppWindow &operator=( const AppWindow &rhs ) = delete;
	AppWindow &operator=( AppWindow &&rhs ) = delete;

	~AppWindow();

	bool IsClosing() const;
	void Close();

	uvec2 Dims() const;

	RGFW_window *Handle() { return m_window; }
private:

	RGFW_window *m_window;
};