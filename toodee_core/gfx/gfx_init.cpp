#include "common/com_debug.h"
#include "common/com_types.h"
#include "common/com_utils.h"
#include "gfx_init.h"
#include "gfx_system.h"

#include <format>
#include <vector>
#include <iostream>

#include <vulkan/vulkan.h>

void TestApp()
{
	std::cout << std::format( "hello world\n" );
	GfxSystem &gfxSystem = GfxSystem::Inst();
	//vkCreateInstance( nullptr, nullptr, nullptr );
}