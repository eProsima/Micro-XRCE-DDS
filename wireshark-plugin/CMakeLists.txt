include(WiresharkPlugin)

set_module_info(dds-xrce 0 0 4 0)

set(DISSECTOR_SRC
	dds-xrce.c
)

set(PLUGIN_FILES
	plugin.c
	${DISSECTOR_SRC}
)

set_source_files_properties(
	${PLUGIN_FILES}
	PROPERTIES
	COMPILE_FLAGS "${WERROR_COMMON_FLAGS}"
)

register_plugin_files(plugin.c
	plugin
	${DISSECTOR_SRC}
)

add_plugin_library(dds-xrce epan)
target_link_libraries(dds-xrce epan)

# target_include_directories(dds-xrce PRIVATE Micro-XRCE-DDS-Client/include)
# target_include_directories(dds-xrce PRIVATE Micro-CDR/include)

install_plugin(dds-xrce epan)

file(GLOB DISSECTOR_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
CHECKAPI(
	NAME
	  dds-xrce
	SWITCHES
	  --group dissectors-prohibited
	  --group dissectors-restricted
	SOURCES
	  ${DISSECTOR_SRC}
	  ${DISSECTOR_HEADERS}
)
