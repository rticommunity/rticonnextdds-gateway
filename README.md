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

- [Connext DDS 6.0.1](https://community.rti.com/content/page/downloads)
- [cmake 3.10+](https://cmake.org/download/)

In order to build the RTI Gateway components you have to run the following
command:

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

## Documentation

The documentation can be found online in the following link:
https://community.rti.com/static/documentation/rti-gateway/latest/index.html

### Building Documentation Locally

**Requirements**:

- [doxygen](https://www.doxygen.nl/download.html)
- [sphinx](https://www.sphinx-doc.org/en/master/usage/installation.html)

> **NOTE**: you may need to install manually the `sphinx_rtd_theme` and
> `breathe`. See below how to install them.

- Installing `sphinx_rtd_theme`:

    ```sh
    pip install sphinx_rtd_theme
    ```

- Installing `breathe`:

    ```sh
    pip install breathe
    ```

In order to build the RTI Gateway documentation, you have to add the
`RTIGATEWAY_ENABLE_DOCS` flag when calling CMake:

```sh
mkdir build
cd build
cmake .. -DCONNEXTDDS_DIR=/path/to/connextdds/installation/dir \
    -DCMAKE_INSTALL_PREFIX=../install \
    -DRTIGATEWAY_ENABLE_DOCS=ON
cmake --build . -- install
```

The documentation will be generated under the install directory. The following
file contains the RTI Gateway documentation:

- *&lt;RTI Gateway folder&gt;/install/doc/index.html*

Specific documentation for the plugins can be found find them in the section
`1.4. RTI Gateway Plugins`.

Further information about how to customize the build can be found in the
section `4. Building from source`.
