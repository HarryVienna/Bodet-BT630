#include "flip_clock.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <time.h> 

static const char* TAG = "FLIP_CLOCK";

// Konstruktor
FlipClock::FlipClock(gpio_num_t enable_pin, gpio_num_t input1_pin, gpio_num_t input2_pin,
                     int pulse_width_ms, int pulse_interval_ms)
    : _enable_pin(enable_pin),
      _input1_pin(input1_pin),
      _input2_pin(input2_pin),
      _pulse_width_ms(pulse_width_ms),
      _pulse_interval_ms(pulse_interval_ms),
      _clock_time(0),
      _polarity_level(0)
{
    ESP_LOGI(TAG, "FlipClock-Objekt wird erstellt.");
    _init_gpio();

    // Dummy-Impulse senden, um den Motor zu initialisieren
    ESP_LOGI(TAG, "Sende 2 Initialisierungsimpulse zur Motor-Polarisierung...");
    sendPulses(2);
}

// Private Helferfunktion zur GPIO-Initialisierung
void FlipClock::_init_gpio() {
    gpio_reset_pin(_enable_pin);
    gpio_set_direction(_enable_pin, GPIO_MODE_OUTPUT);
    gpio_reset_pin(_input1_pin);
    gpio_set_direction(_input1_pin, GPIO_MODE_OUTPUT);
    gpio_reset_pin(_input2_pin);
    gpio_set_direction(_input2_pin, GPIO_MODE_OUTPUT);
    ESP_LOGI(TAG, "GPIOs initialisiert.");
}

// Öffentliche Methode zum Senden von Impulsen
void FlipClock::sendPulses(int count) {
    _send_pulses_internal(count);
}

// Interne Methode, die die eigentliche Arbeit macht
void FlipClock::_send_pulses_internal(int count) {
    if (count <= 0) return;
    ESP_LOGI(TAG, "Sende %d Impuls(e)...", count);

    for (int i = 0; i < count; i++) {
        // Polarität für Schrittmotor umkehren
        gpio_set_level(_input1_pin, _polarity_level);
        gpio_set_level(_input2_pin, !_polarity_level);

        // Enable-Pin für die Dauer des Impulses aktivieren
        gpio_set_level(_enable_pin, 1);
        vTaskDelay(pdMS_TO_TICKS(_pulse_width_ms));
        
        gpio_set_level(_enable_pin, 0);
        vTaskDelay(pdMS_TO_TICKS(_pulse_interval_ms));  

        // Polarität für den nächsten Impuls wechseln
        _polarity_level = !_polarity_level;
    }
}

// Setzt die Startzeit der Uhr
void FlipClock::setTime(uint8_t hour, uint8_t minute) {
    struct tm real_time_info;
    time_t now;
    time(&now);
    localtime_r(&now, &real_time_info);

    struct tm clock_time_info = real_time_info;
    clock_time_info.tm_hour = hour;
    clock_time_info.tm_min = minute;
    clock_time_info.tm_sec = 0;
    
    _clock_time = mktime(&clock_time_info);

    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%d.%m.%Y %H:%M:%S", &clock_time_info);
    ESP_LOGI(TAG, "FlipClock Start-Zeitstempel gesetzt auf: %s", time_buf);
}

// Prüft die Zeit und aktualisiert die Uhr
void FlipClock::update() {
    time_t now;
    time(&now);
    
    // Prüfen, ob die Zeit gültig ist (nach 2023)
    if (now < 1672531200) {
        return; 
    }

    time_t current_minute_floored = (now / 60) * 60;

    if (_clock_time < current_minute_floored) {
        int minutes_to_flip = (current_minute_floored - _clock_time) / 60;
        ESP_LOGI(TAG, "Uhr geht %d Minute(n) nach. Sende Impuls(e)...", minutes_to_flip);
        
        _send_pulses_internal(minutes_to_flip);
        
        _clock_time = current_minute_floored;
        ESP_LOGI(TAG, "Zeit ist wieder synchron.");
    }
}