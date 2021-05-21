/******************************************************************************/
/* (c) 2020 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved. */
/*                                                                            */
/* RTI grants Licensee a license to use, modify, compile, and create          */
/* derivative works of the software solely for use with RTI Connext DDS.      */
/* Licensee may redistribute copies of the software provided that all such    */
/* copies are subject to this license.                                        */
/* The software is provided "as is", with no warranty of any type, including  */
/* any warranty for fitness for any purpose. RTI is under no obligation to    */
/* maintain or support the software.  RTI shall not be liable for any         */
/* incidental or consequential damages arising out of the use or inability to */
/* use the software.                                                          */
/*                                                                            */
/******************************************************************************/

/**
 * @file rtiadapt_mqtt.h
 * @brief Main header file for the MQTT Adapter.
 *
 * This header file should be included by applications wanting to use the
 * MQTT Adapter as a library.
 *
 * @defgroup RtiMqtt_API MQTT Adapter API
 *
 * @addtogroup RtiMqtt_API
 * @{
 */

#ifndef rtiadapt_mqtt_h
    #define rtiadapt_mqtt_h

    /**
     * @defgroup RtiMqtt_Compiler Compiler defines
     * @addtogroup RtiMqtt_Compiler
     * @{
     *
     *
     * @defgroup RtiMqtt_Compiler_User User defines
     * @defgroup RtiMqtt_Compiler_Auto Automatic defines
     *
     */

    /*****************************************************************************
     *                   Target Build Platform Detection
     *****************************************************************************/
    /**
     * @defgroup RtiMqtt_Compiler_Platform Platform defines
     * @ingroup RtiMqtt_Compiler
     *
     * The MQTT Adapter exposes some macros which allow users to more easily
     * write platform-specific code.
     *
     * Code can be guarded on different values of @ref RTI_MQTT_PLATFORM to
     * selectively compile it only on specific target platforms.
     *
     * @addtogroup RtiMqtt_Compiler_Platform
     * @{
     */


    /**
     * @brief Identifier for an unknown platform.
     *
     * This value is defined only as a default when automatic platform detection
     * fails. It should not be used to guard code.
     */
    #define RTI_MQTT_PLATFORM_UNKNOWN 0
    /**
     * @brief Identifier for POSIX compliant platforms.
     *
     */
    #define RTI_MQTT_PLATFORM_POSIX 1
    /**
     * @brief Identifier for Windows platforms.
     *
     */
    #define RTI_MQTT_PLATFORM_WINDOWS 2

    /* If an RTI_MQTT_PLATFORM wasn't specified to the compiler,
       then try to detect it automatically from known macros */
    #ifndef RTI_MQTT_PLATFORM

        #if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)) \
                || defined(__linux__) || defined(__QNXNTO__)

            #define RTI_MQTT_PLATFORM RTI_MQTT_PLATFORM_POSIX

        #elif defined(_MSC_VER) || defined(WIN32)

            #define RTI_MQTT_PLATFORM RTI_MQTT_PLATFORM_WINDOWS

        #else

            /**
             * @ingroup RtiMqtt_Compiler_User
             *
             * @brief The current target build platform.
             *
             * This macro can be specified by users to select the target build
             * platform,
             *
             * If not explicitly set, the value will be automatically detected
             * based on available compiler features, or an error will be thrown
             * by the preprocessor.
             *
             */
            #define RTI_MQTT_PLATFORM RTI_MQTT_PLATFORM_UNKNOWN

            #error "failed to detect type of target platform. Please define RTI_MQTT_PLATFORM"

        #endif

    #endif /* RTI_MQTT_PLATFORM */

    #if RTI_MQTT_PLATFORM != RTI_MQTT_PLATFORM_POSIX \
            && RTI_MQTT_PLATFORM != RTI_MQTT_PLATFORM_WINDOWS
        #warning "unsupported target platform detected"
    #endif

/*****************************************************************************
 *                            Platform Libraries
 *****************************************************************************/

    #include <string.h>

    #if RTI_MQTT_PLATFORM == RTI_MQTT_PLATFORM_POSIX
        #include <pthread.h>
    #elif RTI_MQTT_PLATFORM == RTI_MQTT_PLATFORM_WINDOWS

    #endif

/** @} */


/*****************************************************************************
 *                              SSL/TLS Support
 *****************************************************************************/

    #ifdef DOCUMENTATION_ONLY
        /**
         * @ingroup RtiMqtt_Compiler_User
         *
         * @brief Compiler flag that enables SSL/TLS support.
         *
         * Users should define this flag is they want to establish SSL/TLS
         * connections with MQTT Brokers.
         *
         * Flag @ref RTI_MQTT_USE_SSL will be automatically enabled if this flag
         * is set.
         *
         * @see RTI_MQTT_USE_SSL
         *
         */
        #define RTI_MQTT_ENABLE_SSL
    #endif /* DOCUMENTATION_ONLY */

    #ifndef RTI_MQTT_ENABLE_SSL
        #define RTI_MQTT_USE_SSL 0
    #else
        /**
         * @ingroup RtiMqtt_Compiler_Auto
         *
         * @brief Flag that can be used to guard code related to SSL/TLS
         * functionality.
         *
         * This flag is automatically defined based on the presence (or absence)
         * of
         * @ref RTI_MQTT_ENABLE_SSL. It should not be defined manually by user.
         *
         * @see RTI_MQTT_ENABLE_SSL
         */
        #define RTI_MQTT_USE_SSL 1
    #endif

/** @} */

/*****************************************************************************
 *                          DDS C API Selection
 *****************************************************************************/

    #include "ndds/ndds_c.h"

    /*****************************************************************************
     *                            DDS Type Definitions
     *****************************************************************************/
    #include "rtiadapt_mqtt_types_adapterSupport.h"
    #include "rtiadapt_mqtt_types_clientSupport.h"
    #include "rtiadapt_mqtt_types_messageSupport.h"

    #ifdef DOCUMENTATION_ONLY
        /**
         * @ingroup RtiMqtt_Compiler_User
         *
         * @brief Compiler flag that enable internal use of static DDS types
         * instead of `DDS_DynamicData`.
         *
         * Users should define this flag is they prefer the MQTT Adapter to use
         * static DDS types `RTI_MQTT_Message`, and `RTI_MQTT_KeyedMessage`,
         * instead of `DDS_DynamicData`.
         *
         * If static types are used internally, an additional copy will be
         * performed when data is passed to RTI Routing Service (which requires
         * samples to be represented as `DDS_DynamicData`).
         *
         * Flag @ref RTI_MQTT_USE_STATIC_TYPES will be automatically enabled if
         * this flag is set.
         *
         * @see RTI_MQTT_USE_STATIC_TYPES
         *
         */
        #define RTI_MQTT_STATIC_TYPES
    #endif /* DOCUMENTATION_ONLY */

    #if RTI_MQTT_ENABLE_STATIC_TYPES
        #define RTI_MQTT_USE_STATIC_TYPES 1
    #else
        /**
         * @ingroup RtiMqtt_Compiler_Auto
         *
         * @brief Flag that can be used to enable use of static types instead of
         * `DDS_DynamicData` in the library interfaces.
         */
        #define RTI_MQTT_USE_STATIC_TYPES 0
    #endif /* RTI_MQTT_ENABLE_STATIC_TYPES */

/*****************************************************************************
 *                       RTI MQTT Client Library
 *****************************************************************************/
/**
 * @defgroup RtiMqtt_Client MQTT Client API
 *
 * The MQTT Adapter implements a simple MQTT Client API which wraps an
 * external MQTT client library to expose an interface whose operations more
 * closely match the behavior of a `RTI::RoutingService::MessageWriter` or
 * `RTI::RoutingService::MessageReader`.
 *
 * @addtogroup RtiMqtt_Client
 * @{
 */

    #include "rtiadapt_mqtt_client.h"

/**
 * @}
 */

/*****************************************************************************
 *                       RTI MQTT Adapter Plugin
 *****************************************************************************/
/**
 * @defgroup RtiMqtt_Adapter Routing Service Adapter
 *
 * The MQTT Adapter implements the Routing Service Adapter API to allow the
 * creation of `RTI::RoutingService::Connection`,
 * `RTI::RoutingService::MessageReader`, and
 * `RTI::RoutingService::MessageWriter` objects, and access to the MQTT
 * "data domain".
 *
 * @addtogroup RtiMqtt_Adapter
 * @{
 */

    #include "rtiadapt_mqtt_adapter.h"

    /**
     * @}
     */

    /*****************************************************************************
     *                  RTI MQTT Client Library Properties
     *****************************************************************************/
    /**
     * @defgroup RtiMqtt_Properties Configuration Properties
     *
     * The MQTT Adapter's behavior can be configured with `(key,value)`
     * properties.
     *
     * Accepted configuration keys are exposed as macro string literals.
     *
     * @addtogroup RtiMqtt_Properties
     * @{
     */

    /**
     * @defgroup RtiMqtt_Properties_Client MQTT Client Configuration
     * @defgroup RtiMqtt_Properties_Subscription MQTT Subscription Configuration
     * @defgroup RtiMqtt_Properties_Publication MQTT Publication Configuration
     */

    /**
     * @brief Common prefix for all configuration properties accepted by
     * `RTI_MQTT_Client`.
     * @ingroup RtiMqtt_Properties_Client
     */
    #define RTI_MQTT_PROPERTY_PREFIX_CLIENT "client."

    #if RTI_MQTT_USE_SSL

        /**
         * @brief Common prefix for all configuration properties accepted by
         * `RTI_MQTT_Client` to control supported SSL/TLS functions.
         * @ingroup RtiMqtt_Properties_Client
         */
        #define RTI_MQTT_PROPERTY_PREFIX_CLIENT_SSL \
            RTI_MQTT_PROPERTY_PREFIX_CLIENT "ssl."

    #endif /* RTI_MQTT_USE_SSL */

    /**
     * @brief Common prefix for all configuration properties accepted by
     * `RTI_MQTT_Subscription`.
     * @ingroup RtiMqtt_Properties_Subscription
     */
    #define RTI_MQTT_PROPERTY_PREFIX_SUBSCRIPTION "subscription."

    /**
     * @brief Common prefix for all configuration properties accepted by
     * `RTI_MQTT_Publication`.
     * @ingroup RtiMqtt_Properties_Publication
     */
    #define RTI_MQTT_PROPERTY_PREFIX_PUBLICATION "publication."

    /************************* Client Properties
     * *********************************/

    /**
     * @addtogroup RtiMqtt_Properties_Client
     * @{
     */

    /**
     * @brief Configuration property to specify the "client id" sent by an
     * `RTI_MQTT_Client` to an MQTT Broker during connection.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_ID RTI_MQTT_PROPERTY_PREFIX_CLIENT "id"

    /**
     * @brief Configuration property to select the MQTT protocol version used by
     * an `RTI_MQTT_Client`.
     *
     * Accepted values:
     *
     * - `default`
     * - `3.1`
     * - `3.1.1`
     * - `5`
     */
    #define RTI_MQTT_PROPERTY_CLIENT_PROTOCOL_VERSION \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "protocol_version"

    /**
     * @brief Configuration property to specify a list of MQTT Broker URIs to
     * which an `RTI_MQTT_Client` will try to connect.
     *
     * The value of this property must be a list of semicolon separated URIs.
     *
     * Broker URIs take the form:
     *
     * - TCP Connection: `tcp://&lt;address&gt;:&lt;port&gt;
     * - SSL/TLS Connection: `ssl://&lt;address&gt;:&lt;port&gt;
     */
    #define RTI_MQTT_PROPERTY_CLIENT_SERVERS \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "servers"

    /**
     * @brief Common prefix for configuration properties that control the
     * maximum timeout for establishing a connection between an
     * `RTI_MQTT_Client`, and an MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_CONNECTION_TIMEOUT \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "connection_timeout"

    /**
     * @brief Configuration property to specify the seconds component of the
     * maximum timeout for establishing a connection between an
     * `RTI_MQTT_Client`, and an MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_CONNECTION_TIMEOUT_SECONDS \
        RTI_MQTT_PROPERTY_CLIENT_CONNECTION_TIMEOUT ".sec"

    /**
     * @brief Configuration property to specify the nanoseconds component of the
     * maximum timeout for establishing a connection between an
     * `RTI_MQTT_Client`, and an MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_CONNECTION_TIMEOUT_NANOSECONDS \
        RTI_MQTT_PROPERTY_CLIENT_CONNECTION_TIMEOUT ".nanosec"

    /**
     * @brief Configuration property to specify the maximum number of attempts
     * that an `RTI_MQTT_Client` will perform to establish a connection to an
     * MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_MAX_CONNECTION_RETRIES \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "max_connection_retries"

    /**
     * @brief Common prefix for configuration properties controlling the
     * "keep alive period" between an `RTI_MQTT_Client` and its MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_KEEP_ALIVE_PERIOD \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "keep_alive_period"

    /**
     * @brief Configuration property to specity the seconds component of the
     * "keep alive period" between an `RTI_MQTT_Client` and its MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_KEEP_ALIVE_PERIOD_SECONDS \
        RTI_MQTT_PROPERTY_CLIENT_KEEP_ALIVE_PERIOD ".sec"

    /**
     * @brief Configuration property to specity the nanoseconds component of the
     * "keep alive period" between an `RTI_MQTT_Client` and its MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_KEEP_ALIVE_PERIOD_NANOSECONDS \
        RTI_MQTT_PROPERTY_CLIENT_KEEP_ALIVE_PERIOD ".nanosec"

    /**
     * @brief Configuration property to select whether an `RTI_MQTT_Client`
     * should set the "clean session" flag when establishing a connection to an
     * MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_CLEAN_SESSION \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "clean_session"

    /**
     * @brief Configuration property controlling whether an `RTI_MQTT_Client`
     * should delete all the subscriptions it created before disconnecting from
     * an MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_UNSUBSCRIBE_ON_DISCONNECT \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "unsubscribe_on_disconnect"

    /**
     * @brief Common prefix for configuration properties which control the
     * maximum time for which an `RTI_MQTT_Client` will wait to receive a reply
     * from an MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_MAX_REPLY_TIMEOUT \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "max_reply_timeout"

    /**
     * @brief Configuration property to specify the seconds component of the
     * maximum time for which an `RTI_MQTT_Client` will wait to receive a reply
     * from an MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_MAX_REPLY_TIMEOUT_SECONDS \
        RTI_MQTT_PROPERTY_CLIENT_MAX_REPLY_TIMEOUT ".sec"

    /**
     * @brief Configuration property to specify the nanoseconds component of the
     * maximum time for which an `RTI_MQTT_Client` will wait to receive a reply
     * from an MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_MAX_REPLY_TIMEOUT_NANOSECONDS \
        RTI_MQTT_PROPERTY_CLIENT_MAX_REPLY_TIMEOUT ".nanosec"

    /**
     * @brief Configuration property controlling whether an `RTI_MQTT_Client`
     * will try to re-establish a connection to an MQTT Broker when an existing
     * one is lost.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_RECONNECT \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "reconnect"

    /**
     * @brief Configuration property to specity the maximum number of
     * unacknowledged messages at MQTT Qos 1 or 2, that an `RTI_MQTT_Client` is
     * allowed to have at any time.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_MAX_UNACK_MESSAGES \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "max_unack_messages"

    /**
     * @brief Configuration property to select the type of storage use by an
     * `RTI_MQTT_Client` to persist MQTT session data.
     *
     * Accepted values:
     *
     * - `none`
     * - `durable`
     */
    #define RTI_MQTT_PROPERTY_CLIENT_PERSISTENCE \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "persistence"

    /**
     * @brief Configuration property specifying the path where an
     * `RTI_MQTT_Client` will store persistent MQTT session data.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_PERSISTENCE_STORAGE \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "persistence_storage"

    /**
     * @brief Configuration property to select the "username" that an
     * `RTI_MQTT_Client` will send to an MQTT Broker during connection.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_USERNAME \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "username"

    /**
     * @brief Configuration property to select the "password" that an
     * `RTI_MQTT_Client` will send to an MQTT Broker during connection.
     */
    #define RTI_MQTT_PROPERTY_CLIENT_PASSWORD \
        RTI_MQTT_PROPERTY_PREFIX_CLIENT "password"

    #if RTI_MQTT_USE_SSL
        /**
         * @defgroup RtiMqtt_Properties_Client_Ssl SSL/TLS Configuration
         *
         * @addtogroup RtiMqtt_Properties_Client_Ssl
         * @{
         */


        /**
         * @brief Configuration property to specify the path of a PEM file
         * containing the certificate of the Certificate Authoritative that
         * generated the security material used by an `RTI_MQTT_Client` to
         * establish an SSL/TLS connection to an MQTT Broker.
         */
        #define RTI_MQTT_PROPERTY_CLIENT_SSL_CA \
            RTI_MQTT_PROPERTY_PREFIX_CLIENT_SSL "ca"

        /**
         * @brief Configuration property to specify the path of a PEM file
         * containing the public certificate used by an `RTI_MQTT_Client` to
         * establish an SSL/TLS connection to an MQTT Broker.
         */
        #define RTI_MQTT_PROPERTY_CLIENT_SSL_IDENTITY \
            RTI_MQTT_PROPERTY_PREFIX_CLIENT_SSL "id"

        /**
         * @brief Configuration property to specify the path of a PEM file
         * containing the private key used by an `RTI_MQTT_Client` to establish
         * an SSL/TLS connection to an MQTT Broker.
         */
        #define RTI_MQTT_PROPERTY_CLIENT_SSL_PRIVATE_KEY \
            RTI_MQTT_PROPERTY_PREFIX_CLIENT_SSL "key"

        /**
         * @brief Configuration property to specify the password used to decrypt
         * the private key.
         */
        #define RTI_MQTT_PROPERTY_CLIENT_SSL_PRIVATE_KEY_PASSWORD \
            RTI_MQTT_PROPERTY_PREFIX_CLIENT_SSL "key_password"

        /**
         * @brief Configuration property to specify the set of cyper suites
         * allowed by the SSL/TLS connection.
         *
         * See <a
         * href="https://www.openssl.org/docs/manmaster/man1/ciphers.html#CIPHER_LIST_FORMAT">
         * OpenSSL's documentation</a> for more information on the format of
         * this string.
         *
         * The default value is "ALL", that is, all available cipher suites,
         * excluding those offering no encryption.
         */
        #define RTI_MQTT_PROPERTY_CLIENT_SSL_CYPHER_SUITES \
            RTI_MQTT_PROPERTY_PREFIX_CLIENT_SSL "cypher suites"


        /**
         * @brief Configuration property to specify the SSL/TLS protocol version
         * to use.
         */
        #define RTI_MQTT_PROPERTY_CLIENT_SSL_PROTOCOL_VERSION \
            RTI_MQTT_PROPERTY_PREFIX_CLIENT_SSL "protocol_version"

        /**
         * @brief Configuration property to control whether the certificate
         * provided by the Broker should be verified using the Certificate
         * Authority's certificate.
         */
        #define RTI_MQTT_PROPERTY_CLIENT_SSL_VERIFY_SERVER_CERTIFICATE \
            RTI_MQTT_PROPERTY_PREFIX_CLIENT_SSL "verify_server_certificate"

    /** @} */

    #endif /* RTI_MQTT_USE_SSL */

    /** @} */

    /********************** Subscription Properties
     * ******************************/

    /**
     * @addtogroup RtiMqtt_Properties_Subscription
     * @{
     */

    /**
     * @brief Configuration property to specify the list of topic filters used
     * by an `RTI_MQTT_Subscription` to subscribe to data on an MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_SUBSCRIPTION_TOPICS \
        RTI_MQTT_PROPERTY_PREFIX_SUBSCRIPTION "topics"

    /**
     * @brief Configuration property controlling the maximum MQTT Qos level used
     * to deliver MQTT data to an `RTI_MQTT_Subscription`.
     */
    #define RTI_MQTT_PROPERTY_SUBSCRIPTION_MAX_QOS \
        RTI_MQTT_PROPERTY_PREFIX_SUBSCRIPTION "max_qos"

    /**
     * @brief Configuration property to control the size of the message queue
     * in an `RTI_MQTT_Subscription`.
     */
    #define RTI_MQTT_PROPERTY_SUBSCRIPTION_QUEUE_SIZE \
        RTI_MQTT_PROPERTY_PREFIX_SUBSCRIPTION "queue_size"


    /**
     * @}
     */

    /*********************** Publication Properties
     * ******************************/

    /**
     * @addtogroup RtiMqtt_Properties_Publication
     * @{
     */

    /**
     * @brief Configuration property to specify the MQTT topic to which all
     * messages written by an `RTI_MQTT_Publication` will be published.
     */
    #define RTI_MQTT_PROPERTY_PUBLICATION_TOPIC \
        RTI_MQTT_PROPERTY_PREFIX_PUBLICATION "topic"

    /**
     * @brief Configuration property to specify the MQTT Qos level used to
     * publish all messages written by an `RTI_MQTT_Publication`.
     */
    #define RTI_MQTT_PROPERTY_PUBLICATION_QOS \
        RTI_MQTT_PROPERTY_PREFIX_PUBLICATION "qos"

    /**
     * @brief Configuration property to control whether messages written by an
     * `RTI_MQTT_Publication` should be "retained" by the MQTT Broker.
     */
    #define RTI_MQTT_PROPERTY_PUBLICATION_RETAINED \
        RTI_MQTT_PROPERTY_PREFIX_PUBLICATION "retained"

    /**
     * @brief Configuration property to control whether an
     * `RTI_MQTT_Publication` should determine "topic", "qos", and "retained"
     * information for each written message from the message's "info" attribute,
     * or from the `RTI_MQTT_Publication`'s other properties.
     */
    #define RTI_MQTT_PROPERTY_PUBLICATION_USE_MESSAGE_INFO \
        RTI_MQTT_PROPERTY_PREFIX_PUBLICATION "use_message_info"

    /**
     * @brief Common prefix for configuration properties controlling the maximum
     * time for which an `RTI_MQTT_Publication` will wait to receive an
     * acknowledgement to a message published with MQTT Qos 1 or 2.
     */
    #define RTI_MQTT_PROPERTY_PUBLICATION_MAX_WAIT_TIME \
        RTI_MQTT_PROPERTY_PREFIX_PUBLICATION "max_wait_time"

    /**
     * @brief Configuration property to specify the seconds components of the
     * maximum time for which an `RTI_MQTT_Publication` will wait to receive an
     * acknowledgement to a message published with MQTT Qos 1 or 2.
     */
    #define RTI_MQTT_PROPERTY_PUBLICATION_MAX_WAIT_TIME_SECONDS \
        RTI_MQTT_PROPERTY_PUBLICATION_MAX_WAIT_TIME ".sec"

    /**
     * @brief Configuration property to specify the nanoseconds components of
     * the maximum time for which an `RTI_MQTT_Publication` will wait to receive
     * an acknowledgement to a message published with MQTT Qos 1 or 2.
     */
    #define RTI_MQTT_PROPERTY_PUBLICATION_MAX_WAIT_TIME_NANOSECONDS \
        RTI_MQTT_PROPERTY_PUBLICATION_MAX_WAIT_TIME ".nanosec"


/** @} */

/** @} */

/*****************************************************************************
 *                     Logging macros and messages
 *****************************************************************************/

    #ifdef DOCUMENTATION_ONLY
        /**
         * @ingroup RtiMqtt_Compiler_User
         *
         * @brief Compiler flag that enables the logging module
         *
         * The MQTT Adapter includes a static logging module that can be enabled
         * or disabled at compile-time.
         *
         * When enabled, the logging module can optionally produce more verbose
         * output by enabling trace-level logging.
         *
         * By default, the logging module will be automatically enabled if
         * `NDEBUG` and @ref RTI_MQTT_DISABLE_LOG are not defined.
         *
         * @see RTI_MQTT_USE_LOG RTI_MQTT_DISABLE_LOG
         *
         */
        #define RTI_MQTT_ENABLE_LOG

        /**
         * @ingroup RtiMqtt_Compiler_User
         *
         * @brief Compiler flag that disables the logging module
         *
         * Flag @ref RTI_MQTT_ENABLE_LOG takes precedence if explicitly set.
         *
         * This flag should be used to disable automatic enabling of
         * the logging module if `NDEBUG` is not defined (i.e. for debug
         * builds).
         *
         * @see RTI_MQTT_USE_LOG RTI_MQTT_ENABLE_LOG
         *
         */
        #define RTI_MQTT_DISABLE_LOG
        /**
         * @ingroup RtiMqtt_Compiler_User
         *
         * @brief Compiler flag that enables trace-level output from the logging
         * module.
         *
         * @see RTI_MQTT_USE_TRACE RTI_MQTT_ENABLE_LOG
         *
         */
        #define RTI_MQTT_ENABLE_TRACE

    #endif /* DOCUMENTATION_ONLY */

    #ifndef RTI_MQTT_ENABLE_LOG
        #define RTI_MQTT_USE_LOG 0
    #else
        /**
         * @ingroup RtiMqtt_Compiler_Auto
         *
         * @brief Flag that can be used to guard code related to logging.
         *
         */
        #define RTI_MQTT_USE_LOG 1
    #endif

    #ifndef RTI_MQTT_ENABLE_TRACE
        #define RTI_MQTT_USE_TRACE 0
    #else
        /**
         * @ingroup RtiMqtt_Compiler_Auto
         *
         * @brief Flag used to guard code generating trace-level logging output.
         */
        #define RTI_MQTT_USE_TRACE 1
    #endif

    #include "rtiadapt_mqtt_log.h"

#endif /* rtiadapt_mqtt_h */

/** @} */
