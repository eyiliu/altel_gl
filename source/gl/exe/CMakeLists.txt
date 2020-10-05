add_executable(AltelGlExample AltelGlExample.cxx)
target_link_libraries(AltelGlExample PRIVATE mycommon altel-gl)
set_target_properties(AltelGlExample PROPERTIES CXX_STANDARD 17)

add_executable(TelGL_test TelGL_test.cxx)
target_link_libraries(TelGL_test PRIVATE mycommon altel-telgl glfw)
set_target_properties(TelGL_test PROPERTIES CXX_STANDARD 17)

install(TARGETS AltelGlExample TelGL_test
  EXPORT ${PROJECT_NAME}Targets
  RUNTIME       DESTINATION bin      COMPONENT runtime
  LIBRARY       DESTINATION lib      COMPONENT runtime
  ARCHIVE       DESTINATION lib      COMPONENT devel
  PUBLIC_HEADER DESTINATION include  COMPONENT devel
  RESOURCE      DESTINATION resource COMPONENT runtime
  )
