set N_ARCH=/

rem set
rem @echo "HOWDY!"

rem Not using the downloaded sources while debugging
rem cd C:\Users\Chris\svn\das2\core\devel\libdas2

nmake.exe /nologo /f makefiles\Windows.mak clean

if %ERRORLEVEL% NEQ 0 (
	EXIT /B 1
)

nmake.exe /nologo /f makefiles\Windows.mak build

if %ERRORLEVEL% NEQ 0 (
	EXIT /B 2
)

nmake.exe /nologo /f makefiles\Windows.mak run_test

if %ERRORLEVEL% NEQ 0 (
	EXIT /B 3
)

nmake.exe /nologo /f makefiles\Windows.mak install

if %ERRORLEVEL% NEQ 0 (
	EXIT /B 4
)
