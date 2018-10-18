cmake_minimum_required (VERSION 3.7)
project (pascal_compiler VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(fmt)

add_executable(compiler src/main.cpp src/lexer.cpp src/parser.cpp) 

target_link_libraries(compiler PUBLIC fmt)

add_executable(massager src/grammar_massager.cpp) 
target_link_libraries(massager PUBLIC fmt)

if(MSVC)
    target_compile_options(compiler PRIVATE "/std:c++17")
	target_compile_options(compiler PRIVATE "/permissive-") 
    target_compile_options(massager PRIVATE "/std:c++17")
	target_compile_options(massager PRIVATE "/permissive-")    
   
endif(MSVC)






add_custom_command(
        TARGET compiler POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/test_input
                ${CMAKE_CURRENT_BINARY_DIR}/test_input)

add_custom_command(TARGET compiler POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${PROJECT_SOURCE_DIR}/test_input $<TARGET_FILE_DIR:compiler>/test_input)

add_custom_command(TARGET massager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/grammars $<TARGET_FILE_DIR:massager>/grammars)
    