cmake_minimum_required(VERSION 3.16)
project(obs)

# Set Qt installation path
set(CMAKE_PREFIX_PATH "H:/Qt/6.9.0/msvc2022_64")

# Set C++ Standard
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
endif()

include_directories("C:/ffmpeg-dev/include")
link_directories("C:/ffmpeg-dev/lib")

# Add source files
add_executable(obs "src/obsproject.cpp" src/ScreenCapture.cpp src/MainWindow.cpp incl/ScreenCapture.h incl/MainWindow.h  incl/PTR_INFO.h "src/AudioCapture.cpp" "incl/AudioCapture.h" "incl/VolumeMeter.h" "src/VolumeMeter.cpp"  "incl/RecordingManager.h" "incl/FFmpegRecorder.h" "incl/FFmpegStreamer.h" "src/RecordingManager.cpp" "incl/FFmpegRecorder.h" "src/FFmpegRecorder.cpp")

# Link libraries
target_link_libraries(obs Qt::Core Qt::Gui Qt::Widgets d3d11 dxgi avcodec avformat avutil swscale swresample)

if(WIN32)
  set(DEBUG_SUFFIX)
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(DEBUG_SUFFIX "d")
  endif()
  set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
  if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
    set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
    if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
      set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
    endif()
  endif()
  if(EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
    add_custom_command(TARGET obs POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:obs>/plugins/platforms/")
    add_custom_command(TARGET obs POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy
        "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
        "$<TARGET_FILE_DIR:obs>/plugins/platforms/")
    foreach(QT_LIB Core Gui Widgets)
      add_custom_command(TARGET obs POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
          "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
          "$<TARGET_FILE_DIR:obs>/")
    endforeach(QT_LIB)
  endif()
endif()

if(WIN32)
  set(THREADS_PREFER_PTHREAD_FLAG OFF)
endif()

if(MSVC)
    foreach(flag_var
        CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG)
        string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach()
endif()