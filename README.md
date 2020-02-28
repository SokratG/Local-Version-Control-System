# Local-Version-Control-System
Project is a implementation local version control system like a "git". 

## Build
For building using VS 2019:
* Open solution file
* Properties->C/C++->General->Additional Include Directories - setup path to boost header library.
* Properties->Linker->Input - setup path to boost lib.(Necessary files can be found in "lib" folder project)
* Use Build

## Supported command
* init
* add
* cat-file
* commit
* hash-object
* status
* ls-files
* diff
* ls
* cd
* pwd
* help

## Third party libraries
* <a href="https://github.com/vog/sha1">SHA-1</a>
* <a href="https://www.boost.org">Boost(iostream)</a>
* <a href="https://www.zlib.net">Zlib</a>
* <a href="https://github.com/duckie/difflib">difflib</a>
