/*
 * Copyright (c), MM Weiss
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *     1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *     2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *     3. Neither the name of the MM Weiss nor the names of its contributors
 *     may be used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  clock_gettime_stub.c
 *  gcc -Wall -c clock_gettime_stub.c
 *  posix realtime functions; MacOS user space glue
 */

/*  @comment
 *  other possible implementation using intel builtin rdtsc
 *  rdtsc-workaround: http://www.mcs.anl.gov/~kazutomo/rdtsc.html
 *
 *  we could get the ticks by doing this
 *
 *  __asm __volatile("mov %%ebx, %%esi\n\t"
 *  		"cpuid\n\t"
 *  		"xchg %%esi, %%ebx\n\t"
 *  		"rdtsc"
 *  		: "=a" (a),
 *  		  "=d" (d)
 * 	);
 
 *  we could even replace our tricky sched_yield call by assembly code to get a better accurency,
 *  anyway the following C stub will satisfy 99% of apps using posix clock_gettime call,
 *  moreover, the setter version (clock_settime) could be easly written using mach primitives:
 *  http://www.opensource.apple.com/source/xnu/xnu-${VERSION}/osfmk/man/ (clock_[set|get]_time)
 *
 *  hackers don't be crackers, don't you use a flush toilet?
 *
 *
 *  @see draft: ./posix-realtime-stub/posix-realtime-stub.c
 *
 */


#ifdef __APPLE__

#pragma weak clock_gettime

#include <sys/time.h>
#include <sys/resource.h>
#include <mach/mach.h>
#include <mach/clock.h>
#include <mach/mach_time.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>

typedef enum {
    CLOCK_REALTIME,
    CLOCK_MONOTONIC,
    CLOCK_PROCESS_CPUTIME_ID,
    CLOCK_THREAD_CPUTIME_ID
} clockid_t;

static mach_timebase_info_data_t __clock_gettime_inf;

int clock_gettime(clockid_t clk_id, struct timespec *tp) {
    kern_return_t   ret;
    clock_serv_t    clk;
    clock_id_t clk_serv_id;
    mach_timespec_t tm;
    
    uint64_t start, end, delta, nano;
    
    task_basic_info_data_t tinfo;
    task_thread_times_info_data_t ttinfo;
    mach_msg_type_number_t tflag;
    
    int retval = -1;
    switch (clk_id) {
        case CLOCK_REALTIME:
        case CLOCK_MONOTONIC:
            clk_serv_id = clk_id == CLOCK_REALTIME ? CALENDAR_CLOCK : SYSTEM_CLOCK;
            if (KERN_SUCCESS == (ret = host_get_clock_service(mach_host_self(), clk_serv_id, &clk))) {
                if (KERN_SUCCESS == (ret = clock_get_time(clk, &tm))) {
                    tp->tv_sec  = tm.tv_sec;
                    tp->tv_nsec = tm.tv_nsec;
                    retval = 0;
                }
            }
            if (KERN_SUCCESS != ret) {
                errno = EINVAL;
                retval = -1;
            }
            break;
        case CLOCK_PROCESS_CPUTIME_ID:
        case CLOCK_THREAD_CPUTIME_ID:
            start = mach_absolute_time();
            if (clk_id == CLOCK_PROCESS_CPUTIME_ID) {
                getpid();
            } else {
                sched_yield();
            }
            end = mach_absolute_time();
            delta = end - start;
            if (0 == __clock_gettime_inf.denom) {
                mach_timebase_info(&__clock_gettime_inf);
            }
            nano = delta * __clock_gettime_inf.numer / __clock_gettime_inf.denom;
            tp->tv_sec = nano * 1e-9;  
            tp->tv_nsec = nano - (tp->tv_sec * 1e9);
            retval = 0;
            break;
        default:
            errno = EINVAL;
            retval = -1;
    }
    return retval;
}

#endif // __APPLE__

/* EOF */

// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#include "impl/simple_order_book.h"
#include "book/types.h"

#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <sys/time.h>

using namespace liquibook;
using namespace liquibook::book;

typedef impl::SimpleOrderBook<5> FullDepthOrderBook;
typedef impl::SimpleOrderBook<1> BboOrderBook;
typedef book::OrderBook<impl::SimpleOrder*> NoDepthOrderBook;

void build_histogram(timespec* timestamps, int count) {
  timespec* prev = NULL;
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
    order_book.perform_callbacks();
    ++pp_order;
    ++timestamp;
    if (*pp_order == NULL) {
      break;
    }
    ++count;
  } while (true);
  // Take timestamp at end
  int status = clock_gettime(CLOCK_REALTIME, timestamp);
  if (status) {
    throw std::runtime_error("clock_gettime() failed");
  }
  return (pp_order - orders);
}

template <class TypedOrderBook>
bool build_and_run_test(uint32_t num_to_try, bool dry_run = false) {
  TypedOrderBook order_book;
  impl::SimpleOrder** orders = new impl::SimpleOrder*[num_to_try + 1];
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
    orders[i] = new impl::SimpleOrder(is_buy, price, qty);
  }
  orders[num_to_try] = NULL; // Final null
  
  run_test(order_book, orders, timestamps);
  for (uint32_t i = 0; i <= num_to_try; ++i) {
    delete orders[i];
  }
  delete [] orders;
  std::cout << " - complete!" << std::endl;
  uint32_t remain = order_book.bids().size() + order_book.asks().size();
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

