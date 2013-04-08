
#include <iostream>
#include "exchange.h"
#include "depth_feed_publisher.h"

using namespace liquibook;

void populate_exchange(examples::Exchange& exchange);

int main(int argc, const char* argv[])
{
  // Create feed publisher
  examples::DepthFeedPublisher feed;

  // Create exchange
  examples::Exchange exchange(&feed);

  // Populate with securities
  populate_exchange(exchange);
  
  // Generate orders
  return 0;
}

void
populate_exchange(examples::Exchange& exchange) {
  exchange.add_order_book("AAPL");
  exchange.add_order_book("ADBE");
  exchange.add_order_book("ADI");
  exchange.add_order_book("ADP");
  exchange.add_order_book("ADSK");
  exchange.add_order_book("AKAM");
  exchange.add_order_book("ALTR");
  exchange.add_order_book("ALXN");
  exchange.add_order_book("AMAT");
  exchange.add_order_book("AMGN");
  exchange.add_order_book("AMZN");
  exchange.add_order_book("ATVI");
  exchange.add_order_book("AVGO");
  exchange.add_order_book("BBBY");
  exchange.add_order_book("BIDU");
  exchange.add_order_book("BIIB");
  exchange.add_order_book("BMC");
  exchange.add_order_book("BRCM");
  exchange.add_order_book("CA");
  exchange.add_order_book("CELG");
  exchange.add_order_book("CERN");
  exchange.add_order_book("CHKP");
  exchange.add_order_book("CHRW");
  exchange.add_order_book("CMCSA");
  exchange.add_order_book("COST");
  exchange.add_order_book("CSCO");
  exchange.add_order_book("CTRX");
  exchange.add_order_book("CTSH");
  exchange.add_order_book("CTXS");
  exchange.add_order_book("DELL");
  exchange.add_order_book("DISCA");
  exchange.add_order_book("DLTR");
  exchange.add_order_book("DTV");
  exchange.add_order_book("EBAY");
  exchange.add_order_book("EQIX");
  exchange.add_order_book("ESRX");
  exchange.add_order_book("EXPD");
  exchange.add_order_book("EXPE");
  exchange.add_order_book("FAST");
  exchange.add_order_book("FB");
  exchange.add_order_book("FFIV");
  exchange.add_order_book("FISV");
  exchange.add_order_book("FOSL");
  exchange.add_order_book("GILD");
  exchange.add_order_book("GOLD");
  exchange.add_order_book("GOOG");
  exchange.add_order_book("GRMN");
  exchange.add_order_book("HSIC");
  exchange.add_order_book("INTC");
  exchange.add_order_book("INTU");
  exchange.add_order_book("ISRG");
  exchange.add_order_book("KLAC");
  exchange.add_order_book("LBTYA");
  exchange.add_order_book("LIFE");
  exchange.add_order_book("LINTA");
  exchange.add_order_book("LLTC");
  exchange.add_order_book("MAT");
  exchange.add_order_book("MCHP");
  exchange.add_order_book("MDLZ");
  exchange.add_order_book("MNST");
  exchange.add_order_book("MSFT");
  exchange.add_order_book("MU");
  exchange.add_order_book("MXIM");
  exchange.add_order_book("MYL");
  exchange.add_order_book("NTAP");
  exchange.add_order_book("NUAN");
  exchange.add_order_book("NVDA");
  exchange.add_order_book("NWSA");
  exchange.add_order_book("ORCL");
  exchange.add_order_book("ORLY");
  exchange.add_order_book("PAYX");
  exchange.add_order_book("PCAR");
  exchange.add_order_book("PCLN");
  exchange.add_order_book("PRGO");
  exchange.add_order_book("QCOM");
  exchange.add_order_book("REGN");
  exchange.add_order_book("ROST");
  exchange.add_order_book("SBAC");
  exchange.add_order_book("SBUX");
  exchange.add_order_book("SHLD");
  exchange.add_order_book("SIAL");
  exchange.add_order_book("SIRI");
  exchange.add_order_book("SNDK");
  exchange.add_order_book("SPLS");
  exchange.add_order_book("SRCL");
  exchange.add_order_book("STRZA");
  exchange.add_order_book("STX");
  exchange.add_order_book("SYMC");
  exchange.add_order_book("TXN");
  exchange.add_order_book("VIAB");
  exchange.add_order_book("VMED");
  exchange.add_order_book("VOD");
  exchange.add_order_book("VRSK");
  exchange.add_order_book("VRTX");
  exchange.add_order_book("WDC");
  exchange.add_order_book("WFM");
  exchange.add_order_book("WYNN");
  exchange.add_order_book("XLNX");
  exchange.add_order_book("XRAY");
  exchange.add_order_book("YHOO");
};
