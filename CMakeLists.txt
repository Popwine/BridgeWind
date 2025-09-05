
cmake_minimum_required(VERSION 3.18)
project(BridgeWind LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
else()
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
# 首先，为 Debug 版本设置路径
if(CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_CONFIGURATION_TYPES MATCHES "Debug")
    list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/libdxfrw-install") # 指向你的 Debug 版本
    list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/VTK-9.4.2-install-debug/lib/cmake/vtk-9.4")
    list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/hdf5-1.14.4-3-install")
    list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/CGNS-4.4.0-install")
endif()

# 然后，为 Release 版本设置路径
if(CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_CONFIGURATION_TYPES MATCHES "Release")
    list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/libdxfrw-install-release") # 指向你刚刚创建的 Release 版本
    list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/VTK-9.4.2-install-release/lib/cmake/vtk-9.4")
    list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/hdf5-1.14.4-3-install-release")
    list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/CGNS-4.4.0-install-release")
endif()





#set(Qt5_DIR "./3rdPartyInstall/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5")
list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/Qt/Qt5.14.2/5.14.2/msvc2017_64")
set(Qt5_DIR "./3rdPartyInstall/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5")
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


find_package(VTK COMPONENTS
    # --- 核心数据结构与算法 ---
    CommonCore          # 基础核心库，例如 vtkObject
    CommonDataModel     # 数据对象，如 vtkUnstructuredGrid, vtkPolyData, vtkPoints. [1]
    CommonExecutionModel # 执行管线模型
    FiltersCore         # 基础滤波器，vtkContourFilter 依赖它. [2]
    FiltersGeneral      # 通用滤波器

    # --- 渲染与交互 ---
    RenderingCore       # 渲染核心，包含 vtkActor, vtkRenderer, vtkLookupTable 等. [3, 4]
    RenderingOpenGL2    # OpenGL2 渲染后端 (您已包含)
    GUISupportQt        # Qt GUI 支持 (您已包含)
    InteractionStyle    # 交互样式 (您已包含)
    RenderingAnnotation # 用于注释，例如 vtkScalarBarActor. [5]

    # --- 可选，但建议保留 ---
    IOXML               # 用于读写 XML 格式的 vtk 文件，调试时可能有用
    IOCGNSReader        # 您已包含，用于读取 CGNS 文件
REQUIRED)

# 这一行让CMake在处理Qt的 .h 文件时能自动调用 moc, uic, rcc 等工具
# 这对于使用信号和槽、.ui文件、资源文件至关重要
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# 查找Qt5的包。我们需要 Core 和 Widgets 模块来创建最基本的GUI应用。
# REQUIRED 意味着如果找不到Qt5，CMake会报错并停止。
find_package(Qt5 COMPONENTS Core Widgets Svg REQUIRED)
find_package(Qt5 COMPONENTS LinguistTools REQUIRED)
message(STATUS "CMake found windeployqt at: ${Qt5_WINDEPLOYQT_EXECUTABLE}")
set(TS_FILES "${CMAKE_SOURCE_DIR}/translations/bridgewind_zh_CN.ts")

include_directories(
    src
    ${Qt5Widgets_INCLUDE_DIRS} # 这是 find_package 帮我们找到的Qt头文件路径
)

message(STATUS "Found HDF5: ${HDF5_FOUND}, Version: ${HDF5_VERSION_STRING}")
message(STATUS "Found CGNS: ${CGNS_FOUND}, Version: ${CGNS_VERSION_STRING}")
message(STATUS "Found libdxfrw: ${libdxfrw_FOUND}, Version: ${libdxfrw_VERSION_STRING}")
add_subdirectory(src)
