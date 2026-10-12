#include "app_base.h"
#include "gfx/gfx_system.h"

ApplicationBase::ApplicationBase()
	: m_mainWindow( AppName() )
{}

void ApplicationBase::Run( EventLoopCB cb )
{
	GfxSystem &gfx = GfxSystem::Inst();
	gfx.Init( m_mainWindow );

	while ( !m_mainWindow.IsClosing() )
	{
		RGFW_event nextEvent;
		while ( RGFW_checkEvent( &nextEvent ) )
		{
			cb( m_mainWindow.Handle(), nextEvent );
		}

		gfx.RenderFrame();
	}

	gfx.Finalize();
}
