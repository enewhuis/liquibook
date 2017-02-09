RELEASE NOTES
=============
## Release [TODO: release number] [TODO: release date]
* Reserved for future use

## Release 1.2.0 [TODO: release date]
* Add version number information to source (liquibook::Version class).
* Create manual order entry example/test program (mt_order_entry) 
* Order book knows and uses current market price.
* Repair Market order handling. Market orders can trade with each other at market price. 
* Add STOP LOSS order support.
* Add batch files to run tests. (pt_run and ut_run) [WINDOWS]
* Logging can be directed to any ostream.
* Order book is aware of the symbol it is managing.
* Optional: Order is aware of its own conditions (AON, IOC).
  * #define LIQUIBOOK_ORDER_KNOWS_CONDITIONS to enable.
* Source code reorganization.  
  * Separate files for (some) separate classes.
  * Hide (some) implementation details (may be a breaking change if anyone was depending on them.)

## Release 1.1.1 January 18, 2017
* Fixed the winenv.bat file to do a more comprehensive job of setting up the windows environment.
* Added mpc.bat batch file to run mpc.
* Add winenv_clear.bat batch file to undo the results of winenv.bat so it can be re-run with different environment settings.
* Added a Windows-only replacements for the *nix functions clock_gettime() and sleep()
* Added asio_safe_include.h to include ASIO on windows without nasty error messages.
* Updated .gitignore to be windows/visual studio aware.
* Disabled examples and QuickFAST dependancies in the .features file (by default).
* Partial fixes for warnings on 64 bit builds.
* Partial fixes for warnings about testing non-boolean values in if statements etc.


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
