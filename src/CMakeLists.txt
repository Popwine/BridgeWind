

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

set(QRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc)

# 将编译后的.qm文件添加到资源文件(.qrc)中
# 这段代码会动态生成 resources.qrc 文件的内容
set(QRC_CONTENT "<!DOCTYPE RCC><RCC version=\"1.0\">\n<qresource prefix=\"/translations\">\n")
foreach(TS_FILE ${TS_FILES})
    # 从.ts文件名推导出.qm文件名
    get_filename_component(TS_FILENAME ${TS_FILE} NAME_WE)
    set(QM_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TS_FILENAME}.qm")
    # 把.qm文件路径加到.qrc内容里
    set(QRC_CONTENT "${QRC_CONTENT}<file alias=\"${TS_FILENAME}.ts\">${QM_FILE}</file>\n")
endforeach()
set(QRC_CONTENT "${QRC_CONTENT}</qresource>\n</RCC>\n")

# 将动态生成的内容写入.qrc文件
file(WRITE ${QRC_FILE} "${QRC_CONTENT}")


# --- 创建一个核心逻辑库 ---
# 它包含了除UI以外的所有源文件
add_library(BridgeWindCore STATIC
    # 把所有相关的.cpp和.h文件都列在这里
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
)

# --- 为核心库统一设置包含目录和链接 ---
# PUBLIC意味着链接了BridgeWindCore的目标也会自动获得这些属性
target_include_directories(BridgeWindCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/autogen/include
)

target_link_libraries(BridgeWindCore PUBLIC
    # 链接所有外部依赖
    CGNS::cgns_shared   # HDF5会作为CGNS的依赖被自动链接
    libdxfrw::libdxfrw
    Qt5::Core           # Service类需要QObject
)


# --- 创建UI库 ---
add_library(BridgeWindUI STATIC
    mainwindow.cpp mainwindow.h mainwindow.ui
)

target_include_directories(BridgeWindUI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(BridgeWindUI PUBLIC
    BridgeWindCore      # UI依赖于核心逻辑
    Qt5::Widgets        # UI需要Widgets
)


# --- 创建最终的可执行文件 ---
add_executable(BridgeWindApp
    main_gui.cpp
)
target_sources(BridgeWindApp PRIVATE
    ${QRC_FILE} # 把资源文件也作为源文件添加
    # ... 其他源文件
)
# 将UI库链接到可执行文件
target_link_libraries(BridgeWindApp PRIVATE
    BridgeWindUI
)