#!/usr/bin/env bash

set -euo pipefail

COMPOSE_FILE="resource/docker/docker-compose.yml"
DOCKERFILE="resource/docker/Dockerfile"

get_connext_version() {
    if [[ ! -f "${DOCKERFILE}" ]]; then
        echo "ERROR: Dockerfile not found: ${DOCKERFILE}" >&2
        exit 1
    fi
    grep -oP '^\s*ARG\s+CONNEXT_VERSION=\K[^\s]+' "${DOCKERFILE}" | head -1
}

CONNEXT_VERSION="$(get_connext_version)"

usage() {
    cat <<'EOF'
Usage:
  dis_docker_compose.sh <action> --license-file <path/to/rti_license.dat>

Actions:
  build       Build Docker Compose services
  build-run   Build and run Docker Compose services
  run         Run Docker Compose services
  remove      Stop and remove Docker Compose services

Notes:
  - RTI_LICENSE_AGREEMENT_ACCEPTED must be set. Please take a look into DIA documentation section 
    Installation Instructions for more details: 
    https://community.rti.com/static/documentation/gateway/current/adapters/dia/index.html.
  - --license-file is required for build, build-run, and run
EOF
}

require_agreement() {
    local agreement_value="${RTI_LICENSE_AGREEMENT_ACCEPTED:-}"

    if [[ "${agreement_value}" != "accepted" ]]; then
        echo "ERROR: RTI license agreement is not accepted."
        echo "Please take a look into DIA documentation section 'Installation Instructions' for more details:"
        echo " https://community.rti.com/static/documentation/gateway/current/adapters/dia/index.html"
        exit 1
    fi
}

require_compose_file() {
    if [[ ! -f "${COMPOSE_FILE}" ]]; then
        echo "ERROR: Docker Compose file not found: ${COMPOSE_FILE}"
        exit 1
    fi
}

license_file_env_var() {
    # get confirmation of license acceptance

    echo ""
    echo "  *****************************************************************************"
    echo "  * DISCLAIMER"
    echo "  *"
    echo "  * The Database Integration Adapter Docker configuration scripts provided herein instruct"
    echo "  * your system to download and run container images or other software components from third-party"
    echo "  * repositories. These include:"
    echo "  *"
    echo "  * MariaDB"
    echo "  *   (https://hub.docker.com/_/mariadb),"
    echo "  * Postgres"
    echo "  *   (https://hub.docker.com/_/postgres),"
    echo "  * UnixOdbc"
    echo "  *   (http://www.unixodbc.org),"
    echo "  * Mariadb Odbc Connector"
    echo "  *   (https://mariadb.com/kb/en/mariadb-connector-odbc),"
    echo "  * Postgres Odbc Connector"
    echo "  *   (https://odbc.postgresql.org),"
    echo "  *"
    echo "  * License Compliance: These third-party images and software components are subject to"
    echo "  * their own respective license terms, which may differ from your RTI license agreement." 
    echo "  * RTI DOES NOT DISTRIBUTE THESE THIRD-PARTY IMAGES AND SOFTWARE COMPONENTS AND PROVIDES"
    echo "  * THESE SCRIPTS \"AS IS\" FOR CONVENIENCE ONLY. IT IS YOUR RESPONSIBILITY TO ENSURE THAT"
    echo "  * YOUR USE OF THESE THIRD-PARTY IMAGES AND SOFTWARE COMPONENTS COMPLIES WITH THEIR"
    echo "  * APPLICABLE LICENSES (INCLUDING ANY OPEN SOURCE REQUIREMENTS)."
    echo "  *****************************************************************************"
    echo ""

    # get user confirmation
    while true; do
        read -p "Do you wish to continue setting up the Database Integration Adapter Docker configuration[Y/n]? " yn
        case $yn in
            [Yy]* | "" ) # accept "enter" (empty string) as default acceptance
                break
                ;;
            [Nn]* )
                echo "\nDatabase Integration Adapter Docker configuration setup canceled.\n"
                exit 0
                ;;
            * )
                echo "Please answer yes (y) or no (n)."
                ;;
        esac
    done
}

parse_args() {
    if [[ $# -lt 1 ]]; then
        usage
        exit 1
    fi

    ACTION="$1"
    shift

    LICENSE_FILE=""

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --license-file)
                if [[ $# -lt 2 ]]; then
                    echo "ERROR: --license-file requires a value"
                    exit 1
                fi
                LICENSE_FILE="$2"
                shift 2
                ;;
            -h|--help)
                usage
                exit 0
                ;;
            *)
                echo "ERROR: Unknown argument: $1"
                usage
                exit 1
                ;;
        esac
    done

    case "${ACTION}" in
        build|build-run|run|remove)
            ;;
        *)
            echo "ERROR: Unknown action: ${ACTION}"
            usage
            exit 1
            ;;
    esac

    if [[ "${ACTION}" != "remove" ]]; then
        if [[ -z "${LICENSE_FILE}" ]]; then
            echo "ERROR: --license-file is required for action '${ACTION}'"
            exit 1
        fi

        if [[ ! -f "${LICENSE_FILE}" ]]; then
            echo "ERROR: License file not found: ${LICENSE_FILE}"
            exit 1
        fi

        LICENSE_FILE="$(realpath "${LICENSE_FILE}")"
    fi
}

run_docker_compose() {

    case "${ACTION}" in
        build)
            license_file_env_var
            docker compose \
                -f "${COMPOSE_FILE}" \
                build dia
            ;;
        build-run)
            license_file_env_var
            docker compose \
                -f "${COMPOSE_FILE}" \
                build dia
            docker compose -f "${COMPOSE_FILE}" run --rm \
		        -v ${LICENSE_FILE}:/opt/rti.com/rti_connext_dds-${CONNEXT_VERSION}/rti_license.dat \
		        -it --entrypoint /bin/bash dia
            ;;
        run)
            docker compose -f "${COMPOSE_FILE}" run --rm \
		        -v ${LICENSE_FILE}:/opt/rti.com/rti_connext_dds-${CONNEXT_VERSION}/rti_license.dat \
		        -it --entrypoint /bin/bash dia
            ;;
        remove)
            docker compose -f "${COMPOSE_FILE}" down
            ;;
    esac
}

main() {
    require_compose_file
    parse_args "$@"
    require_agreement
    run_docker_compose
}

main "$@"
