add_executable(demo-macros demo-macros.c)
target_link_libraries(demo-macros PUBLIC clibrary-c)

add_executable(demo-log demo-log.c)
target_link_libraries(demo-log PUBLIC clibrary-c)

add_executable(demo-glog demo-glog.c)
target_link_libraries(demo-glog PUBLIC clibrary-glib ${GLIB_LIBRARIES})
target_include_directories(demo-glog PUBLIC ${GLIB_INCLUDE_DIRS})

add_executable(demo-qlog demo-qlog.cpp)
target_link_libraries(demo-qlog PUBLIC clibrary-qt5 ${GLIB_LIBRARIES})
target_include_directories(demo-qlog PUBLIC ${GLIB_INCLUDE_DIRS})
