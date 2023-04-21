find_package(XercesC COMPONENTS sax ...)
find_package(Boost COMPONENTS date_time filesystem system thread ...)

#Publisher
add_executable(depth_feed_publisher)

target_sources(depth_feed_publisher
        PRIVATE
        publisher_main.cpp
        depth_feed_connection.cpp
        depth_feed_publisher.cpp
        template_consumer.cpp
        example_order_book.cpp
        exchange.cpp
        order.cpp)

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
        ${Boost_LIBRARIES}
        ${XercesC_LIBRARIES}
        ${QUICKFAST_BUILD}/src/libQuickFAST.a)

#My Publisher
add_executable(my_depth_feed_publisher)

target_sources(my_depth_feed_publisher
        PRIVATE
        my_publisher_main.cpp
        depth_feed_connection.cpp
        depth_feed_publisher.cpp
        template_consumer.cpp
        example_order_book.cpp
        exchange.cpp
        order.cpp)

target_include_directories(my_depth_feed_publisher
        PRIVATE
        ../../src
        ${QUICKFAST_SRC}/src)

target_compile_definitions(my_depth_feed_publisher
        PUBLIC
        BOOST_BIND_GLOBAL_PLACEHOLDERS
        ASIOSERVICE_FWD_H)

target_link_libraries(my_depth_feed_publisher
        PUBLIC
        ${Boost_LIBRARIES}
        ${XercesC_LIBRARIES}
        ${QUICKFAST_BUILD}/src/libQuickFAST.a)

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
        ${QUICKFAST_BUILD}/src/libQuickFAST.a)