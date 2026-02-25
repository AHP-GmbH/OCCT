message(STATUS ">>> OcctRestServer: add_executable invoked")

add_executable(OcctRestServer
        main.cxx
        RestServer.cxx
        Endpoints.cxx
)
target_link_libraries(OcctRestServer PRIVATE
        TKernel
        TKMath
        TKGeomBase
        TKService
)
set_property(TARGET OcctRestServer PROPERTY EXCLUDE_FROM_ALL OFF)
