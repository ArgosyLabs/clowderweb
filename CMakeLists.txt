cmake_minimum_required(VERSION 3.1)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
		STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
		"Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

project(clowderweb)
add_executable(clowderweb main.cc)
target_link_libraries(clowderweb ${CMAKE_DL_LIBS})
install(TARGETS clowderweb RUNTIME DESTINATION bin)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../civetweb/include
	${CMAKE_CURRENT_SOURCE_DIR}/../build_dir/include/
)

foreach(library z ssl crypto pthread)
	find_library(${library}_LIBRARY ${library})
	if (${library}_LIBRARY)
		target_link_libraries(clowderweb ${${library}_LIBRARY})
	endif()
endforeach(library)

add_library(libclowderweb SHARED handler.cc)
set_target_properties(libclowderweb PROPERTIES PREFIX "")
install(TARGETS libclowderweb LIBRARY DESTINATION lib)

foreach(header handler.h)
	install(FILES ${header} DESTINATION "include/clowderweb")
endforeach(header)

target_link_libraries(clowderweb ${CMAKE_CURRENT_SOURCE_DIR}/../civetweb/libcivetweb.a)
target_link_libraries(clowderweb pthread)


foreach(module system_info norobots)
	add_library(${module} SHARED ${module}.cc )
	set_target_properties(${module} PROPERTIES PREFIX "")
	target_link_libraries(${module} libclowderweb)
	install(TARGETS ${module} LIBRARY DESTINATION lib/clowderweb)
endforeach(module)

#
