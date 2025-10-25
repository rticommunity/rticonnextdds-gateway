ARG TEST_IMAGE
FROM ${TEST_IMAGE}

COPY .ci/scripts/connextdds_install.py connextdds_install.py

ARG CONNEXTDDS_VERSION
ARG CONNEXTDDS_HOST_URL
ARG CONNEXTDDS_TARGET_URLS

RUN python connextdds_install.py \
        -V %CONNEXTDDS_VERSION% \
        -H %CONNEXTDDS_HOST_URL% \
        -T %CONNEXTDDS_TARGET_URLS%

RUN setx /M CONNEXTDDS_VERSION "%CONNEXTDDS_VERSION%"

ADD https://mosquitto.org/files/binary/win64/mosquitto-2.0.22-install-windows-x64.exe mosquitto-install.exe

RUN mosquitto-install.exe /S /D=C:\mosquitto

RUN setx /M PATH "%PATH%;C:\mosquitto"

ADD https://github.com/ninja-build/ninja/releases/download/v1.13.1/ninja-win.zip ninja.zip

RUN powershell -Command "Expand-Archive -Path ninja.zip -DestinationPath C:\ninja"

RUN setx /M PATH "%PATH%;C:\ninja"

ADD https://github.com/Kitware/CMake/releases/download/v3.31.9/cmake-3.31.9-windows-x86_64.zip cmake.zip

RUN powershell -Command "Expand-Archive -Path cmake.zip -DestinationPath C:\cmake"

RUN setx /M PATH "C:\cmake\cmake-3.31.9-windows-x86_64\bin;%PATH%"
