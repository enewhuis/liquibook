@REM Copyright (c) 2009, 2010 Object Computing, Inc.
@REM All rights reserved.
@REM See the file license.txt for licensing information.
@REM
@REM This batch file runs MWC which is part of the MPC package to create
@REM Visual Studio solution and project files.
@REM
@REM Note: -expand_vars -use_env options force MPC to expand $(BOOST_ROOT) into the absolute path.  This avoids
@REM        a problem that happened when people were starting Visual Studio from the Start menu rather than
@REM        from the command line where the BOOST_ROOT environment had been defined.
"%MPC_ROOT%\mwc.pl" -expand_vars -use_env -type vc%VCVER% liquibook.mwc
