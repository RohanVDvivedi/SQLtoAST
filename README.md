# SQLtoAST
As the name suggests this C library converts SQL to an AST, that can be used to pass it to a conforming storage engine to execute it.

## Setup instructions
**Install dependencies :**
 * [Cutlery](https://github.com/RohanVDvivedi/Cutlery)

**Download source code :**
 * `git clone https://github.com/RohanVDvivedi/SQLtoAST.git`

**Build from source :**
 * `cd SQLtoAST`
 * `make clean all`

**Install from the build :**
 * `sudo make install`
 * ***Once you have installed from source, you may discard the build by*** `make clean`

## Using The library
 * add `-lcapp` linker flag, while compiling your application
 * do not forget to include appropriate public api headers as and when needed. this includes
   * `#include<capp/capp.h>`
   * `#include<capp/print_temp.h>`

## Instructions for uninstalling library

**Uninstall :**
 * `cd SQLtoAST`
 * `sudo make uninstall`

## Third party acknowledgements
 * *SQL lexer, internally supported by [flex](https://github.com/westes/flex).*
 * *SQL parser, internally supported by bison, checkout their website [here](https://www.gnu.org/software/bison/).*