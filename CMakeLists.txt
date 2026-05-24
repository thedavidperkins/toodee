cmake_minimum_required(VERSION 4.3)

set(CMAKE_CONFIGURATION_TYPES Release Debug)

project(TooDee)

add_library(TooDeeProjConfigs INTERFACE)
target_compile_definitions(TooDeeProjConfigs INTERFACE 
	$<IF:$<CONFIG:Debug>,DEBUG_BUILD,RELEASE_BUILD>
)

add_subdirectory(toodee_core)
add_subdirectory(toodee_devapp)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "TooDeeDevApp")