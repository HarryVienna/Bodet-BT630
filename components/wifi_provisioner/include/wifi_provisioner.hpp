#pragma once
#include <string>
#include "esp_err.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h" 
#include "esp_sntp.h"

class WifiProvisioner {
public:
    WifiProvisioner();
    ~WifiProvisioner();

    /**
     * @brief Startet den blockierenden Provisionierungs-Modus.
     * * Startet AP, DNS- und Webserver und wartet, bis der Benutzer
     * im Captive Portal seine Daten eingegeben hat. Die Server werden danach wieder beendet.
     *
     * @param ap_ssid Der Name des WLAN-Netzwerks, das der ESP32 aufspannt.
     * @param persistent_storage Wenn true, werden die Daten dauerhaft im NVS gespeichert.
     * Wenn false, werden sie nur temporär im Speicher gehalten.
     * @param ap_password Optionales Passwort für den Access Point.
     * @return esp_err_t ESP_OK bei Erfolg.
     */
    esp_err_t start_provisioning(const std::string& ap_ssid, bool persistent_storage, const std::string& ap_password = "");

    /**
     * @brief Prüft, ob gültige WLAN-Zugangsdaten dauerhaft im NVS gespeichert sind.
     */
    bool is_provisioned();

    /**
     * @brief Lädt dauerhaft gespeicherte Zugangsdaten aus dem NVS in die Klasse.
     * @return esp_err_t ESP_OK bei Erfolg.
     */
    esp_err_t get_credentials();

    /**
     * @brief Versucht, eine Verbindung mit den in der Klasse gespeicherten Zugangsdaten herzustellen.
     *
     * @param hostname Der gewünschte Name des Geräts im Netzwerk.
     * @return esp_err_t ESP_OK bei Erfolg der Initiierung.
     */
    esp_err_t connect_sta(const char* hostname);

    /**
     * @brief Konfiguriert und startet den SNTP-Client zur Zeitsynchronisierung.
     * * @note Wird idealerweise automatisch aufgerufen, sobald eine WLAN-Verbindung steht.
     */
    void synchronize_time();

    /**
     * @brief Prüft, ob die Systemzeit erfolgreich mit einem NTP-Server synchronisiert wurde.
     * @return true, wenn die Zeit synchron ist, andernfalls false.
     */
    bool is_time_synchronized() const;

    /**
     * @brief Gibt die vom Benutzer während des Provisionings eingestellte Stunde zurück.
     * @return Die Stunde (0-23) oder -1, wenn noch keine eingestellt wurde.
     */
    int get_provisioned_hour() const;

    /**
     * @brief Gibt die vom Benutzer während des Provisionings eingestellte Minute zurück.
     * @return Die Minute (0-59) oder -1, wenn noch keine eingestellt wurde.
     */
    int get_provisioned_minute() const;

private:
    void init_wifi_();

    esp_err_t start_ap_(const std::string& ssid, const std::string& password);
    void stop_ap_(); 

    esp_err_t start_web_server_();
    void stop_web_server_();

    static esp_err_t root_get_handler_(httpd_req_t *req);
    static esp_err_t scan_get_handler_(httpd_req_t *req);
    static esp_err_t save_post_handler_(httpd_req_t *req);
    static esp_err_t style_get_handler_(httpd_req_t *req);
    static esp_err_t captive_portal_handler_(httpd_req_t *req);

    esp_err_t load_credentials_from_nvs_(std::string& ssid, std::string& password, std::string& timezone);
    esp_err_t save_credentials_to_nvs_();

    // Statische Methoden für C-Callbacks
    static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void time_sync_notification_cb(struct timeval *tv);

    // Member-Variablen zum Speichern der Zugangsdaten
    std::string _ssid;
    std::string _password;
    std::string _timezone;

    // Member-Variablen zum Speichern der Stunde und Minute, die beim Provisioning mitgeschickt werden
    int _provisioned_hour = -1;   // Initialisiert mit einem ungültigen Wert
    int _provisioned_minute = -1; // Initialisiert mit einem ungültigen Wert

    // Gab es schon eine erfolgreiche Verbindung
    bool _has_been_connected = false;

    // Dieses Flag wird vom SNTP-Callback auf 'true' gesetzt.
    bool _is_time_synced = false;

    // Flag, um die SNTP-Initialisierung zu verfolgen
    bool _sntp_initialized = false; 

    // Konfigurations-Flags
    bool _persistent_storage = false;
    
    // FreeRTOS-Objekte für die Synchronisation
    EventGroupHandle_t _provisioning_event_group;
    
    httpd_handle_t server_ = nullptr;
    bool wifi_initialized_ = false;

    // Statischer Pointer, damit der C-Callback auf die Instanz zugreifen kann
    static WifiProvisioner* s_instance;
};