cmake_minimum_required(VERSION 3.15)
project(rafael C)

set(CMAKE_C_STANDARD 99)

add_executable(rafael main.c statement.h parser.c parser.h args.c args.h gc.c gc.h context.c context.h vars.c vars.h functions.c functions.h execution.c execution.h repl.c repl.h util.c util.h script_loader.c script_loader.h terminate.h)
target_link_libraries(rafael m)