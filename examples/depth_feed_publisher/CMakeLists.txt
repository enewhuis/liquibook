find_package(XercesC COMPONENTS sax util ...)
find_package(Boost COMPONENTS date_time filesystem system thread ...)

#Publisher StaticLib
add_library(depth_feed_publisher_lib STATIC)

target_sources(depth_feed_publisher_lib
        PRIVATE
        depth_feed_connection.cpp
        depth_feed_publisher.cpp
        template_consumer.cpp
        example_order_book.cpp
        exchange.cpp
        order.cpp)

target_include_directories(depth_feed_publisher_lib
        PRIVATE
        ../../src
        ${QUICKFAST_SRC}/src)

target_compile_definitions(depth_feed_publisher_lib
        PUBLIC
        BOOST_BIND_GLOBAL_PLACEHOLDERS
        ASIOSERVICE_FWD_H)

target_link_libraries(depth_feed_publisher_lib
        PUBLIC
        ${Boost_LIBRARIES}
        ${XercesC_LIBRARIES}
        QuickFAST)

#Publisher
add_executable(depth_feed_publisher)

target_sources(depth_feed_publisher
        PRIVATE
        publisher_main.cpp
        )

target_include_directories(depth_feed_publisher
        PRIVATE
        ../../src
        ${QUICKFAST_SRC}/src)

target_compile_definitions(depth_feed_publisher
        PUBLIC
        BOOST_BIND_GLOBAL_PLACEHOLDERS
        ASIOSERVICE_FWD_H)

target_link_libraries(depth_feed_publisher
        PUBLIC
        depth_feed_publisher_lib)

#Subscriber
add_executable(depth_feed_subscriber)

target_sources(depth_feed_subscriber
        PRIVATE
        subscriber_main.cpp
        depth_feed_connection.cpp
        depth_feed_subscriber.cpp
        template_consumer.cpp
        order.cpp)

target_include_directories(depth_feed_subscriber
        PRIVATE
        ../../src
        ${QUICKFAST_SRC}/src)

target_compile_definitions(depth_feed_subscriber
        PUBLIC
        BOOST_BIND_GLOBAL_PLACEHOLDERS
        ASIOSERVICE_FWD_H)

target_link_libraries(depth_feed_subscriber
        PUBLIC
        ${Boost_LIBRARIES}
        ${XercesC_LIBRARIES}
        QuickFAST)