cmake_minimum_required(VERSION 3.5)

project(index)

function(add_flag flag)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
endfunction()

set(SOURCE_EXE main.cpp)
set(URHO3D_LIB "${URHO3D_HOME}/lib/libUrho3D.a")

add_executable( ${PROJECT_NAME} ${SOURCE_EXE} )

# common flags
add_flag("-std=c++11")
add_flag("-Wall")
#add_flag("-v")

include_directories("${URHO3D_HOME}/include")

if(EMSCRIPTEN)
  message("Emscripten Build")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  #add_flag("-s USE_SDL=2")
  #add_flag("-s USE_WEBGL2=1")
  add_flag("-s WASM=1")
  add_flag("-s ALLOW_MEMORY_GROWTH=1")
  #add_flag("-s USE_OGG=1")
  #add_flag("-s ASSERTIONS=1")
  #add_flag("-s ERROR_ON_UNDEFINED_SYMBOLS=1")
  #add_flag("-s DISABLE_EXCEPTION_CATCHING=2")
  add_flag("--no-heap-copy")
  add_flag("--emrun")
  add_flag("--preload-file ${PROJECT_SOURCE_DIR}/data@/")
  add_flag("-O2")
  target_link_libraries( ${PROJECT_NAME} ${URHO3D_LIB} GL GLEW)
else()
  message("Linux Build")

  find_package(GLEW REQUIRED)
  if (GLEW_FOUND)
      include_directories(${GLEW_INCLUDE_DIRS})
      link_libraries(${GLEW_LIBRARIES})
  endif()

  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
          "${CMAKE_SOURCE_DIR}/data/CoreData"
          "${CMAKE_BINARY_DIR}/CoreData")

  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
          "${CMAKE_SOURCE_DIR}/data/Data"
          "${CMAKE_BINARY_DIR}/Data")

  target_link_libraries( ${PROJECT_NAME} ${URHO3D_LIB} GL GLEW dl pthread)
endif()

