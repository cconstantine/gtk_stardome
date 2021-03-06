# Set the name and the supported language of the project
project(grab_gtk)

# Set the minimum version of cmake required to build this project
cmake_minimum_required(VERSION 2.6)
# Use the package PkgConfig to detect GTK+ headers/library files
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
include_directories(${GTK3_INCLUDE_DIRS})


include_directories(/opt/vc/include)
include_directories(/opt/vc/include/interface/vcos/pthreads)
include_directories(/opt/vc/include/interface/vmcs_host)
include_directories(/opt/vc/include/interface/vmcs_host/linux)

link_directories(${GTK3_LIBRARY_DIRS})
link_directories(/opt/vc/lib)

# Add other flags to the compiler
add_definitions(${GTK3_CFLAGS_OTHER})
# Add an executable compiled from hello.c
add_executable(grab_gtk grab_gtk.cpp LedMatrix.cpp)

# Link the target to the GTK+ libraries
target_link_libraries(grab_gtk ${GTK3_LIBRARIES} bcm_host)
