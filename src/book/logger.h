// Copyright (c) 2012, 2013 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
#pragma once

#include <exception>
#include <string>
namespace liquibook { namespace book {
/// @brief Interface to allow application to control error logging
class Logger
{
public:
  virtual void log_exception(const std::string & context, const std::exception& ex) = 0;
  virtual void log_message(const std::string & message) = 0;
};

}}
