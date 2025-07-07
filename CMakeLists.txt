
cmake_minimum_required(VERSION 3.15)
project(Gmsh2CGNS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "C:/DevTools/hdf5-1.14.4-3-install")
list(APPEND CMAKE_PREFIX_PATH "C:/DevTools/CGNS-4.4.0-install")
list(APPEND CMAKE_PREFIX_PATH "C:/DevTools/libdxfrw-install")


# 1. 正常找到 HDF5。执行完这行，目标 HDF5::HDF5 就被定义了。
find_package(HDF5 REQUIRED)

# 2. 【关键步骤】为 HDF5::HDF5 创建一个别名，名字是 CGNS 需要的那个。
#    这样，当 CGNS 的配置文件寻找 hdf5::hdf5-shared 时，就能找到了。
if(TARGET HDF5::HDF5 AND NOT TARGET hdf5::hdf5-shared)
    add_library(hdf5::hdf5-shared ALIAS HDF5::HDF5)
endif()

# 3. 现在再去找 CGNS，它就能找到它所依赖的 HDF5 目标了。
# CGNSConfig.cmake and cgns-config.cmake may not be found in the directory,
# In that case, create file "CGNSConfig.cmake" in path "CGNS install path\lib\cmake\cgns
# This file should contain the following content:
# include("${CMAKE_CURRENT_LIST_DIR}/cgns-targets.cmake")
find_package(CGNS REQUIRED)
find_package(libdxfrw REQUIRED)

message(STATUS "Found HDF5: ${HDF5_FOUND}, Version: ${HDF5_VERSION_STRING}")
message(STATUS "Found CGNS: ${CGNS_FOUND}, Version: ${CGNS_VERSION_STRING}")
message(STATUS "Found libdxfrw: ${libdxfrw_FOUND}, Version: ${libdxfrw_VERSION_STRING}")

add_subdirectory(src)