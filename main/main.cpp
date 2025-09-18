#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <time.h> 
#include "slave_clock.hpp"
#include "wifi_provisioner.hpp"

static const char* TAG = "MAIN_APP";

#define PULSE_WIDTH_MS    350  // Pulse duration in milliseconds
#define PULSE_INTERVAL_MS 150  // Time between pulses
#define PULSE_GPIO_ENABLE GPIO_NUM_13 // Pin for Enable of LM293D
#define PULSE_GPIO_INPUT1 GPIO_NUM_27 // Pin for Input1 of LM293D
#define PULSE_GPIO_INPUT2 GPIO_NUM_14 // Pin for Input2 of LM293D


extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Application starting...");

    SlaveClock clock(PULSE_GPIO_ENABLE, PULSE_GPIO_INPUT1, PULSE_GPIO_INPUT2,
                    PULSE_WIDTH_MS, PULSE_INTERVAL_MS);

    WifiProvisioner provisioner;

    if (provisioner.is_provisioned()) {
        provisioner.get_credentials();
    } else {
        provisioner.start_provisioning("Bodet BT 6.30 Setup", false);
    }

    provisioner.connect_sta("Bodet BT 6.30 Clock");

    while(!provisioner.is_time_synchronized()) {

        ESP_LOGI(TAG, "Zeit ist noch nicht mit dem NTP-Server synchronisiert.");

        // Warte eine Sekunde bis zur nächsten Ausgabe
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    clock.setTime(provisioner.get_provisioned_hour(), provisioner.get_provisioned_minute());

    // Hauptschleife
    while (true) {
        clock.update();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}