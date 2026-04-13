cmake_minimum_required(VERSION 3.16)
project(av_controller)

# 1. Compiler Warnings (Strict formatting from the template)
if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
  add_compile_options(-Wall -Wextra -Werror=return-type -Werror=unused-but-set-variable)
endif()

find_package(ament_cmake REQUIRED)

# 2. Organize dependencies in a list to keep the code clean
set(THIS_PACKAGE_INCLUDE_DEPENDS
  controller_interface
  geometry_msgs
  pluginlib
  rclcpp
  rclcpp_lifecycle
)

# Automatically loop through and find all dependencies in the list
foreach(Dependency IN ITEMS ${THIS_PACKAGE_INCLUDE_DEPENDS})
  find_package(${Dependency} REQUIRED)
endforeach()

# 3. Create the shared library for your skid-steer controller
add_library(av_skid_steer_controller SHARED
  src/av_skid_steer_controller.cpp
)

# 4. Enforce modern C++ standards (C++17 is the standard for ROS 2)
target_compile_features(av_skid_steer_controller PUBLIC cxx_std_17)

# 5. Advanced Include Directories (Using Generator Expressions)
target_include_directories(av_skid_steer_controller PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/av_control>
)

# Link the dependencies to the library
ament_target_dependencies(av_skid_steer_controller PUBLIC ${THIS_PACKAGE_INCLUDE_DEPENDS})

# 6. Export the Plugin XML
pluginlib_export_plugin_description_file(controller_interface av_control_plugins.xml)

# 7. Install configurations
install(
  DIRECTORY include/
  DESTINATION include/av_control
)

install(TARGETS av_skid_steer_controller
  EXPORT export_av_control
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

install(FILES av_control_plugins.xml
  DESTINATION share/${PROJECT_NAME}
)

# 8. Export targets so other packages can depend on your controller later
ament_export_targets(export_av_control HAS_LIBRARY_TARGET)
ament_export_dependencies(${THIS_PACKAGE_INCLUDE_DEPENDS})

ament_package()