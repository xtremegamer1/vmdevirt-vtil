# This file is automatically generated from cmake.toml - DO NOT EDIT
# See https://github.com/build-cpp/cmkr for more information

# Create a configure-time dependency on cmake.toml to improve IDE support
if(CMKR_ROOT_PROJECT)
	configure_file(cmake.toml cmake.toml COPYONLY)
endif()

# vmemu
set(CMKR_CMAKE_FOLDER ${CMAKE_FOLDER})
if(CMAKE_FOLDER)
	set(CMAKE_FOLDER "${CMAKE_FOLDER}/vmemu")
else()
	set(CMAKE_FOLDER vmemu)
endif()
add_subdirectory(vmemu)
set(CMAKE_FOLDER ${CMKR_CMAKE_FOLDER})

# VTIL-Core
set(CMKR_CMAKE_FOLDER ${CMAKE_FOLDER})
if(CMAKE_FOLDER)
	set(CMAKE_FOLDER "${CMAKE_FOLDER}/VTIL-Core")
else()
	set(CMAKE_FOLDER VTIL-Core)
endif()
add_subdirectory(VTIL-Core)
set(CMAKE_FOLDER ${CMKR_CMAKE_FOLDER})

# asmjit
set(CMKR_CMAKE_FOLDER ${CMAKE_FOLDER})
if(CMAKE_FOLDER)
	set(CMAKE_FOLDER "${CMAKE_FOLDER}/asmjit")
else()
	set(CMAKE_FOLDER asmjit)
endif()
add_subdirectory(asmjit)
set(CMAKE_FOLDER ${CMKR_CMAKE_FOLDER})

