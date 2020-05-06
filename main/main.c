#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//#include "esp_event.h"
//#include "esp_event_loop.h"
#include "esp_wifi.h"
//#include "nvs_flash.h"

#include "udp_logging.h"

#include "esp_log.h"
static const char *TAG = "main";


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);


    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void app_main(void) {
//    nvs_flash_erase();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_ap_config = {
        .ap = {
            .authmode = WIFI_AUTH_OPEN,
            .max_connection = 4,
        },
    };
    uint8_t mac;
    esp_read_mac(&mac, 1);
    snprintf((char *)wifi_ap_config.ap.ssid, 11, "esp_%02x%02x%02x", (&mac)[3], (&mac)[4], (&mac)[5]);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));                        //must be called before esp_wifi_set_config()
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_ap_config));

    ESP_ERROR_CHECK(esp_wifi_start());
    
    udp_logging_init( "255.255.255.255", 1337);

    while(1) {

        ESP_LOGI(TAG, "Hello ");

        vTaskDelay(2000/portTICK_PERIOD_MS);
    }

}
