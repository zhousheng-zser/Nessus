find_package(Threads REQUIRED)

if(NOT DISABLE_LICENSE)
	find_library(G6_LICENSE_LIBRARY
		NAMES license_client
		HINTS ${COMMON_LIBRARY_DIRS})
	
	message("G6_LICENSE_LIBRARY: ${G6_LICENSE_LIBRARY}")
endif()
