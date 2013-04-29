
#include <boost/thread.hpp>
#include "exchange.h"
#include "depth_feed_publisher.h"
#include "depth_feed_connection.h"
#include "order.h"

#include <cstdlib>
#include <iostream>

using namespace liquibook;

typedef std::vector<std::string> StringVector;

void create_symbols(StringVector& symbols);
void populate_exchange(examples::Exchange& exchange, 
                       const StringVector& symbols);
void generate_orders(examples::Exchange& exchange, 
                     const StringVector& symbols);

int main(int argc, const char* argv[])
{
  StringVector symbols;

  // Create symbols
  create_symbols(symbols);

  // Open connection in background thread
  examples::DepthFeedConnection connection(argc, argv);
  connection.accept();
  boost::function<void ()> acceptor(
      boost::bind(&examples::DepthFeedConnection::run, &connection));
  boost::thread acceptor_thread(acceptor);
  
  // Create feed publisher
  examples::DepthFeedPublisher feed("./templates/Simple.xml");
  feed.set_message_handler(&connection);

  // Create exchange
  examples::Exchange exchange(&feed);

  // Populate exchange with securities
  populate_exchange(exchange, symbols);
  
  sleep(10);

  // Generate random orders
  generate_orders(exchange, symbols);

  return 0;
}

void
create_symbols(StringVector& symbols) {
  symbols.push_back("SRFAS");
  symbols.push_back("AAPL");
  symbols.push_back("ADBE");
  symbols.push_back("ADI");
  symbols.push_back("ADP");
  symbols.push_back("ADSK");
  symbols.push_back("AKAM");
  symbols.push_back("ALTR");
  symbols.push_back("ALXN");
  symbols.push_back("AMAT");
  symbols.push_back("AMGN");
  symbols.push_back("AMZN");
  symbols.push_back("ATVI");
  symbols.push_back("AVGO");
  symbols.push_back("BBBY");
  symbols.push_back("BIDU");
  symbols.push_back("BIIB");
  symbols.push_back("BMC");
  symbols.push_back("BRCM");
  symbols.push_back("CA");
  symbols.push_back("CELG");
  symbols.push_back("CERN");
  symbols.push_back("CHKP");
  symbols.push_back("CHRW");
  symbols.push_back("CMCSA");
  symbols.push_back("COST");
  symbols.push_back("CSCO");
  symbols.push_back("CTRX");
  symbols.push_back("CTSH");
  symbols.push_back("CTXS");
  symbols.push_back("DELL");
  symbols.push_back("DISCA");
  symbols.push_back("DLTR");
  symbols.push_back("DTV");
  symbols.push_back("EBAY");
  symbols.push_back("EQIX");
  symbols.push_back("ESRX");
  symbols.push_back("EXPD");
  symbols.push_back("EXPE");
  symbols.push_back("FAST");
  symbols.push_back("FB");
  symbols.push_back("FFIV");
  symbols.push_back("FISV");
  symbols.push_back("FOSL");
  symbols.push_back("GILD");
  symbols.push_back("GOLD");
  symbols.push_back("GOOG");
  symbols.push_back("GRMN");
  symbols.push_back("HSIC");
  symbols.push_back("INTC");
  symbols.push_back("INTU");
  symbols.push_back("ISRG");
  symbols.push_back("KLAC");
  symbols.push_back("LBTYA");
  symbols.push_back("LIFE");
  symbols.push_back("LINTA");
  symbols.push_back("LLTC");
  symbols.push_back("MAT");
  symbols.push_back("MCHP");
  symbols.push_back("MDLZ");
  symbols.push_back("MNST");
  symbols.push_back("MSFT");
  symbols.push_back("MU");
  symbols.push_back("MXIM");
  symbols.push_back("MYL");
  symbols.push_back("NTAP");
  symbols.push_back("NUAN");
  symbols.push_back("NVDA");
  symbols.push_back("NWSA");
  symbols.push_back("ORCL");
  symbols.push_back("ORLY");
  symbols.push_back("PAYX");
  symbols.push_back("PCAR");
  symbols.push_back("PCLN");
  symbols.push_back("PRGO");
  symbols.push_back("QCOM");
  symbols.push_back("REGN");
  symbols.push_back("ROST");
  symbols.push_back("SBAC");
  symbols.push_back("SBUX");
  symbols.push_back("SHLD");
  symbols.push_back("SIAL");
  symbols.push_back("SIRI");
  symbols.push_back("SNDK");
  symbols.push_back("SPLS");
  symbols.push_back("SRCL");
  symbols.push_back("STRZA");
  symbols.push_back("STX");
  symbols.push_back("SYMC");
  symbols.push_back("TXN");
  symbols.push_back("VIAB");
  symbols.push_back("VMED");
  symbols.push_back("VOD");
  symbols.push_back("VRSK");
  symbols.push_back("VRTX");
  symbols.push_back("WDC");
  symbols.push_back("WFM");
  symbols.push_back("WYNN");
  symbols.push_back("XLNX");
  symbols.push_back("XRAY");
  symbols.push_back("YHOO");
}

void
populate_exchange(examples::Exchange& exchange, const StringVector& symbols) {
  StringVector::const_iterator symbol;
  for (symbol = symbols.begin(); symbol != symbols.end(); ++symbol) {
    exchange.add_order_book(*symbol);
  }
}

void
generate_orders(examples::Exchange& exchange, const StringVector& symbols) {
  time_t now;
  time(&now);
  std::srand(now);

  size_t num_symbols = symbols.size();
  while (true) {
    // which symbol
    size_t index = std::rand() % num_symbols;
    std::string symbol = symbols[index];
    // side
    bool is_buy = std::rand() % 2;
    // price
    uint32_t price_base = (index + 1) * 400; // Each symbol has different base
    uint32_t delta_range = price_base / 10;
    int32_t delta = std::rand() % price_base;
    delta -= (price_base / 2);
    book::Price price = price_base + delta;

    // qty
    book::Quantity qty = (std::rand() % 10 + 1) * 100;

    // order
    examples::OrderPtr order(new examples::Order(is_buy, price, qty));

    // add order
    exchange.add_order(symbol, order);
  }
}
