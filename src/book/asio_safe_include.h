#pragma once
#ifdef _WIN32
//  Set the proper SDK version before including boost/Asio
#   include <SDKDDKVer.h>
//  Note boost/ASIO includes Windows.h. 
#   include <boost/asio.hpp>
#else // _WIN32
#   include <boost/asio.hpp>
#endif //_WIN32