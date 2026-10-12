#pragma once

#include "app/app_rgfw.h"
#include "app/app_window.h"
#include <functional>

using EventLoopCB = std::function<bool( RGFW_window *, const RGFW_event & )>;

class ApplicationBase
{
public:
	ApplicationBase();
	ApplicationBase( const ApplicationBase &rhs ) = delete;
	ApplicationBase &operator=( const ApplicationBase &rhs ) = delete;
	ApplicationBase( ApplicationBase &&rhs ) = delete;
	ApplicationBase &operator=( ApplicationBase &&rhs ) = delete;

	virtual void Run( EventLoopCB cb );

protected:
	virtual const char *AppName() const { return "Default App"; }

	AppWindow m_mainWindow;
};