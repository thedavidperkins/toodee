add_library(TooDeeCore)

include(${CMAKE_SOURCE_DIR}/common.cmake)

set(header_list
  common/com_debug.h
  common/com_logger.h
  common/com_types.h
  common/com_utils.h
  gfx/gfx_system.h
  gfx/gfx_init.h
  gfx/vk/gfx_vk_api.h
  gfx/vk/gfx_vk_funcs.h
  gfx/vk/gfx_vk_struct_factory.h
)

set(source_list
  common/com_debug.cpp
  gfx/gfx_system.cpp
  gfx/gfx_init.cpp
  gfx/vk/gfx_vk_funcs.cpp
)

set(inline_list
  gfx/vk/gfx_vk_funcs.inl
)

target_sources(TooDeeCore
  PRIVATE
    ${source_list}

  PUBLIC
    FILE_SET HEADERS
    FILES
      ${header_list}
      ${inline_list}
)
set_filters(${header_list} ${source_list} ${inline_list})

set(WINDIR $ENV{windir})
string(REGEX REPLACE "\\\\" "/" WINDIR ${WINDIR})

target_include_directories(TooDeeCore PUBLIC $ENV{VK_SDK_PATH}/include/)
target_compile_features(TooDeeCore PRIVATE cxx_std_23)
target_compile_options(TooDeeCore PRIVATE /W3)
target_compile_definitions(TooDeeCore PRIVATE VULKAN_DLL=\"${WINDIR}/System32/vulkan-1.dll\")
target_link_libraries(TooDeeCore PRIVATE TooDeeProjConfigs)