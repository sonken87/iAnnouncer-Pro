cmake_minimum_required(VERSION 3.20)
project(skeuo_ui_cpp LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SKEUO_BUILD_GALLERY "Build the component gallery" ON)
option(SKEUO_BUILD_TESTS "Build the unit tests" ON)

# The stylesheet is shared with the Rust crate: one source of truth.
set(SKEUO_CSS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../crates/skeuo-ui/assets" CACHE PATH "skeuo-ui CSS assets")

set(SKEUO_ASSETS
  "tokens_css=${SKEUO_CSS_DIR}/tokens.css"
  "materials_css=${SKEUO_CSS_DIR}/materials.css"
  "inputs_css=${SKEUO_CSS_DIR}/inputs.css"
  "selection_css=${SKEUO_CSS_DIR}/selection.css"
  "data_display_css=${SKEUO_CSS_DIR}/data_display.css"
  "feedback_css=${SKEUO_CSS_DIR}/feedback.css"
  "surfaces_css=${SKEUO_CSS_DIR}/surfaces.css"
  "navigation_css=${SKEUO_CSS_DIR}/navigation.css"
  "dark_css=${SKEUO_CSS_DIR}/dark.css"
  "runtime_js=${CMAKE_CURRENT_SOURCE_DIR}/runtime/skeuo.js"
)
set(asset_files "")
foreach(entry IN LISTS SKEUO_ASSETS)
  string(REPLACE "=" ";" parts "${entry}")
  list(GET parts 1 path)
  list(APPEND asset_files "${path}")
endforeach()
set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
add_custom_command(
  OUTPUT "${GEN_DIR}/assets.hpp" "${GEN_DIR}/assets.cpp"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${GEN_DIR}"
  COMMAND ${CMAKE_COMMAND} "-DOUT_HEADER=${GEN_DIR}/assets.hpp" "-DOUT_SOURCE=${GEN_DIR}/assets.cpp"
          "-DINPUTS=${SKEUO_ASSETS}" -P "${CMAKE_CURRENT_SOURCE_DIR}/tools/embed.cmake"
  DEPENDS ${asset_files} "${CMAKE_CURRENT_SOURCE_DIR}/tools/embed.cmake"
  COMMENT "Embedding skeuo-ui stylesheet and runtime"
  VERBATIM)

file(GLOB_RECURSE SKEUO_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(skeuo STATIC ${SKEUO_SOURCES} "${GEN_DIR}/assets.cpp"
  host/stdio_host.cpp host/webview2_host.cpp)
target_include_directories(skeuo PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" PRIVATE "${GEN_DIR}")
if(MSVC)
  target_compile_options(skeuo PRIVATE /W4 /utf-8)
else()
  target_compile_options(skeuo PRIVATE -Wall -Wextra)
endif()
if(WIN32)
  target_include_directories(skeuo SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview2/include")
  target_link_libraries(skeuo PUBLIC ole32 user32 gdi32)
  set(SKEUO_WEBVIEW2_LOADER "${CMAKE_CURRENT_SOURCE_DIR}/third_party/webview2/x64/WebView2Loader.dll")
endif()

if(SKEUO_BUILD_GALLERY)
  file(GLOB GALLERY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/gallery/*.cpp"
       "${CMAKE_CURRENT_SOURCE_DIR}/gallery/pages/*.cpp")
  add_executable(gallery WIN32 ${GALLERY_SOURCES})
  target_link_libraries(gallery PRIVATE skeuo)
  if(WIN32)
    # The WebView2 loader must sit next to the exe.
    add_custom_command(TARGET gallery POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SKEUO_WEBVIEW2_LOADER}" "$<TARGET_FILE_DIR:gallery>")
    if(MINGW)
      target_link_options(gallery PRIVATE -static)
    endif()
  endif()
endif()

if(SKEUO_BUILD_TESTS)
  enable_testing()
  file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
  add_executable(skeuo_tests ${TEST_SOURCES})
  target_link_libraries(skeuo_tests PRIVATE skeuo)
  add_test(NAME skeuo_tests COMMAND skeuo_tests)
endif()
