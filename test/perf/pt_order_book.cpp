// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include <simple/simple_order_book.h>
#include <book/types.h>

#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <time.h>

using namespace liquibook;
using namespace liquibook::book;

typedef simple::SimpleOrderBook<5> FullDepthOrderBook;
typedef simple::SimpleOrderBook<1> BboOrderBook;
typedef book::OrderBook<simple::SimpleOrder*> NoDepthOrderBook;

template <class TypedOrderBook, class TypedOrder>
int run_test(TypedOrderBook& order_book, TypedOrder** orders, clock_t end) {
  int count = 0;
  TypedOrder** pp_order = orders;
  do {
    order_book.add(*pp_order);
    ++pp_order;
    if (*pp_order == nullptr) {
      return -1;
    }
    ++count;
  } while (clock() < end);
  return int(pp_order - orders);
}

template <class TypedOrderBook>
bool build_and_run_test(uint32_t dur_sec, uint32_t num_to_try) {
  std::cout << "trying run of " << num_to_try << " orders";
  TypedOrderBook order_book;
  simple::SimpleOrder** orders = new simple::SimpleOrder*[num_to_try + 1];
  
  for (uint32_t i = 0; i <= num_to_try; ++i) {
    bool is_buy((i % 2) == 0);
    uint32_t delta = is_buy ? 1880 : 1884;
    // ASK 1893
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
  
  clock_t start = clock();
  clock_t stop = start + (dur_sec * CLOCKS_PER_SEC);

  int count = run_test(order_book, orders, stop);
  for (uint32_t i = 0; i <= num_to_try; ++i) {
    delete orders[i];
  }
  delete [] orders;
  if (count > 0) {
    std::cout << " - complete!" << std::endl;
    std::cout << "Inserted " << count << " orders in " << dur_sec << " seconds"
              << ", or " << count / dur_sec << " insertions per sec"
              << std::endl;
    uint32_t remain = uint32_t(order_book.bids().size() + order_book.asks().size());
    std::cout << "Run matched " << count - remain << " orders" << std::endl;
    return true;
  } else {
    std::cout << " - not enough orders" << std::endl;
    return false;
  }

  return count > 0;
}

int main(int argc, const char* argv[])
{
  uint32_t dur_sec = 3;
  if (argc > 1) {
    dur_sec = atoi(argv[1]);
    if (!dur_sec) { 
      dur_sec = 3;
    }
  }
  std::cout << dur_sec << " sec performance test of order book" << std::endl;
  
  srand(dur_sec);

  {
    std::cout << "testing order book with depth" << std::endl;
    uint32_t num_to_try = dur_sec * 125000;
    while (true) {
      if (build_and_run_test<FullDepthOrderBook>(dur_sec, num_to_try)) {
        break;
      } else {
        num_to_try *= 2;
      }
    }
  }

  {
    std::cout << "testing order book with bbo" << std::endl;
    uint32_t num_to_try = dur_sec * 125000;
    while (true) {
      if (build_and_run_test<BboOrderBook>(dur_sec, num_to_try)) {
        break;
      } else {
        num_to_try *= 2;
      }
    }
  }

  {
    std::cout << "testing order book without depth" << std::endl;
    uint32_t num_to_try = dur_sec * 125000;
    while (true) {
      if (build_and_run_test<NoDepthOrderBook>(dur_sec, num_to_try)) {
        break;
      } else {
        num_to_try *= 2;
      }
    }
  }

}

