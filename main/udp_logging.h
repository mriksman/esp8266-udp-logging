#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_log.h"
#include <string.h>

int udp_logging_init(const char *ipaddr, unsigned long port);

//int udp_logging_vprintf( const char *str, va_list l );
int udp_logging_putchar( int chr );

void udp_logging_free(void);

#ifdef __cplusplus
}
#endif