# Liquibook

Open source order matching engine from [OCI](http://ociweb.com)

Liquibook provides the low-level components that make up an order matching engine. 

Order matching is the process of accepting buy and sell orders for a security (or other fungible asset) and matching them to allow 
trading between parties who are otherwise unknown to each other.

An order matching engine is the heart of every financial exchange, 
and may be used in many other circumstances including trading non-financial assets, serving as a test-bed for trading algorithms, etc.

In addition to the order matching process itself, Liquibook can be configured
to maintain an "order book" that records the number of open orders and total quantity
represented by those orders at individual price levels.  

#### Example of an order book
* Symbol XYZ: 
  * Buy Side: 
    * $53.20 per share: 1203 orders; 150,398 shares.
    * $53.19 per share: 87 orders; 63,28 shares
    * $52.00 per share 3 orders; 2,150 shares
  * Sell Side
    * $54.00 per share 507 orders; 120,700 shares
    * etc...            

## Order properties supported by Liquibook.

The orders managed by Liquibook can include the following properties:

* Buy or Sell
* Quantity
* Symbol to represent the asset to be traded
  * Liquibook imposes no restrictions on the symbol.  It is treated as a simple character string.
* Desired price or "Market" to accept the current price defined by the market.
  * Trades will be generated at the specified price or any better price (higher price for sell orders, lower price for buy orders)
* Stop loss price to hold the order until the market price reaches the specified value.
  * This is often referred to as simply a stop price.
* All or None flag to specify that the entire order should be filled or no trades should happen.
* Immediate or Cancel flag to specify that after all trades that can be made against existing orders on the market have been made, the remainder of the order should be canceled.
  * Note combining All or None and Immediate or Cancel produces an order commonly described as Fill or Kill.
  
## Operations on Orders

In addition to submitting orders, traders may also submit requests to cancel or modify existing orders.  (Modify is also know as cancel/replace)
The requests may succeed or fail depending on previous trades executed against the order.
  
## Notifications returned to the application.

Liquibook will notify the application when significant events occur to allow the application to actually execute the trades identified by Liquibook, and to allow the application to publish market data for use by traders.

The notifications generated include:

* Notifications intended for trader submitting an order:
  * Order accepted 
  * Order rejected
  * Order filled (full or partial)
  * Order replaced
  * Replace request rejected
  * Order canceled
  * Cancel request rejected.
* Notifications intended to be published as Market Data
  * Trade
    * Note this should also trigger the application to do what it needs to do to make the trade happen.
  * Security changed
    * Triggered by any event that effects a security
      * Does not include rejected requests.
  * Notification of changes in the order book (if enabled)
    * Order book changed
    * Best Bid or Best Offer (BBO) changed.


## Performance
* Liquibook is written in C++ using modern, high-performance techniques.
  * Benchmark testing (with source included in this repository) show sustained rates of  
__2.0 million__ to __2.5 million__ inserts per second. 

## Works with Your Design
* Allows an application to use smart or regular pointers to orders.
* Compatible with existing order model, 
  * Requires a trivial interface which can be added to or wrapped around an existing Order object.
* Compatible with existing identifiers for securities, accounts, exchanges, orders, fills

##Example
This repository contains two complete example programs.  These programs can be used to evaluate Liquibook to see if it meets your needs.  
They can also be used as models for your application or even incorporated directly into your application thanks to the
liberal license under which Liquibook is distributed.

The examples are:
* Depth feed publisher and subscriber
  * Generates orders that are submitted to Liquibook and publishes the resulting market data.
  * Uses OCI's [QuickFAST](https://www.ociweb.com/products/quickfast/) to publish the market data

* Manual Order Entry
  * Allows orders and other requests to be read from the console or submitted by a script (text file)
  * Submits these to Liquibook.
  * Displays the notifications received from Liquibook to the console or to a log file.
  * [Detailed instructions are in the README_ORDER_ENTRY.md file.] ( README_ORDER_ENTRY.md)

Build Dependencies
------------------

* [MPC](http://www.ociweb.com/products/mpc) for cross-platform builds.
* [BOOST](http://www.boost.org/) (optional) for unit testing.
* [QuickFAST](https://www.ociweb.com/products/quickfast/) (optional) for building the example depth feed publisher/subscriber.

## Submodule Note
The Assertive test framework was used in previous versions, but it is no longer needed.  
If you have imported this submodule to support previous versions, you may delete the liquibook/test/unit/assertiv directory.

## Linux Build Notes

If you want to run the Liquibook unit tests (highly recommended!) you should install and/or build boost test before trying to build Liquibook.  Boost test is used in the multifile-test mode so the boost test library must be available.

Please follow the instructions on the [boost web site](http://www.boost.org/) for building/installing the library in your environment.
Avoid a common mistake: Be sure you actually install the libraries rather than leaving them in the staging area.

If you prefer not to install boost you can edit the liquibook.features file to change the appropriate line to say `boost=0`  This will disable building the unit tests.

Make sure the $BOOST_ROOT, $QUICKFAST_ROOT (set to liquibook/noQuickFAST if you don't want to use QuickFAST) and $MPC_ROOT environment variables are set, then open a shell

If you don't have readlink, set the $LIQUIBOOK_ROOT environment variable the directory containing liquibook before running env.sh

<pre>
$ cd liquibook
$ . env.sh
$ mwc.pl -type make liquibook.mwc
$ make depend
$ make all
</pre>

## Windows Build Notes

Use the following commands to set up the build environment and create Visual Studio project and solution files.
Note if you are using MinGW or other linux-on-Windows techniques, follow the Linux instructions; however, OCI does not normally test this.

<pre>
> cd liquibook
> copy winenv.bat w.bat #optional if you want to keep the original
                        # note that single character batch file names are ignored in .getignore so the customized
                        # file will not be checked into the git repository (a good thing.)
> edit w.bat            # edit is your choice of text editor
                        # follow the instructions in the file itself.
> w.bat                 # sets and verifies environment variables
> mpc.bat               # generate the visual studio solution and project files.
</pre>

Then:
* Start Visual Studio from the command line by typing liquibook.sln or 
* Start Visual Studio from the Windows menu and use the menu File|Open|Project or Solution to load liquibook.sln.

## For any platform

See other [build notes](BUILD_NOTES.md).
