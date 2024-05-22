
the entire project in debug, you can run:

    utils/build-script --debug

For documentation of all available arguments, as well as additional usage
information, see the inline help:

    utils/build-script -h

#### Xcode

To build using Xcode, specify the `--xcode` argument on any of the above commands.
Xcode can be used to edit the Swift source code, but it is not currently
fully supported as a build environment for SDKs other than macOS. The generated
Xcode project does not integrate with the test runner, but the tests can be run
with the 'check-swift' target.

#### Build Products

All of the build products are placed in `swift-source/build/${TOOL}-${MODE}/${PRODUCT}-${PLATFORM}/`.
If macOS Swift with Ninja in DebugAssert mode was built, all of the products
would be in `swift-source/build/Ninja-DebugAssert/swift-macosx-x86_64/`. It
helps to save this directory as an environment variable for future use.

    export SWIFT_BUILD_DIR="~/swift-source/build/Ninja-DebugAssert/swift-macosx-x86_64"

#### Ninja

Once the first build has completed, Ninja can perform fast incremental builds of
various products. These incremental builds are a big timesaver when developing
and debugging.

    cd ${SWIFT_BUILD_DIR}
    ninja swift

This will build the Swift compiler, but will not rebuild the standard library or
any other target. Building the `swift-stdlib` target as an additional layer of
testing from time to time is also a good idea. To build just the standard
library, run:

    ninja swift-stdlib

It is always a good idea to do a full build after using `update-checkout`.

#### Using Xcode

To open the Swift project in Xcode, open `${SWIFT_BUILD_DIR}/Swift.xcodeproj`.
It will auto-create a *lot* of schemes for all of the available targets. A
common debug flow would involve:

 - Select the 'swift' scheme.
 - Pull up the scheme editor (⌘⇧<).
 - Select the 'Arguments' tab and click the '+'.
 - Add the command line options.
 - Close the scheme editor.
 - Build and run.

Another option is to change the scheme to "Wait for executable to be launched",
then run the build product in Terminal.

### Swift Toolchains

#### Building

Swift toolchains are created using the script
[build-toolchain](https://github.com/apple/swift/blob/master/utils/build-toolchain). This
script is used by swift.org's CI to produce snapshots and can allow for one to
locally reproduce such builds for development or distribution purposes. A typical 
invocation looks like the following:

```
  $ ./swift/utils/build-toolchain $BUNDLE_PREFIX
```

where ``$BUNDLE_PREFIX`` is a string that will be prepended to the build 
date to give the bundle identifier of the toolchain's ``Info.plist``. For 
instance, if ``$BUNDLE_PREFIX`` was ``com.example``, the toolchain 
produced will have the bundle identifier ``com.example.YYYYMMDD``. It 
will be created in the directory you run the script with a filename 
of the form: ``swift-LOCAL-YYYY-MM-DD-a-osx.tar.gz``.

Beyond building the toolchain, ``build-toolchain`` also supports the 
following (non-exhaustive) set of useful options::

- ``--dry-run``: Perform a dry run build. This is off by default.
- ``--test``: Test the toolchain after it has been compiled. This is off by default.
- ``--distcc``: Use distcc to speed up the build by distributing the c++ part of
  the swift build. This is off by default.

More options may be added over time. Please pass ``--help`` to
``build-toolchain`` to see the full set of options.

#### Installing into Xcode

On macOS if one wants to install such a toolchain into Xcode:

1. Untar and copy the toolchain to one of `/Library/Developer/Toolchains/` or
   `~/Library/Developer/Toolchains/`. E.x.:

```
  $ sudo tar -xzf swift-LOCAL-YYYY-MM-DD-a-osx.tar.gz -C /
  $ tar -xzf swift-LOCAL-YYYY-MM-DD-a-osx.tar.gz -C ~/
```

The script also generates an archive containing debug symbols which
can be installed over the main archive allowing symbolication of any
compiler crashes.

```
  $ sudo tar -xzf swift-LOCAL-YYYY-MM-DD-a-osx-symbols.tar.gz -C /
  $ tar -xzf swift-LOCAL-YYYY-MM-DD-a-osx-symbols.tar.gz -C ~/
```

2. Specify the local toolchain for Xcode's use via `Xcode->Toolchains`.

### Build Failures

Make sure you are using the [correct release](#macos) of Xcode.

If you have changed Xcode versions but still encounter errors that appear to
be related to the Xcode version, try passing `--clean` to `build-script`.

When a new version of Xcode is released, you can update your build without
recompiling the entire project by passing the `--reconfigure` option.

Make sure all repositories are up to date with the `update-checkout` command
described above.

## Testing Swift

See [docs/Testing.md](docs/Testing.md), in particular the section on [lit.py](docs/Testing.md#using-litpy).

## Learning More

Be sure to look through the [docs](https://github.com/apple/swift/tree/master/docs)
directory for more information about the compiler. In particular, the documents
titled [Debugging the Swift Compiler](docs/DebuggingTheCompiler.rst) and
[Continuous Integration for Swift](docs/ContinuousIntegration.md) are very
helpful to understand before submitting your first PR.

### Building Documentation

To read the compiler documentation, start by installing the
[Sphinx](http://sphinx-doc.org) documentation generator tool by running the
command:

    easy_install -U "Sphinx < 2.0"

Once complete, you can build the Swift documentation by changing directory into
[docs](https://github.com/apple/swift/tree/master/docs) and typing `make`. This
compiles the `.rst` files in the [docs](https://github.com/apple/swift/tree/master/docs)
directory into HTML in the `docs/_build/html` directory.

Many of the docs are out of date, but you can see some historical design
documents in the `docs` directory.

Another source of documentation is the standard library itself, located in
`stdlib`. Much of the language is actually implemented in the library
(including `Int`), and the standard library gives some examples of what can be
expressed today.

## Build Dependencies

### CMake
[CMake](https://cmake.org) is the core infrastructure used to configure builds of
Swift and its companion projects; at least version 3.4.3 is required.

On macOS, you can download the [CMake Binary Distribution](https://cmake.org/download),
bundled as an application, copy it to `/Applications`, and add the embedded
command line tools to your `PATH`:

    export PATH=/Applications/CMake.app/Contents/bin:$PATH

On Linux, if you have not already installed Swift's [development
dependencies](#linux), you can download and install the CMake
package separately using the following command:

    sudo apt-get install cmake


### Ninja
[Ninja](https://ninja-build.org) is the current recommended build system
for building Swift and is the default configuration generated by CMake. [Pre-built
packages](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages)
are available for macOS and Linux distributions. You can also clone Ninja
next to the other projects and it will be bootstrapped automatically:

**Via HTTPS**

    git clone https://github.com/ninja-build/ninja.git && cd ninja
    git checkout release
    cat README

**Via SSH**

    git clone git@github.com:ninja-build/ninja.git && cd ninja
    git checkout release
    cat README
