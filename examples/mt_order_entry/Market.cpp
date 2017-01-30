// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.

#include "Market.h"

#include <functional> 
#include <cctype>
#include <locale>
namespace {
    const uint32_t INVALID_UINT32 = UINT32_MAX;
    const int32_t INVALID_INT32 = INT32_MAX;

    ////////////////////////////////////
    // Command parsing helpers

    std::string nextToken(const std::vector<std::string> & tokens, size_t & pos)
    {
        if(pos < tokens.size())
        {
            return tokens[pos++];
        }
        return "";
    }

    uint32_t toUint32(const std::string & input)
    {
        char * end;
        uint32_t value = strtoul(input.c_str(), &end, 10);
        if(*end != '\0')
        {
            value = INVALID_UINT32;
        }
        return value;
    }

    uint32_t toInt32(const std::string & input)
    {
        char * end;
        uint32_t value = strtol(input.c_str(), &end, 10);
        if(*end != '\0')
        {
            value = INVALID_INT32;
        }
        return value;
    }


    liquibook::book::Price stringToPrice(const std::string & str)
    {
        if(str == "MARKET" || str == "MKT")
        {
            return 0;
        } else
        {
            return toUint32(str);
        }
    }

    std::string promptForString(const std::string & prompt)
    {
        std::cout << "\n" << prompt << ": " << std::flush;
        std::string input;
        std::getline(std::cin, input);
        std::transform(input.begin(), input.end(), input.begin(), toupper);
        return input;
    }

    liquibook::book::Price promptForPrice(const std::string & prompt)
    {
        std::string str = promptForString(prompt);
        return stringToPrice(str);
    }

    uint32_t promptForUint32(const std::string & prompt)
    {
        std::cout << "\n" << prompt << ": " << std::flush;
        std::string input;
        std::getline(std::cin, input);
        return toUint32(input);
    }

    int32_t promptForInt32(const std::string & prompt)
    {
        std::cout << "\n" << prompt << ": " << std::flush;
        std::string input;
        std::getline(std::cin, input);
        return toInt32(input);
    }

    bool promptForYesNo(const std::string & prompt)
    {
        while(true)
        {
            std::string input = promptForString(prompt);
            if(input == "Y" || input == "YES" || input == "T" || input == "TRUE")
            {
                return true;
            }
            if(input == "N" || input == "NO" || input == "F" || input == "FALSE")
            {
                return false;
            }
        }
    }

    // trim from start (in place)
    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), 
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    }

    // trim from end (in place)
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), 
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    }

    // trim from both ends (in place)
    static inline void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

    // trim from start (copying)
    static inline std::string ltrimmed(std::string s) {
        ltrim(s);
        return s;
    }

    // trim from end (copying)
    static inline std::string rtrimmed(std::string s) {
        rtrim(s);
        return s;
    }

    // trim from both ends (copying)
    static inline std::string trimmed(std::string s) {
        trim(s);
        return s;
    }

}


namespace orderentry
{

uint32_t Market::orderIdSeed_ = 0;

Market::Market()
{
}

Market::~Market()
{
}

const char * 
Market::prompt()
{
    return "\t(B)uy\n\t(S)ell\n\t(M)odify\n\t(C)ancel\n\t(D)isplay\n";
}

void 
Market::help(std::ostream & out)
{
    out << "Buy: Create a new Buy order and add it to the book\n"
        << "Sell: Create a new Sell order and add it to the book\n"
        << "  Arguments for BUY or SELL\n"
        << "     <Quantity>\n"
        << "     <Symbol>\n"
        << "     <Price> or MARKET\n"
        << "     AON            (optional)\n"
        << "     IOC            (optional)\n"
        << "     STOP <Price>   (optional)\n"
        << "     ;              end of order\n"
        << std::endl;

    out << "Modify: Request Modify an existing order\n"
        << "  Arguments:\n"
        << "     <order#>\n"
        << "     PRICE <new price>\n"
        << "     QUANTITY <new initial quantity>\n"
        << "     ;              end of modify requet\n"
        << std::endl;

    out << "Cancel: Request cancel an existing order\n"
        << "  Arguments:\n"
        << "     <order#>\n"
        << "     ;              end of cancel request (optional)\n"
        << std::endl;

    out << "Display: Display status of an existing order\n"
        << "  Arguments:\n"
        << "     +       (enable verbose display)\n"
        << "     <order#> or <symbol> or \"all\"\n"
        << std::endl;

}

bool 
Market::apply(const std::vector<std::string> & tokens)
{
    const std::string & command = tokens[0];
    if(command == "BUY" || command == "B")
    {
        return doAdd("BUY", tokens, 1);
    }
    if(command == "SELL" || command == "S")
    {
        return doAdd("SELL", tokens, 1);
    }
    else if (command == "CANCEL" || command == "C")
    {
        return doCancel(tokens, 1);
    }
    else if(command == "MODIFY" || command == "M")
    {
        return doModify(tokens, 1);
    }
    else if(command == "DISPLAY" || command == "D")
    {
        return doDisplay(tokens, 1);
    }
    return false;
}


////////
// ADD
bool 
Market::doAdd(const  std::string & side, const std::vector<std::string> & tokens, size_t pos)
{
    //////////////
    // Quantity
    liquibook::book::Quantity quantity;
    std::string qtyStr = nextToken(tokens, pos);
    if(!qtyStr.empty())
    {
        quantity = toUint32(qtyStr);
    }
    else
    {
        quantity = promptForUint32("Quantity");
    }
    // sanity check
    if(quantity == 0 || quantity > 1000000000)
    {
        std::cerr << "--Expecting quantity" << std::endl;
        return false;
    }

    //////////////
    // SYMBOL
    std::string symbol = nextToken(tokens, pos);
    if(symbol.empty())
    {
        symbol = promptForString("Symbol");
    }
    if(!symbolIsDefined(symbol))
    {
        if(symbol[0] == '+')
        {
            symbol = symbol.substr(1);
            if(!symbolIsDefined(symbol))
            {
                addBook(symbol);
            }
        }
        else
        {
            if(!promptForYesNo("Is " + symbol +  " a new symbol?"))
            {
                std::cerr << "--Expecting valid symbol" << std::endl;
                return false;
            }
            addBook(symbol);
        }
    }

    ///////////////
    // PRICE
    uint32_t price = 0;
    std::string priceStr = nextToken(tokens, pos);
    if(!priceStr.empty())
    {
        price = stringToPrice(priceStr);
    }
    else
    {
        price = promptForPrice("Limit Price or MKT");
    }
    if(price > 10000000)
    {
        std::cerr << "--Expecting price or MARKET" << std::endl;
        return false;
    }

    //////////////////////////
    // OPTIONS: AON, IOC STOP
    bool aon = false;
    bool ioc = false;
    liquibook::book::Price stopPrice = 0;
    bool go = false;
    while(!go)
    {
        bool prompted = false;
        bool optionOk = false;
        std::string option = nextToken(tokens, pos);
        if(option.empty())
        {
            prompted = true;
            option = promptForString("AON, or IOC, or STOP, or END");
        }
        if(option == ";" || option == "E" || option == "END")
        {
            go = true;
            optionOk = true;
        }
        else if(option == "A" || option == "AON")
        {
            aon = true;
            optionOk = true;
        }
        else if(option == "I" || option == "IOC")
        {
            ioc = true;
            optionOk = true;
        }
        else if(option == "S" || option == "STOP")
        {
            std::string stopstr = nextToken(tokens, pos);

            if(!stopstr.empty())
            {
                stopPrice = stringToPrice(stopstr);
            } 
            else
            {
                stopPrice = promptForUint32("Stop Price");
                prompted = true;
            }
            optionOk = stopPrice <= 10000000;
        }
        if(!optionOk)
        {
            std::cout << "Unknown option " << option << std::endl;
            if(!prompted)
            {
                std::cerr << "--Expecting AON IOC STOP or END" << std::endl;
                return false;
            }
        }
    }

    std::string orderId = std::to_string(++orderIdSeed_);

    OrderPtr order = std::make_shared<Order>(orderId, side == "BUY", quantity, symbol, price, stopPrice, aon, ioc);

    const liquibook::book::OrderConditions AON(liquibook::book::oc_all_or_none);
    const liquibook::book::OrderConditions IOC(liquibook::book::oc_immediate_or_cancel);
    const liquibook::book::OrderConditions NOC(liquibook::book::oc_no_conditions);

    const liquibook::book::OrderConditions conditions = 
        (aon ? AON : NOC) | (ioc ? IOC : NOC);


    auto book = findBook(symbol);
    if(!book)
    {
        std::cerr << "--No order book for symbol" << symbol << std::endl;
        return false;
    }

    order->onSubmitted();
    std::cout << "ADDING order:  " << *order << std::endl;

    orders_[orderId] = order;
    book->add(order, conditions);
    book->perform_callbacks();
    return true;
}

///////////
// CANCEL
bool
Market::doCancel(const std::vector<std::string> & tokens, size_t position)
{
    OrderPtr order;
    OrderBookPtr book;
    if(!findExistingOrder(tokens, position, order, book))
    {
        return false;
    }
    std::cout << "Requesting Cancel: " << *order << std::endl;
    book->cancel(order);
    book->perform_callbacks();
    return true;
}



///////////
// MODIFY
bool
Market::doModify(const std::vector<std::string> & tokens, size_t position)
{
    OrderPtr order;
    OrderBookPtr book;
    if(!findExistingOrder(tokens, position, order, book))
    {
        return false;
    }

    //////////////
    // options
    //////////////////////////
    // OPTIONS: PRICE (price) ; QUANTITY (delta)

    int32_t quantityChange = liquibook::book::SIZE_UNCHANGED;
    liquibook::book::Price price = liquibook::book::PRICE_UNCHANGED;

    bool go = false;
    while(!go)
    {
        bool prompted = false;
        bool optionOk = false;
        std::string option = nextToken(tokens, position);
        if(option.empty())
        {
            prompted = true;
            option = promptForString("PRICE, or QUANTITY, or END");
        }
        if(option == ";" || option == "E" || option == "END")
        {
            go = true;
            optionOk = true;
        }
        else if(option == "P" || option == "PRICE")
        {
            uint32_t newPrice = INVALID_UINT32;
            std::string priceStr = nextToken(tokens, position);
            if(priceStr.empty())
            {
                newPrice = promptForUint32("New Price");
            }
            else
            {
                newPrice = toUint32(priceStr);
            }

            if(newPrice > 0 && newPrice != INVALID_UINT32)
            {
                price = newPrice;
                optionOk = true;
            }
            else
            {
                std::cerr << "Invalid price" << std::endl;
            }
        }
        else if(option == "Q" || option == "QUANTITY")
        {
            int32_t qty = INVALID_INT32;
            std::string qtyStr = nextToken(tokens, position);
            if(qtyStr.empty())
            {
                qty = promptForInt32("Change in quantity");
            }
            else
            {
                qty = toInt32(qtyStr);
            }
            if(qty != INVALID_INT32)
            {
                quantityChange = qty;
                optionOk = true;
            }
            else
            {
                std::cerr << "Invalid quantity change." << std::endl;
            }
        }

        if(!optionOk)
        {
            std::cout << "Unknown or invalid option " << option << std::endl;
            if(!prompted)
            {
                std::cerr << "--Expecting PRICE <price>, or QUANTITY <change>, or  END" << std::endl;
                return false;
            }
        }
    }

    book->replace(order, quantityChange, price);
    std::cout << "Requested Modify" ;
    if(quantityChange != liquibook::book::SIZE_UNCHANGED)
    {
        std::cout << " QUANTITY  += " << quantityChange;
    }
    if(price != liquibook::book::PRICE_UNCHANGED)
    {
        std::cout << " PRICE " << price;
    }
    std::cout << std::endl;
    book->perform_callbacks();
    return true;
}

///////////
// DISPLAY
bool
Market::doDisplay(const std::vector<std::string> & tokens, size_t pos)
{
    bool verbose = false;
    // see if first token could be an order id.
    // todo: handle prompted imput!
    std::string parameter = nextToken(tokens, pos);
    if(parameter.empty())
    {
        parameter = promptForString("+ or #OrderId or -orderOffset or symbol or \"ALL\"");
    }
    else
    {
        --pos; // Don't consume this parameter yet.
    }
    if(parameter[0] == '+')
    {
        verbose = true;
        if(parameter.length() > 1)
        {
            parameter = parameter.substr(1);
        }
        else
        {
            ++pos; // now we can consume the first parameter (whether or not it's there!)
            parameter = nextToken(tokens, pos);
            if(parameter.empty())
            {
                parameter = promptForString("#OrderId or -orderOffset or symbol or \"ALL\"");
            }
            else
            {
                --pos; // Don't consume this parameter yet.
            }
        }
    }
    if(parameter[0] == '#' || parameter[0] == '-' || isdigit(parameter[0]))
    {
        OrderPtr order;
        OrderBookPtr book;
        if(findExistingOrder(parameter, order, book))
        {
            std::cout << *order << std::endl;
            return true;
        }
    }

    // Not an order id.  Try for a symbol:
    std::string symbol = parameter;
    if(symbolIsDefined(symbol))
    {
        for(auto pOrder = orders_.begin(); pOrder != orders_.end(); ++pOrder)
        {
            const OrderPtr & order = pOrder->second;
            if(order->symbol() == symbol)
            {
                std::cout << order->verbose(verbose) << std::endl;
                order->verbose(false);
            }
        }
        auto book = findBook(symbol);
        if(!book)
        {
            std::cerr << "--No order book for symbol" << symbol << std::endl;
        }
        else
        {
            book->log(std::cout);
        }
        return true;
    }
    else if( symbol == "ALL")
    {
        for(auto pOrder = orders_.begin(); pOrder != orders_.end(); ++pOrder)
        {
            const OrderPtr & order = pOrder->second;
            std::cout << order->verbose(verbose) << std::endl;
            order->verbose(false);
        }

        for(auto pBook = books_.begin(); pBook != books_.end(); ++pBook)
        {
            std::cout << "Order book for " << pBook->first << std::endl;
            pBook->second->log(std::cout);
        }
        return true;
    }
    else
    {
        std::cout << "--Unknown symbol: " << symbol << std::endl;
    }
    return false;
}


////////////////////////////
// Command parsing helpers

/////////////////////////////
// Order book interactions

bool
Market::symbolIsDefined(const std::string & symbol)
{
    auto book = books_.find(symbol);
    return book != books_.end();
}

OrderBookPtr
Market::addBook(const std::string & symbol)
{
    OrderBookPtr result = std::make_shared<OrderBook>(symbol);
    result->set_order_listener(this);
    result->set_trade_listener(this);
    result->set_order_book_listener(this);
    books_[symbol] = result;
    return result;
}

OrderBookPtr
Market::findBook(const std::string & symbol)
{
    OrderBookPtr result;
    auto entry = books_.find(symbol);
    if(entry != books_.end())
    {
        result = entry->second;
    }
    return result;
}

bool Market::findExistingOrder(const std::vector<std::string> & tokens, size_t & position, OrderPtr & order, OrderBookPtr & book)
{
    ////////////////
    // Order ID
    std::string orderId = nextToken(tokens, position);
    trim(orderId);
    if(orderId.empty())
    {
        orderId = promptForString("Order Id#");
        trim(orderId);
    }
    // discard leading # if any
    if(orderId[0] == '#')
    {
        orderId = orderId.substr(1);
        trim(orderId);
        if(orderId.empty())
        {
            std::cerr << "--Expecting #orderID" << std::endl;
            return false;
        }
    }

    if(orderId[0] == '-') // relative addressing
    {
        int32_t orderOffset = toInt32(orderId);
        if(orderOffset == INVALID_INT32)
        {
            std::cerr << "--Expecting orderID or offset" << std::endl;
            return false;
        }
        uint32_t orderNumber = orderIdSeed_  + 1 + orderOffset;
        orderId = std::to_string(orderNumber);
    }
    return findExistingOrder(orderId, order, book);
}

bool Market::findExistingOrder(const std::string & orderId, OrderPtr & order, OrderBookPtr & book)
{
    auto orderPosition = orders_.find(orderId);
    if(orderPosition == orders_.end())
    {
        std::cerr << "--Can't find OrderID #" << orderId << std::endl;
        return false;
    }

    order = orderPosition->second;
    std::string symbol = order->symbol();
    book = findBook(symbol);
    if(!book)
    {
        std::cerr << "--No order book for symbol" << symbol << std::endl;
        return false;
    }
    return true;
}

/////////////////////////////////////
// Implement OrderListener interface

void 
Market::on_accept(const OrderPtr& order)
{
//todo
    order->onAccepted();
    std::cout << "\tAccepted: " <<*order<< std::endl;
}

void 
Market::on_reject(const OrderPtr& order, const char* reason)
{
    //todo
    order->onRejected(reason);
    std::cout << "\tRejected: " <<*order<< ' ' << reason << std::endl;

}

void 
Market::on_fill(const OrderPtr& order, 
    const OrderPtr& matched_order, 
    liquibook::book::Quantity fill_qty, 
    liquibook::book::Cost fill_cost)
{
    order->onFilled(fill_qty, fill_cost);
    matched_order->onFilled(fill_qty, fill_cost);
    std::cout << (order->is_buy() ? "\tBought: " : "\tSold: ") 
        << fill_qty << " Shares for " << fill_cost << ' ' <<*order<< std::endl;
    std::cout << (matched_order->is_buy() ? "\tBought: " : "\tSold: ") 
        << fill_qty << " Shares for " << fill_cost << ' ' << *matched_order << std::endl;

    //todo
}

void 
Market::on_cancel(const OrderPtr& order)
{
    order->onCancelled();
    std::cout << "\tCanceled: " << *order<< std::endl;
    //todo
}

void Market::on_cancel_reject(const OrderPtr& order, const char* reason)
{
    order->onCancelRejected(reason);
    std::cout << "\tCancel Reject: " <<*order<< ' ' << reason << std::endl;
    //todo
}

void Market::on_replace(const OrderPtr& order, 
    const int32_t& size_delta, 
    liquibook::book::Price new_price)
{
    order->onReplaced(size_delta, new_price);
    std::cout << "\tModify " ;
    if(size_delta != liquibook::book::SIZE_UNCHANGED)
    {
        std::cout << " QUANTITY  += " << size_delta;
    }
    if(new_price != liquibook::book::PRICE_UNCHANGED)
    {
        std::cout << " PRICE " << new_price;
    }
    std::cout <<*order<< std::endl;
    //todo
}

void 
Market::on_replace_reject(const OrderPtr& order, const char* reason)
{
    order->onReplaceRejected(reason);
    std::cout << "\tReplace Reject: " <<*order<< ' ' << reason << std::endl;
    //todo
}

////////////////////////////////////
// Implement TradeListener interface

void 
Market::on_trade(const OrderBook* book, 
    liquibook::book::Quantity qty, 
    liquibook::book::Cost cost)
{
    std::cout << "\tTrade: " << qty <<  ' ' << book->symbol() << " Cost "  << cost  << std::endl;

    //todo
}

/////////////////////////////////////////
// Implement OrderBookListener interface

void 
Market::on_order_book_change(const OrderBook* book)
{
    std::cout << "\tChange: " << ' ' << book->symbol() << std::endl;

    //todo
}


}  // namespace orderentry