
#include <boost/thread.hpp>
#include "exchange.h"
#include "depth_feed_publisher.h"
#include "depth_feed_connection.h"
#include "order.h"

#include <cstdlib>
#include <iostream>

using namespace liquibook;

struct SecurityInfo {
  std::string symbol;
  double ref_price;
  SecurityInfo(const char* sym, double price)
  : symbol(sym),
    ref_price(price)
  {
  }
};

typedef std::vector<SecurityInfo> SecurityVector;

void create_securities(SecurityVector& securities);
void populate_exchange(examples::Exchange& exchange,
                       const SecurityVector& securities);
void generate_orders(examples::Exchange& exchange,
                     const SecurityVector& securities);

int main(int argc, const char* argv[])
{
  try
  {
    // Feed connection
    examples::DepthFeedConnection connection(argc, argv);

    // Open connection in background thread
    connection.accept();
    boost::function<void ()> acceptor(
        boost::bind(&examples::DepthFeedConnection::run, &connection));
    boost::thread acceptor_thread(acceptor);

    // Create feed publisher
    examples::DepthFeedPublisher feed;
    feed.set_connection(&connection);

    // Create exchange
    examples::Exchange exchange(&feed, &feed);

    // Create securities
    SecurityVector securities;
    create_securities(securities);

    // Populate exchange with securities
    populate_exchange(exchange, securities);

    // Generate random orders
//    generate_orders(exchange, securities);
    while (true) {
      sleep(10000);
    }
  }
  catch (const std::exception & ex)
  {
    std::cerr << "Exception caught at main level: " << ex.what() << std::endl;
    return -1;
  }

  return 0;
}

void
create_securities(SecurityVector& securities) {
  securities.push_back(SecurityInfo("AAPL", 436.36));
}

void
populate_exchange(examples::Exchange& exchange, const SecurityVector& securities) {
  SecurityVector::const_iterator sec;
  for (sec = securities.begin(); sec != securities.end(); ++sec) {
    exchange.add_order_book(sec->symbol);
  }
}

void
generate_orders(examples::Exchange& exchange, const SecurityVector& securities) {
  time_t now;
  time(&now);
  std::srand(uint32_t(now));

  size_t num_securities = securities.size();
  while (true) {
    // which security
    size_t index = std::rand() % num_securities;
    const SecurityInfo& sec = securities[index];
    // side
    bool is_buy = (std::rand() % 2) != 0;
    // price
    uint32_t price_base = uint32_t(sec.ref_price * 100);
    uint32_t delta_range = price_base / 50;  // +/- 2% of base
    int32_t delta = std::rand() % delta_range;
    delta -= (delta_range / 2);
    double price = double (price_base + delta) / 100;

    // qty
    book::Quantity qty = (std::rand() % 10 + 1) * 100;

    // order
    examples::OrderPtr order(new examples::Order(is_buy, price, qty));

    // add order
    exchange.add_order(sec.symbol, order);

    // Wait for eyes to read
    sleep(1);
  }
}
