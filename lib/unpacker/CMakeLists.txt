# Copyright 2013-2016 Jan de Cuveland <cmail@cuveland.de>

file(GLOB LIB_SOURCES *.cpp)
file(GLOB LIB_HEADERS *.hpp)

add_library(unpacker ${LIB_SOURCES} ${LIB_HEADERS})

target_include_directories(unpacker PUBLIC .)

target_include_directories(unpacker SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(unpacker
  PUBLIC fles_ipc
  PUBLIC logging
  PUBLIC crcutil
)

if(USE_NUMA AND NUMA_FOUND)
  target_compile_definitions(unpacker PRIVATE HAVE_NUMA)
  target_link_libraries(unpacker PRIVATE ${NUMA_LIBRARY})
endif()

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(unpacker PUBLIC OpenMP::OpenMP_CXX)
endif()