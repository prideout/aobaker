[![Build Status](https://travis-ci.org/prideout/aobaker.svg?branch=master)](https://travis-ci.org/prideout/aobaker)

This little public domain tool does the following:

- Generates reasonable UV's for an arbitrary mesh.
- Creates a texture representing ambient occlusion.

The first bullet uses Thekla's parameterization code; the second bullet uses Intel's embree library for raytracing.

![](https://github.com/prideout/aobaker/blob/master/suzanne.jpg)

**Similar Tools**

Texture coordinates can be generated using Microsoft's open source [UVAtlas](https://github.com/Microsoft/UVAtlas) tool, which is a somewhat Windows-oriented project.

[libigl](http://libigl.github.io/libigl) can do mesh parameterization and ambient occlusion baking.  It is protected by the Mozilla Public License.

[IBLBaker](http://www.derkreature.com/iblbaker/) does something completely different, but also looks interesting.

**Usage**

```
$ aobaker --help

  Usage: aobaker [options] input_mesh.obj

  Options:
    --outmesh   OBJ file to produce (result.obj)
    --atlas     PNG file to produce (result.png)
    --sizehint  Controls resolution of atlas (32)
    --nsamples  Quality of ambient occlusion (128)
    --gbuffer   Generate diagnostic images
    --ids       Add a chart id to the alpha channel
    --multiply  Scales the AO values by a constant (1.0)
    --version   Output version
    --help      Output help
```

**Building in OS X**

Homebrew is the easiest way to install the dependencies:

```bash
$ cp ~/git/aobaker/embree.rb /usr/local/Library/Formula
$ brew install embree tbb cmake
```

This is optional, but you might want to use a compiler that supports OpenMP, which is a bit tricky if you've already got clang installed.  Here's how I did it:

```bash
brew uninstall gcc
brew install gcc --without-multilib
export CC=/usr/local/bin/gcc-5
export CXX=/usr/local/bin/g++-5
```

Now you're ready to build!  Here's how to invoke CMake so that it puts all the messy stuff into a folder called `build`:
```bash
$ cmake . -Bbuild      # Generate Makefile
$ cmake --build build  # Build the Makefile
```
