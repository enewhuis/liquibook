#include "Order.h"

namespace orderentry
{

Order::Order(const std::string & id,
    bool buy_side,
    liquibook::book::Quantity quantity,
    std::string symbol,
    liquibook::book::Price price,
    liquibook::book::Price stopPrice,
    bool aon,
    bool ioc)
    : id_(id)
    , buy_side_(buy_side)
    , quantity_(quantity)
    , symbol_(symbol)
    , price_(price)
    , stopPrice_(stopPrice)
    , aon_(aon)
    , ioc_(ioc)
{
}

std::string 
Order::order_id() const
{
    return id_;
}

bool 
Order::is_limit() const
{
    return price() == 0;
}

bool 
Order::is_buy() const
{
    return buy_side_;
}

bool 
Order::is_aon() const
{
    return aon_;
}

bool 
Order::is_ioc() const
{
    return ioc_;
}

bool 
Order::is_stop() const
{
    return stop_price() != 0;
}

std::string 
Order::symbol() const
{
   return symbol_;
}

liquibook::book::Price 
Order::price() const
{
    return price_;
}

liquibook::book::Quantity 
Order::order_qty() const
{
    return quantity_;
}


liquibook::book::Price 
Order::stop_price() const
{
    return stopPrice_;
}


std::ostream & operator << (std::ostream & out, Order order)
{
    out << "[#" << order.order_id() 
        << ' ' << (order.is_buy() ? "BUY" : "SELL") 
        << ' ' << order.order_qty() 
        << ' ' << order.symbol();
    if(order.price() == 0)
    {
        out << " MKT";
    }
    else
    {
        out << " $" << order.price();
    }

    if(order.is_stop())
    {
       out << " STOP " << order.stop_price();
    }

   out  << (order.is_aon() ? " AON" : "")
        << (order.is_ioc() ? " IOC" : "");

   out << ']';
   
   return out;
}


}
