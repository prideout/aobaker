[![Build Status](https://travis-ci.org/prideout/aobaker.svg?branch=master)](https://travis-ci.org/prideout/aobaker)

This little public domain tool does the following:

- Generates suprisingly reasonable UV's for an arbitrary mesh.
- Creates a texture representing ambient occlusion.

The first bullet uses Thekla's parameterization code; the second bullet uses Intel's embree library for raytracing.

Note that [libigl](http://libigl.github.io/libigl) can also do this kind of stuff, which is protected by the Mozilla Public License.

---

Presently, only OS X is officially supported.  Since CMake is used, other platforms could easily be supported with a few minor tweaks.

Homebrew is the easiest way to install the dependencies.  Simply cd to your local copy of this repo and do this:

```bash
$ cp embree.rb /usr/local/Library/Formula
$ brew install embree tbb cmake
```

Now you're ready to build!  Here's how to invoke CMake so that it puts all the messy stuff into a folder called `build`:
```bash
$ cmake -H. -Bbuild    # Generate Makefile
$ cmake --build build  # Build the Makefile
```

You can now invoke the tool on the test mesh:
```bash
$ build/aobaker torii.obj
```