cmake_minimum_required (VERSION 3.7)
project (pascal_compiler VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

add_subdirectory(fmt)

add_executable(lexer src/lexer.cpp src/parser.cpp) 

target_link_libraries(lexer PUBLIC fmt)


add_executable(massager src/grammar_massager.cpp) 

target_link_libraries(massager PUBLIC fmt)



add_custom_command(
        TARGET lexer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/test_input
                ${CMAKE_CURRENT_BINARY_DIR}/test_input)

add_custom_command(TARGET lexer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${PROJECT_SOURCE_DIR}/test_input $<TARGET_FILE_DIR:lexer>/test_input)

add_custom_command(TARGET massager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/grammars $<TARGET_FILE_DIR:lexer>/grammars)
    