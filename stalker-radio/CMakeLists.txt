cmake_minimum_required(VERSION 3.20)
project(stalker-radio VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

include_directories(${GTK4_INCLUDE_DIRS})
link_directories(${GTK4_LIBRARY_DIRS})
add_definitions(${GTK4_CFLAGS_OTHER})

# Silence GTK4 deprecation warnings — gtk_style_context_add_provider is
# deprecated in GTK 4.10 at the global level but fine per-widget.
add_compile_options(-Wno-deprecated-declarations)

add_executable(stalker-radio src/main.cpp)
target_link_libraries(stalker-radio ${GTK4_LIBRARIES})

install(TARGETS stalker-radio DESTINATION bin)
install(FILES assets/stalker-radio.desktop DESTINATION share/applications)
install(FILES assets/stalker-radio.png DESTINATION share/icons/hicolor/256x256/apps OPTIONAL)
