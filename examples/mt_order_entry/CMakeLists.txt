add_executable(mt_order_entry)

target_sources(mt_order_entry
        PRIVATE
        mt_order_entry_main.cpp
        Market.cpp
        Market.h
        Order.cpp
        Order.h
        OrderFwd.h
        Util.cpp
        Util.h)

target_include_directories(mt_order_entry
        PRIVATE
        .
        ../../src)