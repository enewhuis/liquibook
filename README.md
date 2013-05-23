liquibook
=========

Open source limit order book matching engine from [OCI](http://ociweb.com)

## Features
* Low-level components for order matching and aggregate depth tracking
* Memory-efficiency: minimal copying of data to internal structures
* Speed: between __2.0 million__ and __2.5 million__ inserts per second.  See full [performance history](PERFORMANCE.md).

## Flexibility
* Works with or without aggregate depth tracking
* Optional aggregate depth tracking to any number of levels (static) or BBO only
* Works with smart or regular pointers

## Works with Your Design
* Preserves your order model, requiring only trivial interface
* Preserves your identifiers for securities, accounts, exchanges, orders, fills
* Use your threading system (or be single-threaded)
* Use your synchronization method

Minimal Example
---------------
<pre>
  // Create type-specific order book
  book::OrderBook<MyOrder*> order_book;

  // Attach desired event handler(s)
  order_book.set_order_listener(&amp;listener);

  // Create order - my Order class, not Liquibook's!
  MyOrder* order = new MyOrder();

  // Add the order to the order book
  order_book.add(order);

  // Trigger event handlers
  order_book.perform_callbacks();
</pre>

Build Dependencies
------------------

* [MPC](http://www.ociweb.com/products/mpc) for cross-platform builds
* [Assertiv](https://github.com/iamtheschmitzer/assertiv) for unit testing
* BOOST (optional) for shared pointer unit testing only

## Submodule Note

Assertiv is included as a submodule.  After cloning liquibook, you must:

<pre>
> cd liquibook
> git submodule init
> git submodule update
</pre>

## Linux Build Notes

Make sure the $BOOST_ROOT and $MPC_ROOT environment variables are set, then open a shell

<pre>
$ cd liquibook
$ . env.sh
$ mwc.pl -type make liquibook.mwc
$ make depend
$ make all
</pre>

## Windows Build Notes

Make sure the %BOOST_ROOT% and %MPC_ROOT% environment variables are set, then open the Visual Studio Command Prompt of choice (this example is for Visual Studio 2010):
<pre>
> cd liquibook
> winenv.bat
> mwc.pl -type vc10 liquibook.mwc
</pre>

Then in the same window, start Visual Studio from the command line, opening liquibook.sln
<pre>
> liquibook.sln
</pre>
In some cases, you may need to provide the path to Visual Studio - This example is the Visual Studio 2010 Express Edition:
<pre>
> "%VS100COMNTOOLS%\..\IDE\VCExpress.exe" liquibook.sln
</pre>

NOTE: If using Visual Studio 2012, you will be asked to upgrade your project.  This is because MPC does not yet support -type vc11.

See other [build notes](BUILD_NOTES.md).
