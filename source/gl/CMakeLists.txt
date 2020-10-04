add_subdirectory(galogen)
add_subdirectory(glsl)

add_library(altel-gl SHARED src/TelescopeGL.cc)
target_link_libraries(altel-gl
  PRIVATE GL GLEW glut altel-glsl
  PUBLIC sfml-window  sfml-system
  )

set(LIB_PUBLIC_HEADERS include/TelescopeGL.hh)
set_target_properties(altel-gl PROPERTIES PUBLIC_HEADER "${LIB_PUBLIC_HEADERS}")

target_include_directories(altel-gl
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
  )

install(TARGETS altel-gl
  EXPORT ${PROJECT_NAME}Targets
  RUNTIME       DESTINATION bin      COMPONENT runtime
  LIBRARY       DESTINATION lib      COMPONENT runtime
  ARCHIVE       DESTINATION lib      COMPONENT devel
  PUBLIC_HEADER DESTINATION include  COMPONENT devel
  RESOURCE      DESTINATION resource COMPONENT runtime
  )

add_subdirectory(exe)
