RELEASE NOTES
=============

## Release 1.1.0 July 6, 2013
* Added release notes.
* Added example depth_feed_publisher.
* Added DepthOrderBook, so depth tracking can be part of core of liquibook.
* Implemented generic callback handling, now subclasses may not need to implement.
* Added BboListener, to receive callbacks for top of book events only.
* Added TradeListener, to receive trade callbacks.
* Changed OrderBookListener template param from OrderPtr to OrderBook.
* Fixed some edge case order book callback crashes.

## Release 1.0.0 March 19, 2013
* Initial commit in OCI GitHub repository.
