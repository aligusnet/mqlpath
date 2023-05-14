add_custom_command(
    OUTPUT lexer.h lexer.cpp
    DEPENDS Reflex ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
    COMMAND Reflex ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l)

add_custom_command(
    OUTPUT parser.h parser.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/grammar.y
    COMMAND bison ${CMAKE_CURRENT_SOURCE_DIR}/grammar.y "-Wcounterexamples")

add_executable(app
    ast.cpp
    ast_eval.cpp
    lexer.cpp
    parser.cpp
    error.cpp
    value.cpp
    ast_eval_test.cpp
    parser_test.cpp
    parse_eval_test.cpp)

target_link_libraries(app Catch2::Catch2WithMain ReflexLibStatic)

list(APPEND INCLUDES ${CMAKE_SOURCE_DIR}/src)
list(APPEND INCLUDES ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND INCLUDES ${CMAKE_SOURCE_DIR}/src/third_party)
list(APPEND INCLUDES ${CMAKE_SOURCE_DIR}/src/third_party/RE-flex/include)

target_include_directories(app PUBLIC ${INCLUDES})
