@REM Copyright (c) 2017 Object Computing, Inc.
@REM All rights reserved.
@REM See the file license.txt for licensing information.

REM Setting up LiquiBook environment

@REM LiquiBook depends on MPC V 3.6 or later. (http://www.ociweb.com/products/mpc)
@REM LiquiBook depends on BOOST V 1.43.0 or later. (http://www.boost.org/)
@REM Assumes VC10(Visual Studio 2010), VC12(Visual Studio 2013), or VC14 (Visual Studio 2015)
@REM installed in the default location (see VC_ROOT below) and VC..COMNTOOLS is set.

@REM Customize this file by setting variables to suit your environment
@REM Also you should customize the LiquiBook.features file to enable particular features on your system.

@REM --------START: DELETE THESE LINES AFTER CUSTOMIZING ----
@ECHO Please make a copy of %~f0 and customize it according to the instructions in the file.
@ECHO You should also customize %~dp0\Liquibook.features
@goto end
@REM --------END: DELETE THESE LINES AFTER CUSTOMIZING ----

@echo off
REM =====================================================================================
REM EDIT THE FOLLOWING LINES OR SET THESE VALUES IN YOUR ENVIRONMENT BEFORE RUNNING SETUP
if "a" == "a%MPC_ROOT%" set MPC_ROOT=c:\MPC\MPC_4_1_22
if "a" == "a%BOOST_VERSION%" set BOOST_VERSION=boost_1_61_0
if "a" == "a%BOOST_ROOT%" set BOOST_ROOT=c:\boost\%BOOST_VERSION%

@REM TODO: For the pub/sub example program set QUICKFAST_ROOT 
@rem to the actual quickfast directory and 
@rem define XEERCESROOT
if "a" == "a%QUICKFAST_ROOT%" set QUICKFAST_ROOT=noQuickFAST
if "a" == "a%XERCESROOT%" set XERCESROOT=C:\Progs\xerces\xerces-c-3.1.1-x86_64-windows-vc-10.0

REM END OF VALUES TO BE SET
REM =====================================================================================

REM Microsoft moved 32 bit apps to a new program files directory on 64 bit systems
set PROGRAM_FILES_X86=Program Files
if exist "C:\Program Files (x86)" set PROGRAM_FILES_X86=Program Files (x86)

REM Verify setup by checking for expected files/directories
set SETUP_CHECKING=MPC_ROOT=%MPC_ROOT%
if not exist "%MPC_ROOT%\mpc.pl" goto setup_is_bad

set SETUP_CHECKING=BOOST_ROOT=%BOOST_ROOT%
if not exist "%BOOST_ROOT%\boost" goto setup_is_bad

REM if you are not running unit tests, delete the REM on the next line
REM goto noBoost
set SETUP_CHECKING=BOOST_ROOT lib=%BOOST_ROOT%\lib
if not exist "%BOOST_ROOT%\lib" goto setup_is_bad
:noBoost

REM If you are not building the pub/sub app remove the REM on the next line
REM goto noPubSub

set SETUP_CHECKING=QUICKFAST_ROOT contains QuickFASTApplication.mpb
if not exist %QUICKFAST_ROOT%\QuickFASTApplication.mpb goto setup_is_bad

set SETUP_CHECKING=XERCESROOT=%XERCESROOT%
if not exist "%XERCESROOT%\lib" goto setup_is_bad

:noPubSub

REM Find visual studio.  
REM You can short-circuit this by setting VCVER before running this
REM However this also skips the check to see if VC is installed in the expected place

rem if VCVER is already set, skip the discovery
if not "a" == "a%VCVER%" goto setup_is_ok

set SETUP_CHECKING=Setup checking visual studio common tools

set VCVER=15
set SETUP_CHECKING=VS150COMNTOOLS=%VS150COMNTOOLS%
set VS_COMMON_TOOLS=%VS150COMNTOOLS%
set VC_ROOT=C:\%PROGRAM_FILES_X86%\Microsoft Visual Studio 15.0\VC\bin
if exist "%VS_COMMON_TOOLS%VSVARS32.BAT" goto setup_is_ok

set VCVER=14
set SETUP_CHECKING=VS140COMNTOOLS=%VS140COMNTOOLS%
set VS_COMMON_TOOLS=%VS140COMNTOOLS%
set VC_ROOT=C:\%PROGRAM_FILES_X86%\Microsoft Visual Studio 14.0\VC\bin
if exist "%VS_COMMON_TOOLS%VSVARS32.BAT" goto setup_is_ok

set VCVER=13
set SETUP_CHECKING=VS130COMNTOOLS=%VS130COMNTOOLS%
set VS_COMMON_TOOLS=%VS130COMNTOOLS%
set VC_ROOT=C:\%PROGRAM_FILES_X86%\Microsoft Visual Studio 13.0\VC\bin
if exist "%VS_COMMON_TOOLS%VSVARS32.BAT" goto setup_is_ok

set VCVER=12
set SETUP_CHECKING=VS120COMNTOOLS=%VS120COMNTOOLS%
set VS_COMMON_TOOLS=%VS130COMNTOOLS%
set VC_ROOT=C:\%PROGRAM_FILES_X86%\Microsoft Visual Studio 12.0\VC\bin
if exist "%VS_COMMON_TOOLS%VSVARS32.BAT" goto setup_is_ok

set VCVER=11
set SETUP_CHECKING=VS110COMNTOOLS=%VS110COMNTOOLS%
set VS_COMMON_TOOLS=%VS110COMNTOOLS%
set VC_ROOT=C:\%PROGRAM_FILES_X86%\Microsoft Visual Studio 11.0\VC\bin
if exist "%VS_COMMON_TOOLS%VSVARS32.BAT" goto setup_is_ok

set VCVER=10
set SETUP_CHECKING=VS100COMNTOOLS=%VS100COMNTOOLS%
set VS_COMMON_TOOLS=%VS100COMNTOOLS%
set VC_ROOT=C:\%PROGRAM_FILES_X86%\Microsoft Visual Studio 10.0\VC\bin
if exist "%VS_COMMON_TOOLS%VSVARS32.BAT" goto setup_is_ok

set VCVER=9
set SETUP_CHECKING=VS90COMNTOOLS=%VS90COMNTOOLS%
set VS_COMMON_TOOLS=%VS90COMNTOOLS%
set VC_ROOT=C:\%PROGRAM_FILES_X86%\Microsoft Visual Studio 9.0\VC\bin
if exist "%VS_COMMON_TOOLS%VSVARS32.BAT" goto setup_is_ok

set VCVER=8
set SETUP_CHECKING=VS80COMNTOOLS=%VS80COMNTOOLS%
set VS_COMMON_TOOLS=%VS80COMNTOOLS%
set VC_ROOT=C:\%PROGRAM_FILES_X86%\Microsoft Visual Studio 8.0\VC\bin
if exist "%VS_COMMON_TOOLS%VSVARS32.BAT" goto setup_is_ok
REM goto setup_is_bad  (you are here) REM Find visual studio.  
rem if VCVER is already set, skip the discovery
if not "a" == "a%VCVER%" goto setup_is_ok

REM goto setup_is_bad  (you are here) 

:setup_is_bad
ECHO Setup check failed: %SETUP_CHECKING%
ECHO Edit the setup.cmd file or change environment variables
goto end

:setup_is_ok
set SETUP_CHECKING=
set LIQUIBOOK_ROOT=%~dp0

call "%VS_COMMON_TOOLS%VSVARS32.BAT" >nul

REM: This avoids growing PATH and INCLUDE every time setup is run
if "a" == "a%BASE_PATH%" set BASE_PATH=%PATH%
if "a" == "a%BASE_INCLUDE%" set BASE_INCLUDE=%INCLUDE%

set RELEASE_PATH=%LiquiBook_ROOT%\bin;%LiquiBook_ROOT%\Output\Release;%MPC_ROOT%;%BOOST_ROOT%\lib;%LiquiBook_ROOT%\lib;%BASE_PATH%
set DEBUG_PATH=%LiquiBook_ROOT%\bin;%LiquiBook_ROOT%\Output\Debug;%MPC_ROOT%;%BOOST_ROOT%\lib;%LiquiBook_ROOT%\lib;%BASE_PATH%

if "a" == "a%QUICKFAST_ROOT%" goto noQuickFAST
set RELEASE_PATH=%RELEASE_PATH%;%QUICKFAST_ROOT%\Output\Release
set DEBUG_PATH=%DEBUG_PATH%;%QUICKFAST_ROOT%\Output\Debug
:noQuickFAST

if "a" == "a%XERCESROOT%" goto noXERCES
set RELEASE_PATH=%RELEASE_PATH%;%XERCESROOT%\bin
set DEBUG_PATH=%DEBUG_PATH%;%XERCESROOT%\bin
:noXERCES

set PATH=%DEBUG_PATH%
set INCLUDE=%BOOST_ROOT%\include;%BASE_INCLUDE%

title LiquiBook
:end


