#pragma once

#include "vk/gfx_vk_api.h"
#include "common/com_types.h"

class GfxCmdBfr;

class GfxPipeline
{
public:
	GfxPipeline() = default;
	GfxPipeline( const GfxPipeline &rhs ) = delete;
	GfxPipeline &operator=( const GfxPipeline &rhs ) = delete;
	GfxPipeline( GfxPipeline &&rhs );
	GfxPipeline &operator=( GfxPipeline &&rhs );

	~GfxPipeline() = default;

	void Init( VkDevice device, VkShaderModule vs, VkShaderModule fs, uvec2 viewSize, VkFormat rtColorFmt );
	void Finalize();

private:

	//VkPipelineLayout m_layout = VK_NULL_HANDLE;
	VkPipeline m_pso = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;

	friend class GfxCmdBfr;
};