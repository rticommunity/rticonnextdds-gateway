/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <unistd.h>
  #include <arpa/inet.h>
  #include <sys/socket.h>
  #include <sys/select.h>
#endif

#include "modbus.h"

#define SERVER_ID 17

enum {
    TCP,
    TCP_PI,
    RTU
};

const uint16_t UT_BITS_NB = 0x5;
const uint16_t UT_INPUT_BITS_NB = 0x3;
const uint16_t UT_REGISTERS_NB_MAX = 0x600; //1536
/*
 *  Input registers number:
 *      INPUT_REGISTER_INT8         1
 *      INPUT_REGISTER_INT16        1
 *      INPUT_REGISTER_INT32        2
 *      INPUT_REGISTER_INT64        4
 *      INPUT_REGISTER_FLOAT_ABCD   2
 *      INPUT_REGISTER_FLOAT_BADC   2
 *      INPUT_REGISTER_FLOAT_CDAB   2
 *      INPUT_REGISTER_FLOAT_DCBA   2
 *
 *  Simple types: 16
 *  Arrays of 2 elements each: 16 * 2 = 32
 *  Total input registers: 16 + 32 = 48 --> 0x30
 */
const uint16_t UT_INPUT_REGISTERS_NB = 0x30;

const uint8_t UT_INPUT_BITS_TAB[] = {TRUE, FALSE, TRUE};

static modbus_t *ctx = NULL;
static modbus_mapping_t *mb_mapping = NULL;
static int s = -1;

#define MODBUS_REGISTER_SIZEOF(type) sizeof(type) / sizeof(uint16_t)
#define NUM_ELEMS_SAME_TYPE 3
#define NEXT_MODBUS_REGISTER(i,my_type, next_type) \
    (MODBUS_REGISTER_SIZEOF(my_type)) * ((NUM_ELEMS_SAME_TYPE) - (i)) \
        + (MODBUS_REGISTER_SIZEOF(next_type) * (i))

static void fill_out_input_registers(uint16_t *register_inputs) {
    uint64_t i = 0;
    /* The number of iterations is 3 because we have 3 elements of the same type */
    for (i = 0; i < 3; ++i) {
        uint64_t accumulator = i;
        /* INT8 */
        register_inputs[i] = (int8_t) i + 1;
        accumulator += NEXT_MODBUS_REGISTER(i, uint16_t, uint16_t);
        /* INT16 */
        register_inputs[accumulator] = (int16_t) i + 1; //3
        accumulator += NEXT_MODBUS_REGISTER(i, uint16_t, uint32_t);
        /* INT32 */
        MODBUS_SET_INT32_TO_INT16(register_inputs, accumulator, i + 1); //6
        accumulator += NEXT_MODBUS_REGISTER(i, uint32_t, uint64_t);
        /* INT64 */
        MODBUS_SET_INT64_TO_INT16(register_inputs, accumulator, i + 1); //12
        accumulator += NEXT_MODBUS_REGISTER(i, uint64_t, float);
        /* FLOAT_ABCD */
        modbus_set_float_abcd(
                (float) i + 1.5f,
                &register_inputs[accumulator]); // 24
        accumulator += NEXT_MODBUS_REGISTER(i, float, float);
        /* FLOAT_BADC */
        modbus_set_float_badc(
                (float) i + 1.5f,
                &register_inputs[accumulator]); //30
        accumulator += NEXT_MODBUS_REGISTER(i, float, float);
        /* FLOAT_CDAB */
        modbus_set_float_cdab(
                (float)i + 1.5f,
                &register_inputs[accumulator]); //36
        accumulator += NEXT_MODBUS_REGISTER(i, float, float);
        /* FLOAT_DCBA */
        modbus_set_float_dcba(
                (float)i + 1.5f,
                &register_inputs[accumulator]); //42
    }
}

static void close_socket(int num) {
#ifdef _WIN32
    closesocket(num);
#else
    close(num);
#endif
}

static void close_sigint(int dummy) {
    if (s != -1) {
        close_socket(s);
    }
    if (ctx != NULL) {
        modbus_free(ctx);
        ctx = NULL;
    }
    if (mb_mapping != NULL) {
        modbus_mapping_free(mb_mapping);
        mb_mapping = NULL;
    }

    exit(dummy);
}

int main(int argc, char *argv[]) {
    int rc;
    int i;
    int use_backend;
    uint8_t *query;
    int header_length;

    int master_socket;
    fd_set refset;
    fd_set rdset;
    /* Maximum file descriptor number */
    int fdmax;

    if (argc > 1) {
        if (strcmp(argv[1], "tcp") == 0) {
            use_backend = TCP;
        } else if (strcmp(argv[1], "tcppi") == 0) {
            use_backend = TCP_PI;
        } else if (strcmp(argv[1], "rtu") == 0) {
            use_backend = RTU;
        } else {
            printf("Usage:\n  %s [tcp|tcppi|rtu] - Modbus server for unit testing\n\n", argv[0]);
            return -1;
        }
    } else {
        /* By default */
        use_backend = TCP;
    }

    if (use_backend == TCP) {
        ctx = modbus_new_tcp("127.0.0.1", 1502);
        query = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
    } else if (use_backend == TCP_PI) {
        ctx = modbus_new_tcp_pi("::0", "1502");
        query = malloc(MODBUS_TCP_MAX_ADU_LENGTH);
    } else {
        ctx = modbus_new_rtu("/dev/ttyUSB0", 115200, 'N', 8, 1);
        modbus_set_slave(ctx, SERVER_ID);
        query = malloc(MODBUS_RTU_MAX_ADU_LENGTH);
    }
    header_length = modbus_get_header_length(ctx);

    mb_mapping = modbus_mapping_new(
            UT_BITS_NB,
            UT_INPUT_BITS_NB,
            UT_REGISTERS_NB_MAX,
            UT_INPUT_REGISTERS_NB);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    /* Initialize input values that's can be only done server side. */
    for (i = 0; i < UT_INPUT_BITS_NB; ++i) {
        mb_mapping->tab_input_bits[i] = UT_INPUT_BITS_TAB[i];
    }
    /* Initialize values of INPUT REGISTERS */
    fill_out_input_registers(mb_mapping->tab_input_registers);

    if (use_backend == TCP) {
        s = modbus_tcp_listen(ctx, 20);
    } else if (use_backend == TCP_PI) {
        s = modbus_tcp_pi_listen(ctx, 1);
        modbus_tcp_pi_accept(ctx, &s);
    } else {
        rc = modbus_connect(ctx);
        if (rc == -1) {
            fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
            modbus_free(ctx);
            return -1;
        }
    }

    signal(SIGINT, close_sigint);

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(s, &refset);

    /* Keep track of the max file descriptor */
    fdmax = s;
    for (;;) {
        rdset = refset;
        if (select(fdmax + 1, &rdset, NULL, NULL, NULL) == -1) {
            perror("Server select() failure.");
            close_sigint(1);
        }

        /* Run through the existing connections looking for data to be
         * read */
        for (master_socket = 0; master_socket <= fdmax; master_socket++) {

            if (!FD_ISSET(master_socket, &rdset)) {
                continue;
            }

            if (master_socket == s) {
                /* A client is asking a new connection */
                socklen_t addrlen;
                struct sockaddr_in clientaddr;
                int newfd;

                /* Handle new connections */
                addrlen = sizeof(clientaddr);
                memset(&clientaddr, 0, sizeof(clientaddr));
                newfd = (int) accept(s, (struct sockaddr *)&clientaddr, &addrlen);
                if (newfd == -1) {
                    perror("Server accept() error");
                } else {
                    FD_SET(newfd, &refset);

                    if (newfd > fdmax) {
                        /* Keep track of the maximum */
                        fdmax = newfd;
                    }
                    printf("New connection from %s:%d on socket %d\n",
                           inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
                }
            } else {
                modbus_set_socket(ctx, master_socket);
                rc = modbus_receive(ctx, query);
                if (rc > 0) {
                    int reply = 0;
                    reply = modbus_reply(ctx, query, rc, mb_mapping);
                } else if (rc == -1) {
                    /* This example server in ended on connection closing or
                     * any errors. */
                    printf("Connection closed on socket %d\n", master_socket);
                    close_socket(master_socket);

                    /* Remove from reference set */
                    FD_CLR(master_socket, &refset);

                    if (master_socket == fdmax) {
                        fdmax--;
                    }
                }
            }
        }
    }

    printf("Quit the loop: %s\n", modbus_strerror(errno));

    if (use_backend == TCP) {
        if (s != -1) {
            close_socket(s);
        }
    }
    modbus_mapping_free(mb_mapping);
    free(query);
    /* For RTU */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
