// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <simple/simple_order_book.h>
#include <book/types.h>
#include "clock_gettime.h"

#include <iostream>
#include <stdexcept>
#include <stdlib.h>

using namespace liquibook;
using namespace liquibook::book;

typedef simple::SimpleOrderBook<5> FullDepthOrderBook;
typedef simple::SimpleOrderBook<1> BboOrderBook;
typedef book::OrderBook<simple::SimpleOrder*> NoDepthOrderBook;

void build_histogram(timespec* timestamps, int count) {
  timespec* prev = nullptr;
  std::cout << "Latency (ns) " << std::endl;

  for (timespec* timestamp = timestamps;
       (timestamp - timestamps) <= count;
       ++timestamp)
  {
    if (prev) {
      timespec elapsed;
      if (timestamp->tv_sec == prev->tv_sec) {
        elapsed.tv_sec = 0;
        elapsed.tv_nsec = timestamp->tv_nsec - prev->tv_nsec;
      } else {
        // Second changed
        elapsed.tv_sec = timestamp->tv_sec - prev->tv_sec;
        // Borrow from sec if necessary
        if (prev->tv_nsec > timestamp->tv_nsec) {
          --elapsed.tv_sec;
          elapsed.tv_nsec = 1000000000 + timestamp->tv_nsec - prev->tv_nsec;
        } else {
          elapsed.tv_nsec = timestamp->tv_nsec - prev->tv_nsec;
        }
      }
      if (elapsed.tv_sec) {
        std::cout << elapsed.tv_sec * 1000000000 + elapsed.tv_nsec << std::endl;
      } else {
        std::cout << elapsed.tv_nsec << std::endl;
      }
    }
    prev = timestamp;
  }
}

template <class TypedOrderBook, class TypedOrder>
int run_test(TypedOrderBook& order_book, TypedOrder** orders,
             timespec* timestamps) {
  int count = 0;
  TypedOrder** pp_order = orders;
  timespec* timestamp = timestamps;
  do {
    // Take timestamp at start of each order
    int status = clock_gettime(CLOCK_REALTIME, timestamp);
    if (status) {
      throw std::runtime_error("clock_gettime() failed");
    }
    order_book.add(*pp_order);
    ++pp_order;
    ++timestamp;
    if (*pp_order == nullptr) {
      break;
    }
    ++count;
  } while (true);
  // Take timestamp at end
  int status = clock_gettime(CLOCK_REALTIME, timestamp);
  if (status) {
    throw std::runtime_error("clock_gettime() failed");
  }
  return int(pp_order - orders);
}

template <class TypedOrderBook>
bool build_and_run_test(uint32_t num_to_try, bool dry_run = false) {
  TypedOrderBook order_book;
  simple::SimpleOrder** orders = new simple::SimpleOrder*[num_to_try + 1];
  timespec* timestamps = new timespec[num_to_try + 1];
  
  for (uint32_t i = 0; i < num_to_try; ++i) {
    bool is_buy((i % 2) == 0);
    uint32_t delta = is_buy ? 1880 : 1884;
    // AsSK 1893
    // ASK 1892
    // ASK 1891
    // ASK 1890
    // ASK 1889 crossable
    // ASK 1888 crossable
    // ASK 1887 crossable
    // ASK 1886 crossable
    // ASK 1885 crossable
    // ASK 1884 crossable

    // BID 1889 crossable
    // BID 1888 crossable
    // BID 1887 crossable
    // BID 1886 crossable
    // BID 1885 crossable
    // BID 1884 crossable
    // BID 1883
    // BID 1882
    // BID 1881
    // BID 1880

    Price price = (rand() % 10) + delta;
    
    Quantity qty = ((rand() % 10) + 1) * 100;
    orders[i] = new simple::SimpleOrder(is_buy, price, qty);
  }
  orders[num_to_try] = nullptr; // Final null
  
  run_test(order_book, orders, timestamps);
  for (uint32_t i = 0; i <= num_to_try; ++i) {
    delete orders[i];
  }
  delete [] orders;
  std::cout << " - complete!" << std::endl;
  uint32_t remain = uint32_t(order_book.bids().size() + order_book.asks().size());
  if (!dry_run) {
    std::cout << "Building histogram" << std::endl;
    build_histogram(timestamps, num_to_try);
  }
  delete [] timestamps;

  return true;
}

int main(int argc, const char* argv[])
{
  uint32_t num_to_try = 10000;
  if (argc > 1) {
    num_to_try = atoi(argv[1]);
    if (!num_to_try) { 
      num_to_try = 10000;
    }
  }
  std::cout << num_to_try << " order latency test of order book" << std::endl;
  srand(num_to_try);

  {
    std::cout << "starting dry run" << std::endl;
    build_and_run_test<FullDepthOrderBook>(num_to_try, true);
  }

  {
    std::cout << "testing order book with depth" << std::endl;
    build_and_run_test<FullDepthOrderBook>(num_to_try);
  }

  {
    std::cout << "testing order book with bbo" << std::endl;
    build_and_run_test<BboOrderBook>(num_to_try);
  }

  {
    std::cout << "testing order book without depth" << std::endl;
    build_and_run_test<NoDepthOrderBook>(num_to_try);
  }
}

