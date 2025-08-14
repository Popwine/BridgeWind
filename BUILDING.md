# BridgeWind 编译指南 (Build Guide)

本文档为希望从源代码编译 BridgeWind 的开发者提供了详细的步骤和指导。此流程涉及手动从源码编译多个依赖库，并直接修改项目中的 `CMakeLists.txt` 文件。请严格按照步骤操作。

## 1. 核心开发工具

请首先确保您的开发环境中已安装以下核心工具：

-   **C++ 编译器**: 需支持 C++17 标准。推荐使用 **Visual Studio 2019** 或更高版本。
-   **CMake**: 版本 **3.15** 或更高。请确保已安装 `cmake-gui` 组件，并已将CMake添加到系统的 `PATH` 环境变量中。

## 2. 编译依赖库 (从源码)

这是最关键且最耗时的部分。您需要依次下载并编译以下库。我们推荐为所有依赖库创建一个统一的安装目录，例如 `C:\dev\BridgeWind-deps`，并将每个库安装到其下的子目录中。

---

### **通用编译流程 (使用 CMake GUI & Visual Studio)**

对于下面列出的每个需要从源码编译的库，请遵循此通用流程：

1.  **下载并解压源码**到您选择的位置。
2.  在源码根目录下，创建一个名为 `build` 的空文件夹。
3.  **启动 CMake GUI** (`cmake-gui.exe`)。
4.  **设置路径**:
    -   "Where is the source code": 指向源码的根目录。
    -   "Where to build the binaries": 指向您创建的 `build` 目录。
5.  **首次配置**:
    -   点击 **"Configure"** 按钮。
    -   在弹出的窗口中，选择与您的 Visual Studio 版本匹配的生成器（例如 "Visual Studio 16 2019"），并确保平台为 `x64`。
    -   点击 "Finish"。CMake 将开始配置，期间可能会出现红色高亮的变量。
6.  **设置变量**:
    -   在 CMake GUI 的变量列表中，找到 `CMAKE_INSTALL_PREFIX`。将其值设置为您规划的安装路径（例如 `C:/dev/BridgeWind-deps/library-name`）。
    -   根据每个库的具体说明，设置其他必要的变量（例如 `BUILD_SHARED_LIBS` 或指向其他依赖的 `..._DIR` 路径）。
7.  **再次配置与生成**:
    -   再次点击 **"Configure"**。所有红色高亮应该会消失。
    -   点击 **"Generate"**，CMake 将在 `build` 目录下生成 Visual Studio 解决方案 (`.sln`) 文件。
8.  **在 Visual Studio 中编译与安装**:
    -   点击 **"Open Project"** 或手动打开 `build` 目录中的 `.sln` 文件。
    -   将解决方案配置从 "Debug" 更改为 **"Release"**。
    -   在“解决方案资源管理器”中，右键点击 **`ALL_BUILD`** 项目并选择“生成”。
    -   编译成功后，右键点击 **`INSTALL`** 项目并选择“生成”。这会将编译好的文件（头文件、库、DLL等）整齐地复制到您在 `CMAKE_INSTALL_PREFIX` 中指定的目录。

---

### **2.1 HDF5 (CGNS 的依赖)**

-   **推荐版本**: 1.12.2
-   **源码下载**: [HDF5 官网](https://www.hdfgroup.org/downloads/hdf5/source-code/)
-   **编译说明**: 遵循上述“通用编译流程”。
    -   **CMake 变量设置**:
        -   `CMAKE_INSTALL_PREFIX`: `C:/dev/BridgeWind-deps/hdf5`
        -   `BUILD_SHARED_LIBS`: 勾选 (ON)

---

### **2.2 CGNS**

-   **推荐版本**: 4.4.0
-   **源码下载**: [CGNS GitHub Releases](https://github.com/CGNS/CGNS/releases)
-   **编译说明**: 遵循上述“通用编译流程”。
    -   **CMake 变量设置**:
        -   `CMAKE_INSTALL_PREFIX`: `C:/dev/BridgeWind-deps/cgns`
        -   `BUILD_SHARED_LIBS`: 勾选 (ON)
        -   `CGNS_ENABLE_HDF5`: 勾选 (ON)
        -   `HDF5_DIR`: 设置为您之前 HDF5 的安装路径，指向包含 `HDF5Config.cmake` 的目录，例如 `C:/dev/BridgeWind-deps/hdf5/share/cmake`。

---

### **2.3 Qt 5 (使用安装包)**

对于 Qt，使用官方安装包是最高效的方式。

-   **推荐版本**: 5.14.2
-   **下载链接**: [qt-opensource-windows-x86-5.14.2.exe](https://download.qt.io/archive/qt/5.14/5.14.2/qt-opensource-windows-x86-5.14.2.exe)
-   **安装说明**:
    1.  运行安装程序。
    2.  在 "Select Components" 步骤中，勾选与您的编译器匹配的组件（例如 `MSVC 2019 64-bit`）。
    3.  记下安装路径，例如 `C:\Qt\5.14.2\msvc2019_64`。

---

### **2.4 VTK**

-   **推荐版本**: 9.2.4
-   **源码下载**: [VTK 官网](https://vtk.org/download/)
-   **编译说明**: 遵循上述“通用编译流程”。
    -   **CMake 变量设置**:
        -   `CMAKE_INSTALL_PREFIX`: `C:/dev/BridgeWind-deps/vtk`
        -   `BUILD_SHARED_LIBS`: 勾选 (ON)
        -   `VTK_GROUP_ENABLE_Qt`: 设置为 `YES`
        -   `VTK_QT_VERSION`: 设置为 `5`
        -   `Qt5_DIR`: 设置为您 Qt 的安装路径，例如 `C:/Qt/5.14.2/msvc2019_64/lib/cmake/Qt5`。

---

### **2.5 libdxfrw**

-   **下载链接**: [libdxfrw SourceForge](https://sourceforge.net/projects/libdxfrw/files/stable/)
-   **编译说明**: 遵循上述“通用编译流程”。
    -   **CMake 变量设置**:
        -   `CMAKE_INSTALL_PREFIX`: `C:/dev/BridgeWind-deps/libdxfrw`

## 3. 配置外部工具 (运行时依赖)

### **3.1 Gmsh**

-   **下载链接**: [Gmsh 官网](https://gmsh.info/)
-   **配置**: 下载Windows二进制包，解压后，将其可执行文件所在目录**添加到系统的 `PATH` 环境变量中**。

### **3.2 PHengLEI**

-   **代码库地址**: [https://www.osredm.com/PHengLEI/PHengLEI](https://www.osredm.com/PHengLEI/PHengLEI)
-   **配置**:
    1.  获取 `PHengLEIv3d0.exe` 及其依赖 (如 `tecio.dll`)。
    2.  在 BridgeWind 项目根目录下，创建 `PHengLEI_template` 文件夹。
    3.  将可执行文件和DLL复制到 `PHengLEI_template`。
    4.  在 `PHengLEI_template` 内创建 `bin` 子文件夹，并将所有 `.hypara` 配置文件模板放入其中。

## 4. 编译并运行 BridgeWind

我们推荐使用 Visual Studio 的 **"Open a local folder"** 功能来处理本项目。

**1. 克隆 BridgeWind 仓库**
```bash
git clone https://osredm.com/p70941386/BridgeWind
# 或者
git clone https://github.com/Popwine/BridgeWind.git
```

**2. 手动修改 `src/CMakeLists.txt`**

这是最关键的一步。由于构建系统不灵活，您需要**直接修改** `src/CMakeLists.txt` 文件，将依赖库的路径硬编码进去。

打开 `src/CMakeLists.txt`，在文件顶部或 `find_package` 命令之前，添加以下内容，并**确保路径与您自己的安装路径完全一致**。

```cmake
# --- 强制指定依赖项路径 ---
# !!! 请将下面的路径替换为您自己的实际安装路径 !!!

set(Qt5_DIR "C:/Qt/5.14.2/msvc2019_64/lib/cmake/Qt5")
set(VTK_DIR "C:/dev/BridgeWind-deps/vtk/lib/cmake/vtk-9.2")
set(CGNS_DIR "C:/dev/BridgeWind-deps/cgns/lib/cmake/CGNS")
set(HDF5_DIR "C:/dev/BridgeWind-deps/hdf5/share/cmake")
set(libdxfrw_DIR "C:/dev/BridgeWind-deps/libdxfrw/cmake") # 假设它的cmake文件在这个路径

# --- 查找包 (这些命令现在会使用上面设置的路径) ---
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
# ... 其他 find_package 命令 ...
```
> **注意**: 您可能需要根据实际情况调整 `target_include_directories` 和 `target_link_libraries`，以确保编译器和链接器能找到所有必需的头文件和 `.lib` 文件。

**3. 在 Visual Studio 中打开项目**

1.  启动 Visual Studio。
2.  选择 **"Continue without code"** -> **"File" -> "Open" -> "Folder..."**。
3.  浏览并选择 `BridgeWind` 仓库的根目录。
4.  Visual Studio 将自动检测到 `CMakeLists.txt` 并开始配置项目。由于您已在文件中硬编码了路径，配置过程应该会成功。

**4. 编译和运行**

1.  在 Visual Studio 的“解决方案资源管理器”中，切换到 **CMake Targets View**。
2.  找到 `BridgeWindApp.exe` 目标。
3.  右键点击 `BridgeWindApp.exe` 并选择 **"Build"**。
4.  构建成功后，可执行文件将位于VS自动生成的构建目录中（例如 `out/build/x64-Release/src/BridgeWindApp.exe`）。
5.  您可以直接从 Visual Studio 中设置 `BridgeWindApp.exe` 为启动项并运行。

---

我们正在积极寻求改进构建系统的方案。如果您对此有经验并愿意做出贡献，我们非常欢迎您提交 **Pull Request**！