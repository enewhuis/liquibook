
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
    generate_orders(exchange, securities);
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
  securities.push_back(SecurityInfo("ADBE", 45.06));
  securities.push_back(SecurityInfo("ADI", 43.93));
  securities.push_back(SecurityInfo("ADP", 67.09));
  securities.push_back(SecurityInfo("ADSK", 38.34));
  securities.push_back(SecurityInfo("AKAM", 43.65));
  securities.push_back(SecurityInfo("ALTR", 31.90));
  securities.push_back(SecurityInfo("ALXN", 96.28));
  securities.push_back(SecurityInfo("AMAT", 14.623));
  securities.push_back(SecurityInfo("AMGN", 104.88));
  securities.push_back(SecurityInfo("AMZN", 247.74));
  securities.push_back(SecurityInfo("ATVI", 14.69));
  securities.push_back(SecurityInfo("AVGO", 31.38));
  securities.push_back(SecurityInfo("BBBY", 68.81));
  securities.push_back(SecurityInfo("BIDU", 85.09));
  securities.push_back(SecurityInfo("BIIB", 214.89));
  securities.push_back(SecurityInfo("BMC", 45.325));
  securities.push_back(SecurityInfo("BRCM", 35.60));
  securities.push_back(SecurityInfo("CA", 26.97));
  securities.push_back(SecurityInfo("CELG", 116.901));
  securities.push_back(SecurityInfo("CERN", 95.24));
  securities.push_back(SecurityInfo("CHKP", 46.43));
  securities.push_back(SecurityInfo("CHRW", 58.89));
  securities.push_back(SecurityInfo("CMCSA", 41.99));
  securities.push_back(SecurityInfo("COST", 108.16));
  securities.push_back(SecurityInfo("CSCO", 20.425));
  securities.push_back(SecurityInfo("CTRX", 57.419));
  securities.push_back(SecurityInfo("CTSH", 63.62));
  securities.push_back(SecurityInfo("CTXS", 62.38));
  securities.push_back(SecurityInfo("DELL", 13.33));
  securities.push_back(SecurityInfo("DISCA", 78.18));
  securities.push_back(SecurityInfo("DLTR", 47.91));
  securities.push_back(SecurityInfo("DTV", 56.56));
  securities.push_back(SecurityInfo("EBAY", 52.215));
  securities.push_back(SecurityInfo("EQIX", 217.015));
  securities.push_back(SecurityInfo("ESRX", 59.26));
  securities.push_back(SecurityInfo("EXPD", 35.03));
  securities.push_back(SecurityInfo("EXPE", 55.15));
  securities.push_back(SecurityInfo("FAST", 48.13));
  securities.push_back(SecurityInfo("FB", 27.52));
  securities.push_back(SecurityInfo("FFIV", 74.11));
  securities.push_back(SecurityInfo("FISV", 87.58));
  securities.push_back(SecurityInfo("FOSL", 95.09));
  securities.push_back(SecurityInfo("GILD", 50.06));
  securities.push_back(SecurityInfo("GOLD", 78.681));
  securities.push_back(SecurityInfo("GOOG", 817.08));
  securities.push_back(SecurityInfo("GRMN", 33.33));
  securities.push_back(SecurityInfo("HSIC", 89.44));
  securities.push_back(SecurityInfo("INTC", 23.9673));
  securities.push_back(SecurityInfo("INTU", 60.15));
  securities.push_back(SecurityInfo("ISRG", 492.3358));
  securities.push_back(SecurityInfo("KLAC", 53.83));
  securities.push_back(SecurityInfo("KRFT", 50.9001));
  securities.push_back(SecurityInfo("LBTYA", 73.99));
  securities.push_back(SecurityInfo("LIFE", 73.59));
  securities.push_back(SecurityInfo("LINTA", 21.44));
  securities.push_back(SecurityInfo("LLTC", 36.25));
  securities.push_back(SecurityInfo("MAT", 44.99));
  securities.push_back(SecurityInfo("MCHP", 36.1877));
  securities.push_back(SecurityInfo("MDLZ", 31.58));
  securities.push_back(SecurityInfo("MNST", 55.75));
  securities.push_back(SecurityInfo("MSFT", 32.75));
  securities.push_back(SecurityInfo("MU", 9.19));
  securities.push_back(SecurityInfo("MXIM", 30.59));
  securities.push_back(SecurityInfo("MYL", 28.90));
  securities.push_back(SecurityInfo("NTAP", 34.17));
  securities.push_back(SecurityInfo("NUAN", 18.89));
  securities.push_back(SecurityInfo("NVDA", 13.7761));
  securities.push_back(SecurityInfo("NWSA", 31.12));
  securities.push_back(SecurityInfo("ORCL", 33.19));
  securities.push_back(SecurityInfo("ORLY", 107.58));
  securities.push_back(SecurityInfo("PAYX", 36.32));
  securities.push_back(SecurityInfo("PCAR", 49.52));
  securities.push_back(SecurityInfo("PCLN", 697.62));
  securities.push_back(SecurityInfo("PRGO", 119.00));
  securities.push_back(SecurityInfo("QCOM", 61.925));
  securities.push_back(SecurityInfo("REGN", 242.49));
  securities.push_back(SecurityInfo("ROST", 65.20));
  securities.push_back(SecurityInfo("SBAC", 78.76));
  securities.push_back(SecurityInfo("SBUX", 60.07));
  securities.push_back(SecurityInfo("SHLD", 49.989));
  securities.push_back(SecurityInfo("SIAL", 77.95));
  securities.push_back(SecurityInfo("SIRI", 3.36));
  securities.push_back(SecurityInfo("SNDK", 51.23));
  securities.push_back(SecurityInfo("SPLS", 13.07));
  securities.push_back(SecurityInfo("SRCL", 108.15));
  securities.push_back(SecurityInfo("STX", 36.82));
  securities.push_back(SecurityInfo("SYMC", 24.325));
  securities.push_back(SecurityInfo("TXN", 36.28));
  securities.push_back(SecurityInfo("VIAB", 66.295));
  securities.push_back(SecurityInfo("VMED", 49.56));
  securities.push_back(SecurityInfo("VOD", 30.49));
  securities.push_back(SecurityInfo("VRSK", 61.1728));
  securities.push_back(SecurityInfo("VRTX", 77.255));
  securities.push_back(SecurityInfo("WDC", 54.76));
  securities.push_back(SecurityInfo("WFM", 89.35));
  securities.push_back(SecurityInfo("WYNN", 136.33));
  securities.push_back(SecurityInfo("XLNX", 37.59));
  securities.push_back(SecurityInfo("XRAY", 42.26));
  securities.push_back(SecurityInfo("YHOO", 24.32));
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
