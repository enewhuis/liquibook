RELEASE NOTES
=============
## Release [TODO: release number] [TODO: release date]
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
