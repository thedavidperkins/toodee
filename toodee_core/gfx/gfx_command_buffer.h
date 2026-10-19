#pragma once

#include "gfx_image.h"
#include "vk/gfx_vk_api.h"

#include "common/com_types.h"

class GfxFence;

class GfxCmdBfr
{
public:
	GfxCmdBfr( VkDevice device, VkQueue queue, uint queueFamilyIndex );

	GfxCmdBfr( const GfxCmdBfr &rhs ) = delete;
	GfxCmdBfr( GfxCmdBfr &&rhs ) = delete;
	GfxCmdBfr &operator=( const GfxCmdBfr &rhs ) = delete;
	GfxCmdBfr &operator=( GfxCmdBfr &&rhs ) = delete;

	~GfxCmdBfr();

	void BeginRecording( bool oneTimeUse = true );
	void EndRecording();
	void Submit( GfxFence &fence );
	void EndRecordingAndSubmit( GfxFence &fence )
	{
		EndRecording();
		Submit( fence );
	}

	// commands for recording
	void ClearColorImg( GfxImage &img, VkClearColorValue color );
	void TransitionImgForUsage( GfxImage &image, GfxImgUsage usage );
	void BlitColor( GfxImage &src, GfxImage &dst );

	// manual finalize, but no manual init, because init operations are lazy with individual buffer allocation
	static void FinalizePools();

private:
	VkCommandBuffer m_cmdBfr = VK_NULL_HANDLE;
	VkQueue m_queue = VK_NULL_HANDLE;
	uint m_queueFamilyIdx = ~0u;
	uint m_cmdBfrPoolIdx = ~0u;
	bool m_isRecording = false;
};