[![Build Status](https://travis-ci.org/prideout/aobaker.svg?branch=master)](https://travis-ci.org/prideout/aobaker)

This little public domain tool does the following:

- Generates suprisingly reasonable UV's for an arbitrary mesh.
- Creates a texture representing ambient occlusion.

The first bullet uses Thekla's parameterization code; the second bullet uses Intel's embree library for raytracing.

**Similar Tools**

[libigl](http://libigl.github.io/libigl) can do mesh parameterization and ambient occlusion baking.  It is protected by the Mozilla Public License.

Texture coordinates can be generated using Microsoft's open source [UVAtlas](https://github.com/Microsoft/UVAtlas) tool, which is a somewhat Windows-oriented project.

---

**Building in OS X**

Homebrew is the easiest way to install the dependencies.  Simply cd to your local copy of this repo and do this:

```bash
$ cp embree.rb /usr/local/Library/Formula
$ brew install embree tbb cmake
```

This is optional, but you'll probably want to use a compiler that supports OpenMP, which is a bit tricky if you've already got clang installed.  Here's how I did it:

```bash
brew uninstall gcc
brew install gcc --without-multilib
export CC=/usr/local/bin/gcc-5
export CXX=/usr/local/bin/g++-5
```

Now you're ready to build!  Here's how to invoke CMake so that it puts all the messy stuff into a folder called `build`:
```bash
$ cmake -H. -Bbuild    # Generate Makefile
$ cmake --build build  # Build the Makefile
```

You can now invoke the tool on the test mesh:
```bash
$ build/aobaker suzanne.obj
```