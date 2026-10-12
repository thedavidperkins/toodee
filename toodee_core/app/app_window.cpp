#include "common/com_logger.h"
#include "app_window.h"
#include "app_rgfw.h"

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