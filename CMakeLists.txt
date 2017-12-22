cmake_minimum_required (VERSION 2.8)
project (lil_calc)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

set (srcs
	main.cpp
	util.cpp
	ntdll_undoc.cpp
	peb_lookup.cpp
)

set (hdrs
	util.h
	ntdll_undoc.h
	ntdll_types.h
	ntddk.h
	peb_lookup.h
)

IF(WIN32)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows")
ENDIF(WIN32)

add_executable (lil_calc ${hdrs} ${srcs})
