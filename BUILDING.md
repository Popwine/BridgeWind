# BridgeWind 编译指南 (Build Guide)

本文档为希望从源代码编译 BridgeWind 的开发者提供了详细的步骤和指导。此流程涉及手动从源码编译多个依赖库，并可能会手动修改项目中的 `CMakeLists.txt` 文件。

本项目的编译流程自动化程度不足，依赖很多手动操作。我们正在积极寻求改进构建系统的方案。

您可以下载已编译好的项目文件夹，并直接使用：

https://pan.baidu.com/s/1EKRSHdYOXqWORBE4fFz8ig?pwd=7ngu
提取码: 7ngu

## 1. 核心开发工具

请首先确保您的开发环境中已安装以下核心工具：

-   **Visual Studio 2019** 或更高版本。安装时除默认选项外请勾选“使用C++的桌面开发”、“Windos 11 SDK”、“Windos 10 SDK”。 https://visualstudio.microsoft.com/
-   **CMake**: 版本 **3.15** 或更高。请确保已安装 `cmake-gui` 组件，并已将CMake添加到系统的 `PATH` 环境变量中。 https://github.com/Kitware/CMake/releases/download/v4.1.2/cmake-4.1.2-windows-x86_64.msi
-   **Git**: https://github.com/git-for-windows/git/releases/download/v2.51.0.windows.2/Git-2.51.0.2-64-bit.exe
-   **MS-MPI**: 下载时请勾选"msmpisdk.msi", "msmpisetup.exe"两个文件并安装。 https://www.microsoft.com/en-us/download/details.aspx?id=57467
-   **Gmsh**: https://gmsh.info/bin/Windows/gmsh-4.14.1-Windows64.zip
-   **Tecplot 360**: https://tecplot.com/products/tecplot-360/ 或者仅获取`tecio.dll`即可

## 2. 克隆桥风智绘（BridgeWind）仓库
1.  在您希望存放BrideWind源码的地方右键打开PowerShell，输入以下命令：
    ```
    git clone https://osredm.com/p70941386/BridgeWind
    ```
2. 克隆后，当前目录会出现`BridgeWind`文件夹，其中有BridgeWind完整的源码，但不包括第三方库的源码。关于第三方库的源码我们需要逐个获得并编译。后面提到的`BridgeWind`文件夹即此路径。
3. 在BridgeWind文件夹下新建一个"3rdPartyInstall"文件夹，用于存放**已编译好的**第三方库的二进制文件。

## 3. 编译依赖库 (从源码)

这是最关键且最耗时的部分。您需要依次下载并编译依赖库。我们推荐把所有依赖库安装到`BridgeWind/3rdPartyInstall`文件夹下，这样不需要手动编码`CMakeLists.txt`中的依赖路径。
 
如果您不熟悉使用`CMake GUI`和`Visual Studio`的编译流程，请先阅读以下内容：

---



### **使用 CMake GUI & Visual Studio 的通用编译流程**

对于这五个依赖：
```
HDF5
CGNS
VTK
libdxfrw
PhengLEI
```
请遵循此通用流程：

1.  **下载并解压源码**到您任意的位置，链接将在后面给出。
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
    -   在 CMake GUI 的变量列表中，找到 `CMAKE_INSTALL_PREFIX`。将其值设置为 `BridgeWind/3rdPartyInstall`。
    -   根据下面每个库的具体说明，设置其他必要的变量（例如 `BUILD_SHARED_LIBS` 或指向其他依赖的 `..._DIR` 路径）。
7.  **再次配置与生成**:
    -   再次点击 **"Configure"**。所有红色高亮应该会消失。
    -   点击 **"Generate"**，CMake 将在 `build` 目录下生成 Visual Studio 解决方案 (`.sln`) 文件。
8.  **在 Visual Studio 中编译与安装**:
    -   点击 **"Open Project"** 或手动打开 `build` 目录中的 `.sln` 文件。
    -   将解决方案配置从 "Debug" 更改为 **"Release"**。
    -   在“解决方案资源管理器”中，右键点击 **`ALL_BUILD`** 项目并选择“生成”。
    -   编译成功后，右键点击 **`INSTALL`** 项目并选择“生成”。这会将编译好的文件（头文件、库、DLL等）整齐地复制到您在 `CMAKE_INSTALL_PREFIX` 中指定的目录。

---

### **逐个编译依赖：**

### **3.1 HDF5 (CGNS 的依赖)**

-   **推荐版本**: 1.14.4-3
-   **源码下载**: [HDF5 官网](https://www.hdfgroup.org/downloads/hdf5/source-code/)
(https://www.hdfgroup.org/downloads/hdf5/source-code/)
-   **编译说明**: 遵循上述“通用编译流程”。
    -   **CMake 变量设置**:
        -   `CMAKE_INSTALL_PREFIX`: `BridgeWind-deps/3rdPartyInstall/hdf5-install-release`
        -   `BUILD_SHARED_LIBS`: 勾选 (ON)

---

### **3.2 CGNS**

-   **推荐版本**: 4.4.0
-   **源码下载**: [CGNS GitHub Releases](https://github.com/CGNS/CGNS/releases)
(https://github.com/CGNS/CGNS/releases)
-   **编译说明**: 遵循上述“通用编译流程”。
    -   **CMake 变量设置**:
        -   `CMAKE_INSTALL_PREFIX`: `BridgeWind/3rdPartyInstall/CGNS-install-release`
        -   `BUILD_SHARED_LIBS`: 勾选 (ON)
        -   `CGNS_ENABLE_HDF5`: 勾选 (ON)
        -   `HDF5_DIR`: 设置为您之前 HDF5 的安装路径，指向包含 `HDF5Config.cmake` 的目录，即 `BridgeWind\3rdPartyInstall\hdf5-install-release\cmake`。

---

### **3.3 Qt 5 (使用安装包)**

对于Qt，不需要手动编译。使用官方安装包是最高效的方式。

-   **推荐版本**: 5.14.2
-   **下载链接**: [qt-opensource-windows-x86-5.14.2.exe](https://download.qt.io/archive/qt/5.14/5.14.2/qt-opensource-windows-x86-5.14.2.exe)
(https://download.qt.io/archive/qt/5.14/5.14.2/qt-opensource-windows-x86-5.14.2.exe)
-   **安装说明**:
    1.  运行安装程序。
    2.  在 "Select Components" 步骤中，勾选与您的编译器匹配的组件（例如 `MSVC 2019 64-bit`）。
    3.  记下安装路径，例如 `C:\Qt`。
    4.  将整个Qt文件夹复制到`BridgeWind/3rdPartyInstall/`文件夹下。推荐的复制后的目录应该为：
        ```
        BridgeWind
        ├── .git
        ├── 3rdPartyInstall
        │   ├── CGNS-install-release
        │   ├── hdf5-install-release
        │   └── Qt
        │       └── Qt5.14.2
        │           ├── 5.14.2
        │           │   └── msvc2017_64
        │           │       ├── lib
        │           │       │   └── cmake
        │           │       └── ...
        │           ├── dist
        │           ├── Docs
        │           └── ...
        ├── images
        ├── res
        ├── src
        └── translations
        ```

---

### **3.4 VTK**

-   **推荐版本**: 9.4.2
-   **源码下载**: [VTK 官网](https://vtk.org/download/)
(https://vtk.org/download/)
-   **编译说明**: 请确保已经编译了Qt。请遵循上述“通用编译流程”。
    -   **CMake 变量设置**:
        -   `CMAKE_INSTALL_PREFIX`: `BridgeWind\3rdPartyInstall\VTK-9.4.2-install-release`
        -   `BUILD_SHARED_LIBS`: 勾选 (ON)
        -   `VTK_GROUP_ENABLE_Qt`: 设置为 `YES`
        -   `VTK_QT_VERSION`: 设置为 `5`
        -   `Qt5_DIR`: 设置为您 Qt5 的安装路径，即 `BridgeWind/3rdPartyInstall/Qt/Qt5.14.2/5.14.2/msvc2019_64/lib/cmake/Qt5`。

---

### **3.5 libdxfrw**

-   **推荐版本**: 0.6.3
-   **下载链接**: [libdxfrw SourceForge](https://sourceforge.net/projects/libdxfrw/files/libdxfrw-0.6.3.tar.bz2/download)
(https://sourceforge.net/projects/libdxfrw/files/libdxfrw-0.6.3.tar.bz2/download)；
或使用git克隆：
```
git clone https://github.com/codelibs/libdxfrw.git
```
-   **编译说明**: 遵循上述“通用编译流程”。
    -   **CMake 变量设置**:
        -   `CMAKE_INSTALL_PREFIX`: `BridgeWind/3rdPartyInstall/libdxfrw-install-release`

---

### **3.6 风雷 (PHengLEI)**

-   **代码库地址**: [https://www.osredm.com/PHengLEI/PHengLEI](https://www.osredm.com/PHengLEI/PHengLEI)
-   请按照官方教程编译风雷。[查看官方教程](https://www.bilibili.com/video/BV1eX4y1T7yW)
(https://www.bilibili.com/video/BV1eX4y1T7yW)





## 4. 编译并运行 BridgeWind

我们推荐使用 Visual Studio 的 **"Open a local folder"** 功能来处理本项目。

**1. 在 Visual Studio 中打开项目**

1.  启动 Visual Studio。
2.  选择 **"Continue without code"** -> "File" -> "Open" -> "Folder..."。
3.  浏览并选择 `BridgeWind` 仓库的根目录。
4.  Visual Studio 将自动检测到 `CMakeLists.txt` 并开始配置项目。由于您已在文件中硬编码了路径，配置过程应该会成功。
   
**2. 手动修改 `src/CMakeLists.txt`**

1. 若您的目录设置完全按照上述步骤设置，您可以忽略此小节。

2. 若您的第三方库的安装目录未能完全严格按照上述步骤来设置，则您需要**直接修改** `BridgeWind/src/CMakeLists.txt` 文件，将依赖库的路径硬编码进去。

3. 打开 `BridgeWind/src/CMakeLists.txt`，在文件顶部或 `find_package` 命令之前，找到以下内容，并**确保路径与您自己的安装路径完全一致**。

    ```cmake
    # Release 版本设置路径
    if(CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_CONFIGURATION_TYPES MATCHES "Release")
        list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/libdxfrw-install-release") # 指向你刚刚创建的 Release 版本
        list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/VTK-9.4.2-install-release/lib/cmake/vtk-9.4")
        list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/hdf5-install-release")
        list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/CGNS-install-release")
    endif()

    # Qt路径
    list(APPEND CMAKE_PREFIX_PATH "./3rdPartyInstall/Qt/Qt5.14.2/5.14.2/msvc2017_64")
    set(Qt5_DIR "./3rdPartyInstall/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5")
    ```




**3. 编译BridgeWind**

1.  在 Visual Studio 的“解决方案资源管理器”中，切换到 **CMake Targets View**。
2.  找到 `BridgeWindApp.exe` 目标。
3.  右键点击 `BridgeWindApp.exe` 并选择 **"Build"**。
4.  构建成功后，可执行文件将位于VS自动生成的构建目录中。即 `BridgeWind/out/build/x64-Release/src/BridgeWindApp.exe`。


**4. 配置依赖项**

1. 请复制编译好的风雷可执行文件`PHengLEIv3d0.exe`复制到`BridgeWind\res\PHengLEI_template`目录中。
2. 请将Gmsh安装目录中的`Gmsh.exe`复制到`BridgeWind\res\PHengLEI_template`目录中。
3. 请您找到Tecplot安装目录中的`tecio.dll`文件（通常在`C:\Program Files\Tecplot\Tecplot 360 EX 2022 R1\bin`），并同样复制到`BridgeWind\res\PHengLEI_template`。
4. 请将以下全部内容复制到`BridgeWind/out/build/x64-Release/`文件夹下：
   1. `BridgeWind\res\PHengLEI_template`整个文件夹；
   2. `BridgeWind\3rdPartyInstall\hdf5-install-release\bin`中的`hdf5.dll`；
   3. `BridgeWind\3rdPartyInstall\CGNS-install-release\bin`中的`cgnsdll.dll`；
   4. `BridgeWind\3rdPartyInstall\VTK-9.4.2-install-release\bin`文件夹下的全部文件。
5. 使用`windeployqt`解析BridgeWindApp.exe并为其配置依赖：
   1. 在开始菜单中找到`Qt 5.14.2 (MSVC 2017 64-bit)`打开，在命令行中输入：
        ```cmd
        cd "您的路径\BridgeWind\out\build\x64-Release\src"
        windeployqt BridgeWindApp.exe
        ```
    2. `windeployqt`命令会自动为您配置qt所需的依赖项。
6. 编译完成并且所有依赖项配置完成后，您的文件夹结构应该与此相同:
    ```
    BridgeWind
    ├── out
    │   └── build
    │       └── x64-Release
    │           └── src
    │               ├── BridgeWindApp_autogen/
    │               ├── BridgeWindCore_autogen/
    │               ├── BridgeWindUI_autogen/
    │               ├── CMakeFiles/
    │               ├── iconengines/
    │               ├── imageformats/
    │               ├── PHengLEI_template/
    │               ├── platforms/
    │               ├── styles/
    │               ├── TestApp_autogen/
    │               ├── translations/
    │               ├── BridgeWindApp.exe
    │               ├── BridgeWindCore.lib
    │               ├── BridgeWindUI.lib
    │               ├── TestApp.exe
    │               ├── bridgewind_de_DE.qm
    │               ├── bridgewind_zh_CN.qm
    │               ├── cgnsdll.dll
    │               ├── hdf5.dll
    │               ├── gmsh.exe
    │               ├── D3Dcompiler_47.dll
    │               ├── libEGL.dll
    │               ├── libGLESv2.dll
    │               ├── opengl32sw.dll
    │               ├── Qt5Core.dll
    │               ├── Qt5Gui.dll
    │               ├── Qt5Svg.dll
    │               ├── Qt5Widgets.dll
    │               ├── vtkcgns-9.4.dll
    │               ├── vtkChartsCore-9.4.dll
    │               ├── vtkCommonColor-9.4.dll
    │               ├── vtkCommonComputationalGeometry-9.4.dll
    │               └── 其他vtk库依赖...
    └── ...
    ```


## **编译完成！**

您可以直接从 Visual Studio 中设置 `BridgeWindApp.exe` 为启动项并运行。

若在编译时出现问题，可以下载已编译好的项目文件夹，并直接使用：

https://pan.baidu.com/s/1EKRSHdYOXqWORBE4fFz8ig?pwd=7ngu
提取码: 7ngu

---

