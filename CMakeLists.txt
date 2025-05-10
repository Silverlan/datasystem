include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME datasystem)
pr_add_library(${PROJ_NAME} SHARED)

pr_add_dependency(${PROJ_NAME} vfilesystem TARGET PUBLIC)
pr_add_external_dependency(${PROJ_NAME} exprtk HEADER_ONLY)

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")

pr_add_compile_definitions(
	${PROJ_NAME}
		-DDATASYSTEM_DLL
)

if(MSVC)
	target_compile_options(${PROJ_NAME} PRIVATE /bigobj)
endif()

pr_finalize(${PROJ_NAME})
