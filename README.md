# Liquibook

Open source order matching engine from [OCI](https://objectcomputing.com/)

Liquibook provides the low-level components that make up an order matching engine. 

Order matching is the process of accepting buy and sell orders for a security (or other fungible asset) and matching them to allow 
trading between parties who are otherwise unknown to each other.

An order matching engine is the heart of every financial exchange, 
and may be used in many other circumstances including trading non-financial assets, serving as a test-bed for trading algorithms, etc.

A typical Liquibook-based application might look something like this:
![Market Application](doc/Images/MarketApplication.png)

In addition to the order matching process itself, Liquibook can be configured
to maintain an "depth book" that records the number of open orders and total quantity
represented by those orders at individual price levels.  

#### Example of an depth book
* Symbol XYZ: 
  * Buy Side: 
    * $53.20 per share: 1203 orders; 150,398 shares.
    * $53.19 per share: 87 orders; 63,28 shares
    * $52.00 per share 3 orders; 2,150 shares
  * Sell Side
    * $54.00 per share 507 orders; 120,700 shares
    * etc...            

## Order properties supported by Liquibook.

Liquibook is aware of the following order properties.

* Side: Buy or Sell
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

The only required properties are side, quantity and price.  Default values are available for the other properties.

The application can define addtional properties on the order object as necessary.  These properties will have no impact on the behavior of Liquibook.
 
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
  * Notification of changes in the depth book (if enabled)
    * Depth book changed
    * Best Bid or Best Offer (BBO) changed.


## Performance
* Liquibook is written in C++ using modern, high-performance techniques. This repository includes
the source of a test program that can be used to measure Liquibook performance.
  * Benchmark testing with this program shows sustained rates of  
__2.0 million__ to __2.5 million__ inserts per second. 

As always, the results of this type of performance test can vary depending on the hardware and operating system on which you run the test, so use these numbers as a rough order-of-magnitude estimate of the type of performance your application can expect from Liquibook. 

## Works with Your Design
* Allows an application to use smart or regular pointers to orders.
* Compatible with existing order model, 
  * Requires a trivial interface which can be added to or wrapped around an existing Order object.
* Compatible with existing identifiers for securities, accounts, exchanges, orders, fills

## Example
This repository contains two complete example programs.  These programs can be used to evaluate Liquibook to see if it meets your needs. They can also be used as models for your application or even incorporated directly into your application thanks to the liberal license under which Liquibook is distributed.

The examples are:
* Depth feed publisher and subscriber
  * Generates orders that are submitted to Liquibook and publishes the resulting market data.
  * Uses [QuickFAST](https://github.com/objectcomputing/quickfast) to publish the market data

* Manual Order Entry
  * Allows orders and other requests to be read from the console or submitted by a script (text file)
  * Submits these to Liquibook.
  * Displays the notifications received from Liquibook to the console or to a log file.
  * [Detailed instructions are in the README_ORDER_ENTRY.md file.]( README_ORDER_ENTRY.md)

# Building Liquibook
The good news is you don't need to build Liquibook.  The core of Liquibook is a header-only library, so you can simply
add Liquibook/src to your include path then `#include <book/order_book.h>` to your source, and Liquibook will be available
to be used in your application.

However, this repository includes tests and example programs for Liquibook.  These programs need to be compiled and built in order to run them.  The remainder of this section describes how to do this.

## Dependencies
Liquibook has no runtime dependencies.  It will run in any environment that can run C++ programs.

To build the Liquibook test and example programs from source you need to create makefiles (for linux, et al.) or Project and Solution files for Windows Visual Studio.

Liquibook uses MPC to create these platform-dependent files from a common build definition:
* [MPC](https://github.com/objectcomputing/MPC) for cross-platform builds.

  MPC itself is written in perl, so your environment needs a working Perl compiler.  Most linux systems already have this. If you need a Perl compiler on Windows, OCI recommends [Active State Perl V5.x or later](http://www.activestate.com/)

If you wish to build the unit tests for Liquibook, you will also need the boost test library:
* [BOOST](http://www.boost.org/) (optional) for unit testing.

One of the example programs (publish and subscribe to market data) uses QuickFAST to encode and decode market data messages.  If you wish to run this example you need QuickFAST:
* [QuickFAST](https://github.com/objectcomputing/quickfast) (optional) for building the example depth feed publisher/subscriber.

  QuickFAST his its own dependencies which are described on its web page.

## Submodule Note
The Assertive test framework was used in previous versions, but it is no longer needed.  
If you have imported this submodule to support previous versions, you may delete the liquibook/test/unit/assertiv directory.

## Getting ready to build the tests and example programs.

### Boost Test
If you want to run the Liquibook unit tests (highly recommended!) you should install and/or build boost test before trying to build Liquibook.  Boost test is used in the multifile-test mode rather than simple header-only mode so the compiled boost test library must be available.

Please follow the instructions on the [boost web site](http://www.boost.org/) for building/installing the library in your environment.
When you are done you should export the $BOOST_ROOT environment varialble.  

Because of the many boost build options, please check to be sure that the include files and library files are in the expected locations.
MPC expects to find:
*  Include files in $BOOST_ROOT/include/boost
*  Library files in $BOOST_ROOT/lib


If you prefer not to install boost you can edit the liquibook.features file to change the appropriate line to say `boost=0`  This will disable building the unit tests.

### QuickFAST
The publish and subscribe example program uses QuickFAST.  If you want to run this example program, please see the [QuickFAST web site](https://github.com/objectcomputing/quickfast) to download and build this library.

Set the environment variable $QUICKFAST_ROOT to point to the location where you installed and build QuickFAST.

Before running MPC you should also edit the file liquibook.features to set the value QuickFAST=1

If you do not plan to run this example program, set the environment variable QUICKFAST_ROOT to liquibook/noQuickFAST.

## Building Liquibook on Linux

The env.sh script uses the readlink program which is present on most Linux/Unix systems. 
If you don't have readlink, set the $LIQUIBOOK_ROOT environment variable the directory containing liquibook before running env.sh

Open a shell and type:

<pre>
$ cd liquibook
$ . ./env.sh
$ $MPC_ROOT/mwc.pl -type make liquibook.mwc
$ make depend
$ make all
</pre>

### Output from build
* The Liquibook test and example libraries will be in $LIQUIBOOK_ROOT/lib
* The Liquibook example programs will be in $LIQUIBOOK_ROOT/bin
* The Liquibook test programs will be in $LIQUIBOOK_ROOT/bin/test

## Building Liquibook examples and test programs with Visual Studio

Use the following commands to set up the build environment and create Visual Studio project and solution files.
Note if you are using MinGW or other linux-on-Windows techniques, follow the Linux instructions; however, OCI does not normally test this.

<pre>
> cd liquibook
> copy winenv.bat w.bat #optional if you want to keep the original
                        # note that single character batch file names are ignored in 
                        # .getignore so the customized
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

Liquibook should work on any platform with a modern C++ compiler (supporting at least C++11.)  

The MPC program used to create the build files and the Boost library used in the tests and some of the examples support a wide variety of platforms.  

See the [MPC documentation](https://github.com/objectcomputing/MPC) for details about using MPC in your enviornment.

See the [Boost website](http://www.boost.org/) for details about using Boost in your environment.
