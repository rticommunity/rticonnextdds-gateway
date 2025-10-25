def WS = [:]

pipeline {
    agent {
        label "docker-win"
    }
    environment {
        CI = "true"
    }
    options {
        disableConcurrentBuilds()
        buildDiscarder(logRotator(numToKeepStr: '5'))
    }
    parameters {
        booleanParam(
            name: 'DOCKER_USE_DEFAULT_CONFIG',
            defaultValue: true,
            description: 'Derive all DOCKER_* parameters from the default CI configuration. All other DOCKER_* parameters will be ignored if this is set to true.'
        )
        booleanParam(
            name: 'CMAKE_USE_DEFAULT_CONFIG',
            defaultValue: true,
            description: 'Derive all CMAKE_* parameters from the default CI configuration. All other CMAKE_* parameters will be ignored if this is set to true.'
        )
        booleanParam(
            name: 'PLUGINS_USE_DEFAULT_CONFIG',
            defaultValue: true,
            description: 'Derive all PLUGINS_* parameters from the default CI configuration. All other PLUGINS_* parameters will be ignored if this is set to true.'
        )
        booleanParam(
            name: 'TEST_USE_DEFAULT_CONFIG',
            defaultValue: true,
            description: 'Derive all TEST_* parameters from the default CI configuration. All other TEST_* parameters will be ignored if this is set to true.'
        )
        string(
            name: 'DOCKER_TEST_PLATFORM',
            defaultValue: '',
            description: 'Docker platform to use for testing (e.g., linux/amd64, linux/arm64).'
        )
        string(
            name: 'DOCKER_TEST_IMAGE',
            defaultValue: '',
            description: 'Docker image to use for testing.'
        )
        string(
            name: 'DOCKER_CONNEXTDDS_ARCH',
            defaultValue: '',
            description: 'Connext architecture to use.'
        )
        string(
            name: 'DOCKER_CONNEXTDDS_VERSION',
            defaultValue: '',
            description: 'Version of Connext to use.'
        )
        string(
            name: 'DOCKER_CONNEXTDDS_HOST_URL',
            defaultValue: '',
            description: 'URL of the Connext Pro host installer to use.'
        )
        string(
            name: 'DOCKER_CONNEXTDDS_TARGET_URLS',
            defaultValue: '',
            description: 'Comma-separated list of URLs to additional Connext Pro target installers.'
        )
        booleanParam(
            name: 'PLUGINS_ALL',
            defaultValue: true,
            description: 'Enable all gateway plugins by default. If set to false, you must enable individual plugins using other options below.'
        )
        booleanParam(
            name: 'PLUGINS_MODBUS',
            defaultValue: false,
            description: 'Explicitly enable the ModBus adapter plugin.'

        )
        booleanParam(
            name: 'PLUGINS_MQTT',
            defaultValue: false,
            description: 'Explicitly enable the MQTT adapter plugin.'
        )
        booleanParam(
            name: 'PLUGINS_KAFKA',
            defaultValue: false,
            description: 'Explicitly enable the Kafka adapter plugin.'
        )
        booleanParam(
            name: 'PLUGINS_FWD',
            defaultValue: false,
            description: 'Explicitly enable the Forwarding processor plugin.'
        )
        booleanParam(
            name: 'PLUGINS_TSFM_FIELD',
            defaultValue: false,
            description: 'Explicitly enable the Field transformation plugin.'
        )
        booleanParam(
            name: 'PLUGINS_TSFM_JSON',
            defaultValue: false,
            description: 'Explicitly enable the JSON transformation plugin.'
        )
        booleanParam(
            name: 'PLUGINS_TSFM_PROTOBUF',
            defaultValue: false,
            description: 'Explicitly enable the Protobuf transformation plugin.'
        )
        booleanParam(
            name: 'PLUGINS_TSFM_SEQUENCE2ARRAY',
            defaultValue: false,
            description: 'Explicitly enable the Sequence2Array transformation plugin.'
        )
        string(
            name: 'CMAKE_BUILD_TYPE',
            defaultValue: 'Release',
            description: 'CMake build type.'
        )
        booleanParam(
            name: 'CMAKE_OPT_ENABLE_TESTS',
            defaultValue: true,
            description: 'Explicitly enable building of included tests.'
        )
        booleanParam(
            name: 'CMAKE_OPT_ENABLE_EXAMPLES',
            defaultValue: true,
            description: 'Build the included examples.'
        )
        booleanParam(
            name: 'CMAKE_OPT_ENABLE_SSL',
            defaultValue: false,
            description: 'Enable support for SSL/TLS.'
        )
        booleanParam(
            name: 'CMAKE_OPT_ENABLE_LOG',
            defaultValue: false,
            description: 'Enable logging to stdout.'
        )
        booleanParam(
            name: 'CMAKE_OPT_ENABLE_TRACE',
            defaultValue: false,
            description: 'Enable trace logging of internal functions calls.'
        )
        booleanParam(
            name: 'CMAKE_OPT_ENABLE_PROTOBUF_BUILD',
            defaultValue: false,
            description: 'Build the included copy of Protobuf instead of relying on a system installation.'
        )
        string(
            name: 'CMAKE_EXTRA_ARGS',
            defaultValue: '',
            description: 'Additional arguments to be passed to cmake when configuring the build.'
        )
        string(
            name: 'CMAKE_EXTRA_BUILD_ARGS',
            defaultValue: '',
            description: 'Additional arguments to be passed to cmake when building the project.'
        )
        string(
            name: 'CMAKE_TEST_TIMEOUT',
            defaultValue: '300',
            description: 'Maximum time (in seconds) to run all tests.'
        )
        booleanParam(
            name: 'TEST_RUN',
            defaultValue: true,
            description: 'Run tests.'
        )
        string(
            name: 'TEST_INCLUDE',
            defaultValue: '',
            description: 'Regular expression to filter tests to run (passed to ctest with -R).'
        )
        string(
            name: 'TEST_EXCLUDE',
            defaultValue: '',
            description: 'Regular expression to filter tests to exclude (passed to ctest with -E).'
        )
    }
    stages {
        stage('Workspace Checkout') {
            steps {
                script {
                    rtigatewayCiWorkspace WS: WS, params: params, env: env
                    echo "Loaded CI configuration '${WS.CONFIG_NAME}' for job '${env.JOB_NAME}': ${WS.CONFIG}"
                    echo "Workspace configuration: ${WS}"
                    currentBuild.displayName = "${currentBuild.displayName} ${WS.DISPLAY_NAME}"
                }

                bat """
                    git submodule update --init --recursive
                """
            }
        }

        stage('Workspace Build') {
            agent {
                dockerfile {
                    filename '.ci/docker/build_test-win.dockerfile'
                    label 'rticonnextdds-gateway'
                    additionalBuildArgs """\
                        --platform ${WS.TEST_PLATFORM} \
                        --build-arg TEST_IMAGE=${WS.TEST_IMAGE} \
                        --build-arg CONNEXTDDS_VERSION=${WS.CONNEXTDDS_VERSION}  \
                        --build-arg CONNEXTDDS_HOST_URL="${WS.CONNEXTDDS_HOST_URL}" \
                        --build-arg CONNEXTDDS_TARGET_URLS="${WS.CONNEXTDDS_TARGET_URLS.join(' ')}"
                    """.stripIndent().trim()
                    args """\
                        --platform ${WS.TEST_PLATFORM}
                    """.stripIndent().trim()
                    reuseNode true
                }
            }
            stages {
                stage('Build') {
                    steps {
                        bat """
                            CALL C:\\connextdds\\rti_connext_dds-${WS.CONNEXTDDS_VERSION}\\resource\\scripts\\rtisetenv_${WS.CONNEXTDDS_ARCH}.bat

                            echo on

                            cmake --version

                            cmake \
                                -B build \
                                -DCMAKE_BUILD_TYPE=${WS.CMAKE_BUILD_TYPE} \
                                -DCMAKE_INSTALL_PREFIX=${env.WORKSPACE}\\install \
                                -DRTIGATEWAY_ENABLE_TESTS=${WS.CMAKE_OPT_ENABLE_TESTS ? "ON" : "OFF"} \
                                -DRTIGATEWAY_ENABLE_EXAMPLES=${WS.CMAKE_OPT_ENABLE_EXAMPLES ? "ON" : "OFF"} \
                                -DRTIGATEWAY_ENABLE_DOCS=OFF \
                                -DRTIGATEWAY_ENABLE_PDF_DOCS=OFF \
                                -DRTIGATEWAY_ENABLE_SSL=${WS.CMAKE_OPT_ENABLE_SSL ? "ON" : "OFF"} \
                                -DRTIGATEWAY_ENABLE_LOG=${WS.CMAKE_OPT_ENABLE_LOG ? "ON" : "OFF"} \
                                -DRTIGATEWAY_ENABLE_TRACE=${WS.CMAKE_OPT_ENABLE_TRACE ? "ON" : "OFF"} \
                                -DRTIGATEWAY_ENABLE_PROTOBUF_BUILD=${WS.CMAKE_OPT_ENABLE_PROTOBUF_BUILD ? "ON" : "OFF"} \
                                -DRTIGATEWAY_ENABLE_ALL=${WS.PLUGINS_ALL ? "ON" : "OFF"} \
                                ${WS.PLUGINS_MODBUS ? "-DRTIGATEWAY_ENABLE_MODBUS=ON" : ""} \
                                ${WS.PLUGINS_MQTT ? "-DRTIGATEWAY_ENABLE_MQTT=ON" : ""} \
                                ${WS.PLUGINS_KAFKA ? "-DRTIGATEWAY_ENABLE_KAFKA=ON" : ""} \
                                ${WS.PLUGINS_FWD ? "-DRTIGATEWAY_ENABLE_FWD=ON" : ""} \
                                ${WS.PLUGINS_TSFM_FIELD ? "-DRTIGATEWAY_ENABLE_TSFM_FIELD=ON" : ""} \
                                ${WS.PLUGINS_TSFM_JSON ? "-DRTIGATEWAY_ENABLE_TSFM_JSON=ON" : ""} \
                                ${WS.PLUGINS_TSFM_PROTOBUF ? "-DRTIGATEWAY_ENABLE_TSFM_PROTOBUF=ON" : ""} \
                                ${WS.PLUGINS_TSFM_SEQUENCE2ARRAY ? "-DRTIGATEWAY_ENABLE_TSFM_SEQUENCE2ARRAY=ON" : ""} \
                                ${WS.CMAKE_EXTRA_ARGS} \
                                .
                        """

                        bat """
                            cmake \
                                --build build \
                                --config ${WS.CMAKE_BUILD_TYPE} \
                                --target install \
                                ${WS.CMAKE_EXTRA_BUILD_ARGS}
                        """
                    }
                }

                stage('Test') {
                    when { expression { WS.TEST_RUN } }
                    steps {
                        script {
                            try {
                                bat """
                                    CALL C:\\connextdds\\rti_connext_dds-${WS.CONNEXTDDS_VERSION}\\resource\\scripts\\rtisetenv_${WS.CONNEXTDDS_ARCH}.bat

                                    echo on

                                    cd build

                                    ctest \
                                        -C ${WS.CMAKE_BUILD_TYPE} \
                                        --timeout ${WS.CMAKE_TEST_TIMEOUT} \
                                        --output-on-failure \
                                        --output-junit test_results.xml \
                                        ${WS.TEST_INCLUDE ? "-R ${WS.TEST_INCLUDE}" : ""} \
                                        ${WS.TEST_EXCLUDE ? "-E ${WS.TEST_EXCLUDE}" : ""}
                                """
                            } catch (err) {
                                unstable "Some tests have failed."
                            }
                        }

                    }
                    post {
                        always {
                            junit 'build/test_results.xml'
                        }
                    }
                }
            }
        }
    }

    post {
        cleanup {
            cleanWs()
        }
    }
}