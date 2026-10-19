#pragma once

#include "vk/gfx_vk_api.h"

#include "common/com_types.h"

struct GfxGlobals
{
	VkClearColorValue clearColor
	{
		.float32 = { 1.f, 1.f, 1.f, 1.f },
	};
};

extern GfxGlobals g_glob;