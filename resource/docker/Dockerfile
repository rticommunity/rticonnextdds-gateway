# (c) 2026 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved.
# RTI grants Licensee a license to use, modify, compile, and create derivative
# works of the software solely for use with RTI Connext DDS.  Licensee may
# redistribute copies of the software, provided that all such copies are subject
# to this license. The software is provided "as is", with no warranty of any
# type, including any warranty for fitness for any purpose. RTI is under no
# obligation to maintain or support the software.  RTI shall not be liable for
# any incidental or consequential damages arising out of the use or inability
# to use the software.
###############################################################################
#
# Build:
#   $ docker build \
#       -t dis-container:latest \
#       -f resource/docker/Dockerfile \
#       resource/docker/
#
# Run the image with default entry point:
#   $ docker run --rm \
#       -v <path to rti_license.dat>:/opt/rti.com/rti_connext_dds-7.3.0/rti_license.dat \
#       -v <path to rticonnextdds-gateway>:/gateway \
#       -it --entrypoint /bin/bash \
#       dis-container:latest
# See container:
#   $ sudo docker ps -a
# See container logs:
#   $ docker logs dis_container
# Stop container main process:
#   $ docker stop dis_container
# Delete container:
#   $ docker rm  dis_container
# Delete image:
#   $ docker rmi dis_container:7.3.0
#
###############################################################################

ARG BASE_IMAGE_REPOSITORY=""
ARG BASE_IMAGE_NAME=ubuntu
ARG BASE_IMAGE_TAG=22.04
ARG CONNEXT_VERSION=7.3.0
ARG CONNEXT_ARCH=x64Linux4gcc7.3.0
ARG NDDSHOME=/opt/rti.com/rti_connext_dds-${CONNEXT_VERSION}

# Install stage ----------------------------------------------------------------
FROM ${BASE_IMAGE_REPOSITORY}${BASE_IMAGE_NAME}:${BASE_IMAGE_TAG} as install-stage

ARG BASE_IMAGE_REPOSITORY
ARG BASE_IMAGE_NAME
ARG BASE_IMAGE_TAG
ARG CONNEXT_VERSION
ARG RTI_LICENSE_AGREEMENT_ACCEPTED
ARG CONNEXT_ARCH
ARG NDDSHOME

RUN apt update && mkdir -p /tmp

RUN apt update && \
    apt install -y make \
	    cmake \
        ca-certificates \
        curl \
        git \
        g++ \
        gcc \
	    unixodbc \
	    unixodbc-dev \
	    odbc-mariadb \
	    odbc-postgresql\
        && apt-get clean \
        && rm -rf /var/lib/apt/lists

# Install the RTI Connext Debian Package
RUN curl -sSL -o /usr/share/keyrings/rti-official-archive.gpg \
        https://packages.rti.com/deb/official/repo.key

RUN printf -- "deb [arch=%s signed-by=%s] %s %s main\n" \
    $(dpkg --print-architecture) \
    /usr/share/keyrings/rti-official-archive.gpg \
    https://packages.rti.com/deb/official \
    $(. /etc/os-release && echo ${VERSION_CODENAME}) \
        > /etc/apt/sources.list.d/rti.list

RUN export DEBIAN_FRONTEND=noninteractive \
    RTI_LICENSE_AGREEMENT_ACCEPTED=${RTI_LICENSE_AGREEMENT_ACCEPTED} \
    && apt-get update \
    && apt-get install -y \
    rti-connext-dds-${CONNEXT_VERSION} \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists

####### Final steps

RUN echo "BASE_IMAGE_REPOSITORY=${BASE_IMAGE_REPOSITORY}" >> ${NDDSHOME}/.env \
    && echo "BASE_IMAGE_NAME=${BASE_IMAGE_NAME}" >> ${NDDSHOME}/.env \
    && echo "BASE_IMAGE_TAG=${BASE_IMAGE_TAG}" >> ${NDDSHOME}/.env \
    && echo "CONNEXT_VERSION=${CONNEXT_VERSION}" >> ${NDDSHOME}/.env \
    && echo "CONNEXT_ARCH=${CONNEXT_ARCH}" >> ${NDDSHOME}/.env \
    && echo "NDDSHOME=${NDDSHOME}" >> ${NDDSHOME}/.env

FROM scratch AS final-stage

ARG NDDSHOME

ENV ODBCSYSINI=/etc
ENV NDDSHOME=${NDDSHOME}
ENV ODBCHOME=/plugins/adapters/dia/resources/
ENV ODBCINI=/gateway/plugins/adapters/dia/resources/odbc/odbc.ini

COPY --from=install-stage / /

WORKDIR /gateway
