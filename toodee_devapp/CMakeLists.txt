add_executable(TooDeeDevApp)

include(${CMAKE_SOURCE_DIR}/common.cmake)

set(header_list)
set(source_list
  main.cpp
)

target_sources(TooDeeDevApp
  PRIVATE
    ${source_list}
)
set_filters(${header_list} ${source_list})

target_link_libraries(TooDeeDevApp
  PRIVATE
    TooDeeCore
)

add_custom_command(
	TARGET TooDeeDevApp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_SOURCE_DIR}/vk_layer_settings.txt
			${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/vk_layer_settings.txt
)

target_compile_features(TooDeeDevApp PRIVATE cxx_std_23)
target_compile_options(TooDeeDevApp PRIVATE /W3)
target_link_libraries(TooDeeDevApp PRIVATE TooDeeProjConfigs)