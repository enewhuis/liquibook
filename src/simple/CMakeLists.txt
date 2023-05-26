add_library(liquibook_lib STATIC)

target_sources(liquibook_lib
        PRIVATE
        simple_order.cpp)

target_include_directories(liquibook_lib
        PRIVATE
        ../)