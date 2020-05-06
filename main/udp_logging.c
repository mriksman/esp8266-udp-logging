#include "udp_logging.h"

#include <string.h>
#include "lwip/sockets.h"

#ifndef UDP_LOGGING_MAX_PAYLOAD_LEN
#define UDP_LOGGING_MAX_PAYLOAD_LEN 1024
#endif

#include "esp_log.h"
static const char *TAG = "udp_logging";

static int udp_log_fd = -1;

//static vprintf_like_t old_function = NULL;
static putchar_like_t old_function = NULL;

static struct sockaddr_in udp_log_socket;
static char buf[UDP_LOGGING_MAX_PAYLOAD_LEN];

void udp_logging_free(void)
{
//	esp_log_set_vprintf( old_function );
    esp_log_set_putchar( old_function );

	shutdown(udp_log_fd, 2);
	close(udp_log_fd);

	udp_log_fd = -1;
}

/*
int udp_logging_vprintf( const char *str, va_list l )
{
	int err = 0;
	int len;
	if (udp_log_fd >= 0)
	{
		len = vsprintf((char*)buf, str, l);
		sendto(udp_log_fd, buf, len, 0, (struct sockaddr *)&udp_log_socket, sizeof(udp_log_socket));
	}
	return old_function( str, l );
}
*/


int udp_logging_putchar( int chr )
{
    size_t len = strlen(buf);
    buf[len] = chr;
    buf[len + 1] = '\0';
    len++;

    if (chr == '\n') {
        // udp send from tiT task can cause a semaphore lock and freeze program.
        char task_name[16];
        char *cur_task = pcTaskGetTaskName(xTaskGetCurrentTaskHandle());
        strncpy(task_name, cur_task, 16);
        task_name[15] = 0;

        if (strncmp(task_name, "tiT", 16) != 0) {
            // if a socket is available
            if (udp_log_fd >= 0)  {
                sendto(udp_log_fd, buf, len, 0, (struct sockaddr *)&udp_log_socket, sizeof(udp_log_socket));
            }
        // 'clear' string. 
        // If it was 'tiT' task, multiple lines will send through in one packet
        buf[0] = '\0';
        }
    }
	return old_function( chr );
}

int udp_logging_init(const char *ipaddr, unsigned long port) {
	struct timeval send_timeout = {1,0};
	if (udp_log_fd >= 0) 	{
		udp_logging_free();
	}
	ESP_LOGI(TAG, "Initializing UDP logging...");
	if( (udp_log_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )	{
		ESP_LOGE(TAG, "Cannot open socket!");
		return -1;
	}

	uint32_t ip_addr_bytes;
	inet_aton(ipaddr, &ip_addr_bytes);

	memset( &udp_log_socket, 0, sizeof(udp_log_socket) );
	udp_log_socket.sin_family = AF_INET;
	udp_log_socket.sin_port = htons( port );
	udp_log_socket.sin_addr.s_addr = ip_addr_bytes;

	int err = setsockopt(udp_log_fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&send_timeout, sizeof(send_timeout));
	if (err < 0) {
		ESP_LOGE(TAG, "Failed to set SO_SNDTIMEO. Error %d", err);
	}

//	old_function = esp_log_set_vprintf(&udp_logging_vprintf);
    old_function = esp_log_set_putchar(&udp_logging_putchar);

	return 0;
}
