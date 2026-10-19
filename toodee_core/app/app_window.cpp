#include "app_window.h"
#include "app_rgfw.h"

#include "common/com_debug.h"
#include "common/com_logger.h"

AppWindow::AppWindow( const char *const appName )
{
	RGFW_init( "Default", RGFW_initVulkan );
	m_window = RGFW_createWindow( appName, 0, 0, 1260, 900, RGFW_windowCenter );
}


AppWindow::~AppWindow()
{
	RGFW_window_close( m_window );
	RGFW_deinit();
}


bool AppWindow::IsClosing() const
{
	return RGFW_window_shouldClose( m_window );
}


void AppWindow::Close()
{
	RGFW_window_setShouldClose( m_window, true );
}


uvec2 AppWindow::Dims() const
{
	int w = -1;
	int h = -1;

	const RGFW_bool success = RGFW_window_getSizeInPixels( m_window, &w, &h );
	Com_Assert_Fatal( success );

	return { static_cast<uint>( w ), static_cast<uint>( h ) };
}