ARG TEST_IMAGE=ubuntu:22.04
FROM ${TEST_IMAGE}

ENV DEBIAN_FRONTEND=noninteractive
RUN set -e; \
    apt-get update; \
    apt-get install -y \
      build-essential \
      cmake \
      git \
      curl \
      unzip \
      zip \
      python3-venv \
      doxygen \
      mosquitto \
      protobuf-compiler \
      ; \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/*

ARG CONNEXTDDS_VERSION=7.5.0
ARG CONNEXTDDS_REPO=https://packages.rti.com/deb/official
ARG CONNEXTDDS_DISTRO_SUFFIX=

RUN set -e; \
    printf -- "deb [arch=%s, signed-by=%s] %s %s main\n" \
        $(dpkg --print-architecture) \
        /usr/share/keyrings/rti-repo-archive.gpg \
        ${CONNEXTDDS_REPO} \
        $(. /etc/os-release && echo ${VERSION_CODENAME})${CONNEXTDDS_DISTRO_SUFFIX} | \
        tee /etc/apt/sources.list.d/rti.list >/dev/null; \
    curl -sSL ${CONNEXTDDS_REPO}/repo.key -o /usr/share/keyrings/rti-repo-archive.gpg; \
    echo "rti-connext-dds-${CONNEXTDDS_VERSION}-common rti-connext-dds-${CONNEXTDDS_VERSION}/license/accepted select true" | \
        debconf-set-selections; \
    apt-get update; \
    apt-get install -y \
        rti-connext-dds-${CONNEXTDDS_VERSION}-services-routing-dev \
        rti-connext-dds-${CONNEXTDDS_VERSION}-services-routing-dbg \
        rti-connext-dds-${CONNEXTDDS_VERSION}-lib-dbg; \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/*

ENV CONNEXTDDS_VERSION=${CONNEXTDDS_VERSION}
