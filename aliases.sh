alias init="
    export CC=/usr/local/bin/gcc-5;
    export CXX=/usr/local/bin/g++-5;
    cmake . -Bbuild"
alias initdbg="cmake -DCMAKE_BUILD_TYPE=Debug . -Bbuild"
alias build="cmake --build build"
alias clean="rm -rf build"
alias debug="lldb -or -- build/aobaker suzanne.obj"
alias run="build/aobaker suzanne.obj"
