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

    std::string nextToken(const std::vector<std::string> & tokens,size_t & pos)
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
        uint32_t value = strtoul(input.c_str(),&end,10);
        if(*end != '\0')
        {
            value = INVALID_UINT32;
        }
        return value;
    }

    uint32_t toInt32(const std::string & input)
    {
        char * end;
        uint32_t value = strtol(input.c_str(),&end,10);
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
        std::getline(std::cin,input);
        std::transform(input.begin(),input.end(),input.begin(),toupper);
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
        std::getline(std::cin,input);
        return toUint32(input);
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
        s.erase(s.begin(),std::find_if(s.begin(),s.end(),
            std::not1(std::ptr_fun<int,int>(std::isspace))));
    }

    // trim from end (in place)
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(),s.rend(),
            std::not1(std::ptr_fun<int,int>(std::isspace))).base(),s.end());
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
    return "\t(A)dd,\n\t(M)odify,\n\t(C)ancel\n\t(D)isplay\n";
}

void 
Market::help(std::ostream & out)
{
    out << "Add: Create a new order and add it to the book\n"
        << "  Arguments:\n"
        << "     BUY or SELL\n"
        << "     <Quantity>\n"
        << "     <Symbol>\n"
        << "     <Price> or MARKET\n"
        << "     AON            (optional)\n"
        << "     IOC            (optional)\n"
        << "     STOP <Price>   (optional)\n"
        << "     END            (marks end of order)\n"
        << std::endl;

    out << "Modify: Request Modify an existing order\n"
        << "  Arguments:\n"
        << "     <order#>\n"
        << "     PRICE <new price>\n"
        << "     QUANTITY <new initial quantity>\n"
        << std::endl;

    out << "Cancel: Request cancel an existing order\n"
        << "  Arguments:\n"
        << "     <order#>\n"
        << std::endl;

    out << "Display: Display status of an existing order\n"
        << "  Arguments:\n"
        << "     <order#> or <symbol> or \"book\"\n"
        << std::endl;

}

bool 
Market::apply(const std::vector<std::string> & tokens)
{
    const std::string & command = tokens[0];
    if(command == "ADD" || command == "A")
    {
        return doAdd(tokens, 1);
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
        return doDisplay(tokens,1);
    }
    return false;
}


////////
// ADD
bool 
Market::doAdd(const std::vector<std::string> & tokens, size_t pos)
{
    std::cout << "Add" << std::endl;

    //////////////
    // SIDE
    std::string side = nextToken(tokens, pos);
    if(side.empty())
    {
        side = "?";
        while (side != "BUY" && side != "SELL" && !side.empty())
        {
            side = promptForString("BUY or SELL");
        }
    }
    if(side != "BUY" && side != "SELL")
    {
        std::cerr << "--Expecting BUY or SELL" << std::endl;
        return false;
    }

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
        if(!promptForYesNo("Is this a new symbol?"))
        {
            std::cerr << "--Expecting valid symbol" << std::endl;
            return false;
        }
        add_book(symbol);
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
        if(option == "E" || option == "END")
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
    orders_[orderId] = order;

    const liquibook::book::OrderConditions AON(liquibook::book::oc_all_or_none);
    const liquibook::book::OrderConditions IOC(liquibook::book::oc_immediate_or_cancel);
    const liquibook::book::OrderConditions NOC(liquibook::book::oc_no_conditions);

    const liquibook::book::OrderConditions conditions = 
        (aon ? AON : NOC) | (ioc ? IOC : NOC);


    std::cout << "ADDING order:  " << *order << std::endl;

    auto book = find_book(symbol);
    book->add(order, conditions);
    book->perform_callbacks();
    return true;
}

///////////
// CANCEL
bool
Market::doCancel(const std::vector<std::string> & tokens,size_t position)
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
Market::doModify(const std::vector<std::string> & tokens,size_t position)
{
    std::cout << "Modify" << std::endl;
    return true;
}

///////////
// DISPLAY
bool
Market::doDisplay(const std::vector<std::string> & tokens,size_t pos)
{
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
Market::add_book(const std::string & symbol)
{
    OrderBookPtr result = std::make_shared<OrderBook>();
    result->set_order_listener(this);
    result->set_trade_listener(this);
    result->set_order_book_listener(this);
    books_[symbol] = result;
    return result;
}

OrderBookPtr
Market::find_book(const std::string & symbol)
{
    OrderBookPtr result;
    auto entry = books_.find(symbol);
    if(entry != books_.end())
    {
        result = entry->second;
    }
    return result;
}

bool Market::findExistingOrder(const std::vector<std::string> & tokens,size_t position,OrderPtr & order,OrderBookPtr & book)
{
    ////////////////
    // Order ID
    std::string orderId = nextToken(tokens,position);
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

bool Market::findExistingOrder(const std::string & orderId, OrderPtr & order,OrderBookPtr & book)
{
    auto orderPosition = orders_.find(orderId);
    if(orderPosition == orders_.end())
    {
        std::cerr << "--Can't find OrderID #" << orderId << std::endl;
        return false;
    }

    order = orderPosition->second;
    std::string symbol = order->symbol();
    book = find_book(symbol);
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
    std::cout << "\tAccepted: " << *order << std::endl;
}

void 
Market::on_reject(const OrderPtr& order,const char* reason)
{
    //todo
    std::cout << "\tRejected: " << *order << std::endl;

}

void 
Market::on_fill(const OrderPtr& order,
    const OrderPtr& matched_order,
    liquibook::book::Quantity fill_qty,
    liquibook::book::Cost fill_cost)
{
    std::cout << (order->is_buy() ? "\tBought: " : "\tSold: ") 
        << fill_qty << " Shares for " << fill_cost << ' ' << *order << std::endl;
    std::cout << (matched_order->is_buy() ? "\tBought: " : "\tSold: ") 
        << fill_qty << " Shares for " << fill_cost << ' ' << *matched_order << std::endl;

    //todo
}

void 
Market::on_cancel(const OrderPtr& order)
{
    std::cout << "\tCanceled: " << *order << std::endl;
    //todo
}

void Market::on_cancel_reject(const OrderPtr& order,const char* reason)
{
    std::cout << "\tCancel Reject: " << *order << std::endl;
    //todo
}

void Market::on_replace(const OrderPtr& order,
    const int32_t& size_delta,
    liquibook::book::Price new_price)
{
    std::cout << "\tReplace: " << *order << std::endl;
    //todo
}

void 
Market::on_replace_reject(const OrderPtr& order,const char* reason)
{
    std::cout << "\tReplace Reject: " << *order << std::endl;
    //todo
}

////////////////////////////////////
// Implement TradeListener interface

void 
Market::on_trade(const OrderBook* book,
    liquibook::book::Quantity qty,
    liquibook::book::Cost cost)
{
    std::cout << "\tTrade: " << qty <<  " UNKNOWN SECURITY total  " << cost  << std::endl;

    //todo
}

/////////////////////////////////////////
// Implement OrderBookListener interface

void 
Market::on_order_book_change(const OrderBook* book)
{
    std::cout << "\tChange: " << " UNKNOWN SECURITY" << std::endl;

    //todo
}


}  // namespace orderentry