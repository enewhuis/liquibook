#include <iostream>
#include "version.h"

#include "depth_order_book.h"
#include "order.h"

using namespace liquibook;
using namespace book;
namespace
{
  // depth order book pulls in all the other header files.
  // except order.h which is actually a concept.
  DepthOrderBook<Order *, 5> unusedDepthOrderBook_;
}

int main(int, const char**)
{
    std::cout << "Liquibook version " << Version::MAJOR << '.' << Version::MINOR << '.' << Version::PATCH 
      << " (" << Version::RELEASE_DATE << ")\n";
    std::cout << "Liquibook is a header-only library.\n\n";
    std::cout << "This executable is a placeholder to make the book header files visible in\n";
    std::cout << "Visual Studio.  It also lets the compiler to do syntax checking of the header\n";
    std::cout << "files at build time." << std::endl;
    return 0;
}

