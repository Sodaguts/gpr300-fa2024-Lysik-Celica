file(
 GLOB_RECURSE TESTPROJECT_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE TESTPROJECT_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this assignment0's asset folder to bin when it is built
add_custom_target(copyAssetsTP ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${TESTPROJECT_INC} DESTINATION include/TestProject)
add_executable(TestProject ${TESTPROJECT_SRC} ${TESTPROJECT_INC})
target_link_libraries(TestProject PUBLIC core IMGUI assimp)
target_include_directories(TestProject PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignment0 is built
add_dependencies(TestProject copyAssetsTP)
