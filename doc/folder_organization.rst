.. include:: vars.rst

.. _section-folder_organization:

*******************
Folder Organization
*******************

In this section we cover the directory structure of the repository and *install*
directory.

Repository Organization
=======================

- **CMakeLists.txt**:

  - Top-level build script.
  
  - This file is designed to be easily extended by users who would like to
    include new plugins, thanks to its simple, repetitive structure.

- **LICENSE**:

  - Text file stating the licensing of the |RTI_GATEWAY|.

- **README**:

  - Top-level documentation file.
  
  - Includes minimal documentation stating the project description
    and how to build it.

- **plugins/**:

  - Directory for plugin implementations, with subdirectories for each type
    of plugin: adapters, processors, and transformations.

  - Each plugin should be placed in the appropriated subdirectory, and it
    should follow the directory structure described in `Build System: File
    Organization <#plugin-file-organization>`__.

- **common/**:

  - Directory for shared buildable modules that support plugin development.

- **doc/**:

  - Top-level documentation for |RTI_GATEWAY| (i.e. this manual).

  - Plugins and other helper modules store their documentation in a ``doc/``
    subdirectory.

  - Generated manuals are consolidated into a single location during the
    ``install`` step, so that they may be linked from the main documentation.

- **examples/**:

  - Directory for example code, which includes both simple applications, and
    more advanced demo scenarios.

- **resource/**:

  - Directory for support files, such as the repository's ``cmake``-based
    build system, and various, plugin-specific helpers.

- **scripts/**:

  - Utility scripts. An example could be a script that sets the library
    path and runs RS.

- **third-party/**:

  - Directory for third-party dependencies, each one stored in a separate
    subdirectory.

.. _section-build_system:

Build System
============

All |RTI_GATEWAY| plugins shall adopt a common, CMake-based,
build system, which along with the utilities provided by the SDK, will
make it easier to create more portable and consistent plugins.

The build system will expose CMake functions to simplify common build
tasks and to support generation of multiple artifacts:

- A dynamic (or static) library, if the plugin uses a compiled language.

- Test executables.

- Plugin documentation.

The behavior of the build functions is controlled by configuration
variables, and the build system relies on certain conventions,
particularly for naming and file organization, to simplify the
development workflow.

.. _section-plugin_file_organization:

Plugin File Organization
========================

Each plugin will be developed as an independent CMake project stored in
its own directory.

A plugin’s root directory should follow a common structure to organize
content consistently across projects.

The root directory must include a *CMakeLists.txt* to include the
project in a CMake build, and possibly a README file providing basic
documentation about the plugin and additional building requirements.

All other content should be organized in subdirectories following the
naming convention presented in the table below:

.. csv-table:: Plugin File Organization
   :file: _static/csv/plugin_file_organization.csv
   :widths: 25 75
   :header-rows: 1

.. _section-install_directory_organization:

Install Directory Organization
==============================

- **bin/**:

  - Directory for generated binaries

- **doc/**:

  - Directory where all documentation will be located.

- **examples/**:

  - Examples directory, separated by plugin.

- **include/**:

  - Public include directory.
  
  - Includes a folder called ``rti`` and inside that folder, all the public
    header files are separated by plugin.

- **lib/**:

  - Directory where all generated libraries (including third-party) are located.

- **resource/**:

  - Directory for additional resources, separated by plugin.

- **test/**:

  - Directory for all test-related generated artifacts, separated by plugin.

- **third-party/**:

  - Directory for third-party generated artifacts that are not libraries.
