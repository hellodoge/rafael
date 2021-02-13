cmake_minimum_required(VERSION 3.15)
project(rafael C)

set(CMAKE_C_STANDARD 99)

add_executable(rafael src/main.c src/include/statement.h src/parser.c src/include/parser.h src/args.c src/include/args.h src/gc.c src/include/gc.h src/context.c src/include/context.h src/vars.c src/include/vars.h src/functions.c src/include/functions.h src/execution.c src/include/execution.h src/repl.c src/include/repl.h src/util.c src/include/util.h src/script_loader.c src/include/script_loader.h src/include/terminate.h src/include/options.h src/options.c)
target_link_libraries(rafael m)