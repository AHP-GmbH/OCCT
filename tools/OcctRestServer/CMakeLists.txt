message(STATUS ">>> OcctRestServer: add_executable invoked")

add_executable(OcctRestServer
        main.cxx
        RestServer.cxx
        Endpoints.cxx
        SessionManager.cpp
        Logger.cxx
        Converter.cxx
)
target_include_directories(OcctRestServer PRIVATE
        ${CMAKE_SOURCE_DIR}/src/FoundationClasses/TKernel/Standard
        ${CMAKE_SOURCE_DIR}/src/DataExchange/TKDEGLTF/RWGltf
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(OcctRestServer PRIVATE
        TKernel
        TKMath
        TKGeomBase
        TKService
        TKG3d
        TKG2d
        TKBRep        # <--- für BRepBndLib, TopoDS_Shape
        TKXSBase      # <--- für XSControl_Reader
        TKDESTEP      # <--- für STEPControl_Reader
        TKDEIGES
        TKLCAF
        TKXCAF
        TKMesh
        TK
)
set_property(TARGET OcctRestServer PROPERTY EXCLUDE_FROM_ALL OFF)
