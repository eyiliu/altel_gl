add_executable(TelescopeGL_main TelescopeGL_main.cxx)
target_link_libraries(TelescopeGL_main PRIVATE mycommon altel-gl altel-rbcp)

add_executable(telescope_main telescope_main.cc)
target_link_libraries(telescope_main PRIVATE mycommon altel-gl altel-rbcp)

install(TARGETS TelescopeGL_main telescope_main
  EXPORT ${PROJECT_NAME}Targets
  RUNTIME       DESTINATION bin      COMPONENT runtime
  LIBRARY       DESTINATION lib      COMPONENT runtime
  ARCHIVE       DESTINATION lib      COMPONENT devel
  PUBLIC_HEADER DESTINATION include  COMPONENT devel
  RESOURCE      DESTINATION resource COMPONENT runtime
  )


