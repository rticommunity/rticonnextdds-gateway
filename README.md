# RTI Gateway

The RTI® Gateway is a software component that allows integrating
different types of connectivity protocols with DDS. Integration in this
context means that data flows from different protocols are adapted to
interface with DDS, establishing communication from and to DDS.

RTI Gateway provides an open framework based on the RTI Routing Service SDK
that enables users to easily add communication protocols as needed, in addition
to any other protocol implementations that may be included as part of the
framework.

## Installation

RTI Gateway is distributed in source format. You can clone the
source repository using git:

```sh
git clone --recurse-submodule https://github.com/rticommunity/rticonnextdds-gateway.git
```

The `--recurse-submodule` option is required to clone additional third-party
repositories as submodules.

If you forget to clone the repository with `--recurse-submodule`, simply run the
following command to pull all the dependencies:

```sh
git submodule update --init --recursive
```

## Building

**Requirements**:

- [Connext DDS 6.0.1](https://community.rti.com/content/page/downloads)*
- [cmake 3.10+](https://cmake.org/download/)

In order to build the RTI Gateway components you have to run the following
command:

Linux® and macOS® systems
```sh
mkdir build
cd build
cmake .. -DCONNEXTDDS_DIR=/path/to/connextdds/installation/dir \
    -DCMAKE_INSTALL_PREFIX=../install
cmake --build . -- install
```

By default CMake builds debug artifacts, if you want to build them as 'release',
you should add the following flag when calling CMake:

```sh
-DCMAKE_BUILD_TYPE=Release
```

Windows® systems

```bat
mkdir build
cd build
cmake .. -DCONNEXTDDS_DIR=/path/to/connextdds/installation/dir \
    -DCMAKE_INSTALL_PREFIX=../install \
    -DCMAKE_BUILD_TYPE=Debug|Release \
    -G <generator name>
cmake --build . --config Debug|Release --target install
```

The generator name comes from CMake, you can see a list of the Visual Studio®
generators here: https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#id13

In case the architecture is 64 bits, we should also add `-A x64`. For
example, the following option will generate a solution for Visual Studio 2015
64 bits:

```sh
-G "Visual Studio 14 2015" -A x64
```

For multi-configuration generator, such as Visual Studio solutions, we need to
specify the configuration mode when building it:

```sh
--config Debug|Release
```

## Documentation

The documentation can be found online in the following link:
https://community.rti.com/static/documentation/gateway/6.0.1/index.html

### Building Documentation Locally

**Requirements**:

- [doxygen](https://www.doxygen.nl/download.html)
- [sphinx](https://www.sphinx-doc.org/en/master/usage/installation.html)
- [graphviz] (http://www.graphviz.org/download/): this might be required in
Windows systems to run the command `dot`.
- [docutils 0.14+](https://docutils.sourceforge.io/)

> **NOTE**: you may need to install manually `sphinx_rtd_theme`,
> `breathe` and `docutils`. See below how to install them.

- Installing `sphinx_rtd_theme` 0.5.1:

    ```sh
    pip install sphinx_rtd_theme
    ```

- Installing `breathe`:

    ```sh
    pip install breathe
    ```

- Installing `docutils`:

    ```sh
    pip install docutils
    ```

In order to build the RTI Gateway documentation, you have to add the
`RTIGATEWAY_ENABLE_DOCS` flag when calling CMake:

```sh
mkdir build
cd build
cmake .. -DCONNEXTDDS_DIR=/path/to/connextdds/installation/dir \
    -DCMAKE_INSTALL_PREFIX=../install \
    -DRTIGATEWAY_ENABLE_DOCS=ON
```

The documentation will be generated under the install directory. The following
file contains the RTI Gateway documentation:

- &lt;RTI Gateway folder&gt;/install/doc/index.html

Specific documentation for the plugins can be found in the section
`1.4. RTI Gateway Plugins`.

Further information about how to customize the build can be found in the
section `4. Building from source`.

Linux® is the registered trademark of Linus Torvalds in the U.S. and other countries.