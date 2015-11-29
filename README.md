[![Build Status](https://travis-ci.org/prideout/aobaker.svg?branch=master)](https://travis-ci.org/prideout/aobaker)

---

Homebrew is the easiest way to install the dependencies.  Simply cd to your local copy of this repo and do this:

```
cp embree.rb /usr/local/Library/Formula
brew install embree tbb cmake
```

Now you're ready to build!  Here's how to invoke CMake so that it puts all the messy stuff into a folder called `build`:
````
cmake -H. -Bbuild
cmake --build build
```
