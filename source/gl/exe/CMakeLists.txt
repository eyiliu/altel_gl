add_executable(AltelGlExample AltelGlExample.cxx)
target_link_libraries(AltelGlExample PRIVATE mycommon altel-gl)
set_target_properties(AltelGlExample PROPERTIES CXX_STANDARD 17)

install(TARGETS AltelGlExample
  EXPORT ${PROJECT_NAME}Targets
  RUNTIME       DESTINATION bin      COMPONENT runtime
  LIBRARY       DESTINATION lib      COMPONENT runtime
  ARCHIVE       DESTINATION lib      COMPONENT devel
  PUBLIC_HEADER DESTINATION include  COMPONENT devel
  RESOURCE      DESTINATION resource COMPONENT runtime
  )
