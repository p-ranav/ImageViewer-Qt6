cmake_minimum_required(VERSION 3.5)

project(HelloWorldQt)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Enable all compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

# Enable optimization level -O3
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

# Add the include directory to the project
include_directories(src)

# Add your resource file
set(RESOURCES
    src/resources.qrc
)

# Generate rules for building source files from the resources
qt6_add_resources(RESOURCE_FILES ${RESOURCES})

add_executable(${PROJECT_NAME} src/main.cpp src/main_window.cpp src/image_loader.cpp src/image_viewer.cpp ${RESOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${LibRaw_INCLUDE_DIRS} ${LibExif_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LibRaw_LIBRARIES} ${LibExif_LIBRARIES} Qt6::Core Qt6::Widgets )

set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
)