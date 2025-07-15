
add_library(vector2d STATIC vector2d.cpp vector2d.h)
target_include_directories(vector2d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(mesh STATIC mesh.cpp mesh.h)
target_include_directories(mesh PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mesh PUBLIC vector2d CGNS::cgns_shared )

add_library(gmsh_reader STATIC gmsh_reader.cpp gmsh_reader.h)
target_include_directories(gmsh_reader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gmsh_reader PUBLIC mesh vector2d)

add_library(gmsh2cgns STATIC gmsh2cgns.cpp gmsh2cgns.h)
target_include_directories(gmsh2cgns PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gmsh2cgns PUBLIC 
    gmsh_reader 
    vector2d 
    mesh 
    CGNS::cgns_shared 
    HDF5::HDF5
)

add_library(gmsh_generator STATIC gmsh_generator.cpp gmsh_generator.h)
target_include_directories(gmsh_generator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(geometry STATIC geometry.cpp geometry.h)
target_include_directories(geometry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(geometry PUBLIC vector2d libdxfrw::libdxfrw )

add_library(topology_analyzer STATIC topology_analyzer.cpp topology_analyzer.h)
target_include_directories(topology_analyzer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(topology_analyzer PUBLIC 
    geometry 
)

add_library(geo_generator STATIC geo_generator.cpp geo_generator.h)
target_include_directories(geo_generator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(geo_generator PUBLIC 
    gmsh_generator 
    geometry 
    topology_analyzer
)

add_library(gmsh_controller STATIC gmsh_controller.cpp gmsh_controller.h)
target_include_directories(gmsh_controller PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


add_executable(BridgeWind
    main.cpp
)

# 将链接目标从 CGNS::CGNS 改为 CGNS 提供的具体目标
target_link_libraries(BridgeWind PRIVATE
    gmsh2cgns
    gmsh_generator
    geometry
    topology_analyzer
    geo_generator
    gmsh_controller
)