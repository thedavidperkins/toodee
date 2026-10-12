#include "app/app_base.h"
#include "app/app_rgfw.h"

#include <print>
#include <thread>


bool MouseEventCallback( RGFW_window *window, const RGFW_event &event )
{
	switch ( event.button.value )
	{
	case RGFW_mouseLeft:
	{
		int mouseX, mouseY;
		RGFW_window_getMouse( window, &mouseX, &mouseY );
		std::print( "You clicked at x: {}, y: {}\n", mouseX, mouseY );
		break;
	}
	default:
		return false;
	}

	return true;
}


bool KeyEventCallback( RGFW_window *window, const RGFW_event &event )
{
	switch ( event.key.value )
	{
	case RGFW_keyEscape:
	{
		RGFW_window_setShouldClose( window, 1 );
		break;
	}
	default:
		return false;
	}

	return true;
}


bool EventCallback( RGFW_window *window, const RGFW_event &event )
{
	switch ( event.type )
	{
	case RGFW_mouseButtonPressed:
	{
		return MouseEventCallback( window, event );
	}
	case RGFW_keyPressed:
	{
		return KeyEventCallback( window, event );
	}
	default:
		return false;
	}
}


int main()
{
	ApplicationBase app;
	app.Run( EventCallback );
}
