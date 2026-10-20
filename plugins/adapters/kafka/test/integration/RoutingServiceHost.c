#include "ndds/ndds_c.h"
#include "routingservice/routingservice_service.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile sig_atomic_t running = 1;
static void interrupted(int number) { (void)number; running = 0; }

/* Use the public embedded Routing Service API, with the same XML and dynamic
 * plugins as the command-line application. Keep SDK monitoring out of this
 * transport test so its setup cannot affect adapter results. */
int main(int argc, char **argv)
{
    struct RTI_RoutingServiceProperty properties = RTI_RoutingServiceProperty_INITIALIZER;
    struct RTI_RoutingService *service;
    struct DDS_Duration_t delay = {0, 100000000};
    int i;
    for (i = 1; i + 1 < argc; i += 2) {
        if (strcmp(argv[i], "-cfgFile") == 0) properties.cfg_file = argv[i+1];
        else if (strcmp(argv[i], "-cfgName") == 0) properties.service_name = argv[i+1];
    }
    if (!properties.cfg_file || !properties.service_name) return 2;
    properties.skip_default_files = DDS_BOOLEAN_TRUE;
    properties.license_file_name = getenv("RTI_LICENSE_FILE");
    signal(SIGINT, interrupted);
    signal(SIGTERM, interrupted);
    service = RTI_RoutingService_new(&properties);
    if (!service) { fprintf(stderr, "Routing Service creation failed\n"); return 1; }
    if (!RTI_RoutingService_start(service)) {
        fprintf(stderr, "Routing Service start failed\n");
        RTI_RoutingService_delete(service);
        return 1;
    }
    puts("Routing Service ready"); fflush(stdout);
    while (running) NDDS_Utility_sleep(&delay);
    if (!RTI_RoutingService_stop(service)) {
        RTI_RoutingService_delete(service);
        return 1;
    }
    RTI_RoutingService_delete(service);
    return 0;
}
