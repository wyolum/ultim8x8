#ifndef ESP_SERIAL_WIFI_H
#define ESP_SERIAL_WIFI_H

#include <ESP8266WiFi.h>
#include <EEPROM.h>

#define SSID_MAX 32
#define PASS_MAX 64

//These save typing 11-12 characters each time
#define O(t) Serial.print(t)
#define OL(t) Serial.println(t)

//Save as above but F() the strings
//Some O/OL calls include strcat and can't use F()
//So separate options needed
#define OF(t) Serial.print(F(t))
#define OFL(t) Serial.println(F(t))

#define NL() Serial.println()

#define CHAROPT(opt, val) bool(opt == val || (opt+32) == val) //quick checking of single char case-insensitive
#define PROMPT_INPUT_SIZE PASS_MAX //Max WPA Password size is 64, so this is good enough
#define WIFI_WAIT_TIMEOUT 30 //seconds to wait for wifi connect

// Allows writing/reading of config structures direct to EEPROM
template <class T> int EWA(int ee, const T& value)
{
	const byte* p = (const byte*)(const void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		EEPROM.write(ee++, *p++);
	return i;
}

template <class T> int ERA(int ee, T& value)
{
	byte* p = (byte*)(void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		*p++ = EEPROM.read(ee++);
	return i;
}

//main config object
typedef struct __attribute__((__packed__))
{
    bool config; //true if config is confirmed and saved
	char ssid[SSID_MAX];
	char password[PASS_MAX];
	bool encrypted;
    bool advanced;
    IPAddress ip;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns1;
    IPAddress dns2;
} esp_wifi_config_t;
#define CONFIGCHECK 14 //change this if the above struct changes to invalidate old saved configs


class ESPSerialWiFiManager {
    public:
        ESPSerialWiFiManager(int eeprom_size = 512, int eeprom_offset = 0);

        uint8_t begin();
        uint8_t begin(String ssid, String pass = "");

        void run_menu(int timeout = 0);

        uint8_t status();

    private:

        esp_wifi_config_t _network_config;
        esp_wifi_config_t _temp_config;
        bool _dirty_config;

        int _eeprom_size;
        int _eeprom_offset;

        //Config
        void _write_config();
        void _read_config();
        void _set_config(String ssid, String pass, bool enc,
                         bool advanced = false,
                         IPAddress ip = 0U, IPAddress gateway = 0U, IPAddress subnet = 0U,
                         IPAddress dns1 = 0U, IPAddress dns2 = 0U);
        void _save_config(String ssid, String pass, bool enc,
                          bool advanced = false,
                          IPAddress ip = 0U, IPAddress gateway = 0U, IPAddress subnet = 0U,
                          IPAddress dns1 = 0U, IPAddress dns2 = 0U);
        void _reset_config();
        void _commit_config();

        //network management
        bool _wait_for_wifi(bool status = false);
        void _disconnect();
        bool _connect_from_config();
        bool _connect(String ssid, String pass);
        bool _connect_noenc(String ssid);
        bool _connect_enc(String ssid);
        bool _connect_manual();
        bool _connect_wps();
        void _scan_for_networks();
        void _get_advanced();

        //menues / etc
        void _disp_network_details();
        int _print_menu(String * menu_list, int menu_size, int timeout = 0);
        String _prompt(String prompt, char mask = ' ', int timeout = 0);
        int _prompt_int(String prompt, int timeout = 0);
        IPAddress _prompt_ip(String prompt, IPAddress def = 0U);
        bool _prompt_bool(String prompt);
};

#endif ESP_SERIAL_WIFI_H
