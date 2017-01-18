liquibook
=========

Open source limit order book matching engine from [OCI](http://ociweb.com)

## Features
* Low-level components for order matching and aggregate depth tracking
* Memory-efficiency: minimal copying of data to internal structures
* Speed: between __2.0 million__ and __2.5 million__ inserts per second.  See full [performance history](PERFORMANCE.md).

## Flexibility
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
  book::OrderBook&lt;MyOrder*&gt; order_book;

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
* [QuickFAST](https://www.ociweb.com/products/quickfast/) (optional) for building the example publisher/subscriber

## Submodule Note

Assertiv is included as a submodule.  After cloning liquibook, you must:

<pre>
> cd liquibook
> git submodule init
> git submodule update
</pre>

## Linux Build Notes

Make sure the $BOOST_ROOT, $QUICKFAST_ROOT (set to liquibook/noQuickFAST if you don't want to use QuickFAST) and $MPC_ROOT environment variables are set, then open a shell

<pre>
$ cd liquibook
$ . env.sh
$ mwc.pl -type make liquibook.mwc
$ make depend
$ make all
</pre>

If you don't have readlink, set the $LIQUIBOOK_ROOT environment variable before running env.sh

## Windows Build Notes
Use the following commands to set up the build environment and create Visual Studio project and solution files.
Note if you are using MinGW or other linux-on-Windows techniques, follow the Linux instructions; however, OCI does not normally test this.

<pre>
> cd liquibook
> copy winenv.bat w.bat #optional if you want to keep the original
                        # note that single character batch file names are ignored in .getignore so the customized
                        # file will not be checked into the git repository.
> edit w.bat            # edit is your choice of text editor
                        # follow the instructions in the file itself.
> w.bat                 # sets and verifies environment variables
> mpc.bat               # generate the visual studio solution and project files.
</pre>

Then in the same window, start Visual Studio from the command line, opening liquibook.sln
<pre>
> liquibook.sln
</pre>
If Windows does not recognize the *.sln file extension as belonging to VisualStudio,
you may need to provide the path to Visual Studio.

This example is the Visual Studio 2010 Express Edition:
<pre>
> "%VS100COMNTOOLS%\..\IDE\VCExpress.exe" liquibook.sln
</pre>

## For any platform

See other [build notes](BUILD_NOTES.md).
