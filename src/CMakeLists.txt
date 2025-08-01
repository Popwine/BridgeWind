# --- 收集所有需要扫描翻译的源文件 ---
# <<< 修改点 1: 创建一个包含所有源文件的列表 >>>
set(TRANSLATION_SOURCES
    # 核心逻辑库的源文件
    vector2d.cpp vector2d.h
    mesh.cpp mesh.h
    gmsh_reader.cpp gmsh_reader.h
    gmsh2cgns.cpp gmsh2cgns.h
    gmsh_generator.cpp gmsh_generator.h
    geometry.cpp geometry.h
    topology_analyzer.cpp topology_analyzer.h
    geo_generator.cpp geo_generator.h
    app_controller.cpp app_controller.h
    hypara_generator.cpp hypara_generator.h
    bridge_simulation_service.cpp bridge_simulation_service.h
    geometry_analysis_service.cpp geometry_analysis_service.h
    meshing_service.cpp meshing_service.h
    solver_service.cpp solver_service.h
    file_utils.cpp file_utils.h
    process_manager.cpp process_manager.h
    flow_data_reader.h flow_data_reader.cpp
    ProjectHistoryManager.h ProjectHistoryManager.cpp
    section_definitions.h
    # UI库的源文件
    mainwindow.cpp mainwindow.h mainwindow.ui
    welcomedialog.cpp welcomedialog.h welcomedialog.ui newprojectdialog.ui
    # 主程序的源文件
    main_gui.cpp
)

# --- 正确调用翻译工具 ---
# <<< 修改点 2: 使用源文件列表来调用 qt5_create_translation >>>
# 注意第二个参数现在是 ${TRANSLATION_SOURCES}，它是一个完整的文件列表
# 我们还需要把主 CMakeLists.txt 中定义的 TS_FILES 传进来
qt5_create_translation(QM_FILES ${TRANSLATION_SOURCES} ${TS_FILES})


# --- 创建一个核心逻辑库 ---
add_library(BridgeWindCore STATIC
    vector2d.cpp vector2d.h
    mesh.cpp mesh.h
    gmsh_reader.cpp gmsh_reader.h
    gmsh2cgns.cpp gmsh2cgns.h
    gmsh_generator.cpp gmsh_generator.h
    geometry.cpp geometry.h
    topology_analyzer.cpp topology_analyzer.h
    geo_generator.cpp geo_generator.h
    app_controller.cpp app_controller.h
    hypara_generator.cpp hypara_generator.h
    bridge_simulation_service.cpp bridge_simulation_service.h
    geometry_analysis_service.cpp geometry_analysis_service.h
    meshing_service.cpp meshing_service.h
    solver_service.cpp solver_service.h
    file_utils.cpp file_utils.h
    process_manager.cpp process_manager.h
    flow_data_reader.h flow_data_reader.cpp
    ProjectHistoryManager.h ProjectHistoryManager.cpp
    section_definitions.h
)

target_include_directories(BridgeWindCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}

)

target_link_libraries(BridgeWindCore PUBLIC
    CGNS::cgns_shared
    libdxfrw::libdxfrw
    Qt5::Core
    hdf5::hdf5-shared
    
)


# --- 创建UI库 ---
add_library(BridgeWindUI STATIC
    mainwindow.cpp mainwindow.h mainwindow.ui
    welcomedialog.cpp welcomedialog.h welcomedialog.ui newprojectdialog.ui
    FileDropWidget.h FileDropWidget.cpp
     
    
)

target_include_directories(BridgeWindUI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_BINARY_DIR}/BridgeWindUI_autogen/include"

)
target_link_libraries(BridgeWindUI PRIVATE dwmapi)

target_link_libraries(BridgeWindUI PUBLIC
    BridgeWindCore
    Qt5::Widgets


    VTK::CommonCore
    VTK::CommonDataModel
    VTK::CommonExecutionModel
    VTK::FiltersCore
    VTK::FiltersGeneral
    VTK::RenderingCore
    VTK::RenderingOpenGL2
    VTK::GUISupportQt
    VTK::InteractionStyle
    VTK::RenderingAnnotation
    VTK::IOXML
    VTK::IOCGNSReader
)



add_executable(BridgeWindApp
    main_gui.cpp
    resources.qrc
    ${QM_FILES}
    
)

# 将UI库链接到可执行文件
target_link_libraries(BridgeWindApp PRIVATE
    BridgeWindUI
)