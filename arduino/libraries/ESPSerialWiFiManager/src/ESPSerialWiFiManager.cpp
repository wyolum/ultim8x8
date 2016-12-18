#include "ESPSerialWiFiManager.h"

//helper to ensure serial read line is cleared
void _flush_serial(){
    while(Serial.available() > 0){
        Serial.read();
    }
}

// Function below from: http://stackoverflow.com/questions/35227449/convert-ip-or-mac-address-from-string-to-byte-array-arduino-or-c
void parseBytes(String val, char sep, byte* bytes, int maxBytes, int base) {
    const char * str = val.c_str();
    for (int i = 0; i < maxBytes; i++) {
        bytes[i] = strtoul(str, NULL, base);  // Convert byte
        str = strchr(str, sep);               // Find next separator
        if (str == NULL || *str == '\0') {
            break;                            // No more separators, exit
        }
        str++;                                // Point to next character after separator
    }
}

ESPSerialWiFiManager::ESPSerialWiFiManager(int eeprom_size, int eeprom_offset)
{ _eeprom_size = eeprom_size; _eeprom_offset = eeprom_offset; }


uint8_t ESPSerialWiFiManager::begin(){
    EEPROM.begin(_eeprom_size);
    _read_config();
    _dirty_config = false;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    if(_network_config.config){
        _connect_from_config();
    }

    return WiFi.status();
}

uint8_t ESPSerialWiFiManager::begin(String ssid, String pass){
    _set_config(ssid, pass, pass.length() > 0);
    return begin();
}

uint8_t ESPSerialWiFiManager::status(){ return WiFi.status(); }

void ESPSerialWiFiManager::_write_config(){
    EWA(_eeprom_offset + 1, _network_config);
}

void ESPSerialWiFiManager::_read_config(){
    if(EEPROM.read(_eeprom_offset) != CONFIGCHECK)
    {
        _reset_config();
        _write_config();
        EEPROM.write(_eeprom_offset, CONFIGCHECK);
        EEPROM.commit();
    }

    ERA(_eeprom_offset + 1, _network_config);
}

void ESPSerialWiFiManager::_reset_config(){
    memset(&_network_config, 0, sizeof(esp_wifi_config_t));
    _network_config.encrypted = false;
    _network_config.config = false;
    _network_config.advanced = false;
    _dirty_config = true;
}

void ESPSerialWiFiManager::_set_config(String ssid, String pass, bool enc,
                 bool advanced,
                 IPAddress ip, IPAddress gateway, IPAddress subnet,
                 IPAddress dns1, IPAddress dns2){
    _reset_config();

    memcpy(_network_config.ssid, ssid.c_str(), sizeof(char) * ssid.length());
    memcpy(_network_config.password, pass.c_str(), sizeof(char) * pass.length());
    _network_config.encrypted = enc;

    if(advanced){
        _network_config.advanced = advanced;
        _network_config.ip = ip;
        _network_config.gateway = gateway;
        _network_config.subnet = subnet;
        _network_config.dns1 = dns1;
        _network_config.dns2 = dns2;
    }

    _network_config.config = true;
    _dirty_config = true;
}

void ESPSerialWiFiManager::_save_config(String ssid, String pass, bool enc,
                 bool advanced,
                 IPAddress ip, IPAddress gateway, IPAddress subnet,
                 IPAddress dns1, IPAddress dns2){
    _set_config(ssid, pass, enc, advanced, ip, gateway, subnet, dns1, dns2);
    _write_config();
    _read_config();
    OFL("Choose Commit Config for changes to persist reboot.\n");
}

void ESPSerialWiFiManager::_commit_config(){
    OFL("Commiting config to EEPROM... ");
    EEPROM.commit();
    _dirty_config = false;
    OFL("Complete. Changes will now persist through reboot.");
}


bool ESPSerialWiFiManager::_connect_from_config(){
    _disconnect();
    if(_network_config.advanced){
        WiFi.config(_network_config.ip,
                    _network_config.gateway,
                    _network_config.subnet,
                    _network_config.dns1,
                    _network_config.dns2);
    }
    else{
        WiFi.config(0U, 0U, 0U); //Clear out manual config
    }
    if(_network_config.encrypted){
        WiFi.begin(_network_config.ssid, _network_config.password);
    }
    else{
        WiFi.begin(_network_config.ssid);
    }

    bool res = _wait_for_wifi(true);
    if(res) _disp_network_details();
    return res;
}

bool ESPSerialWiFiManager::_connect_noenc(String ssid){
    _get_advanced();
    return _connect(ssid, "");
}

bool ESPSerialWiFiManager::_connect(String ssid, String pass){
    _disconnect();
    if(_temp_config.advanced){
        WiFi.config(_temp_config.ip,
                    _temp_config.gateway,
                    _temp_config.subnet,
                    _temp_config.dns1,
                    _temp_config.dns2);
    }
    else{
        WiFi.config(0U, 0U, 0U); //Clear out manual config
    }
    if(pass.length() > 0)
        WiFi.begin(ssid.c_str(), pass.c_str());
    else
        WiFi.begin(ssid.c_str());
    if(_wait_for_wifi(true)){
        _save_config(ssid, pass, pass.length() > 0,
                    _temp_config.advanced,
                    _temp_config.ip,
                    _temp_config.gateway,
                    _temp_config.subnet,
                    _temp_config.dns1,
                    _temp_config.dns2);
        return true;
    }
    else{
        return false;
    }
}

bool ESPSerialWiFiManager::_connect_enc(String ssid){
    OF("Connect to "); O(ssid); OFL(":");

    String pass = _prompt("Password", '*');

    _get_advanced();

    return _connect(ssid, pass);
}

bool ESPSerialWiFiManager::_connect_manual(){
    OFL("Manual WiFi Config:");
    String ssid = _prompt(F("Enter SSID (Case Sensitive)"));
    String opt = _prompt(F("Encrypted Network? y/n"));
    String pass = "";
    if(CHAROPT(opt[0],'y')){
        pass = _prompt("Password", '*');
    }

    _get_advanced();

    _disconnect();
    return _connect(ssid, pass);
}

bool ESPSerialWiFiManager::_connect_wps(){
    _disconnect();
    OFL("Push the WPS button on your access point now.");
    String opt = _prompt("Press Enter when complete (q to abort)");
    if(CHAROPT(opt[0], 'q')) return false;
    OFL("Attempting WPS connection. May take some time...");
    if(WiFi.beginWPSConfig()){
        String ssid = WiFi.SSID();
        if(ssid.length() > 0){
            OL("\nSuccess! Connected to network " + ssid);
            NL();
            _disp_network_details();
            NL();
            _save_config(ssid, WiFi.psk(), true);
            return true;
        }
        else{
            return false;
        }
    }
}

void ESPSerialWiFiManager::_scan_for_networks(){
    int i, opt, n;
    String opt_s;
    bool inv_opt = false;

    while(true){

        if(!inv_opt){
            OFL("Starting network scan...\n");
            n = WiFi.scanNetworks();
        }

        inv_opt = false;
        if(n == 0){
            OFL("\nNo Avaliable Networks!\nChoose Option Below.");
        }
        else{
            O(n); OFL(" networks found:");
            OFL("==================");
            for(i=0; i < n; i++){
                O(i + 1);O(": ");O(WiFi.SSID(i));
                O(" (");O(WiFi.RSSI(i));O(")");
                OL((WiFi.encryptionType(i) == ENC_TYPE_NONE)?"":"*");
            }
            OFL("==================");
        }

        NL();
        OFL("s: Scan Again");
        OFL("q: Quit Network Scan");

        opt_s  = _prompt("");

        if(CHAROPT(opt_s[0], 'q')){ return; } //exit scan
        else if(CHAROPT(opt_s[0], 's')){ continue; }
        else{
            opt = opt_s.toInt();
            if(opt < 1 || opt >= n + 2){
                OFL("Invalid Menu Option!");
                inv_opt = true;
            }
            else{
                    opt = opt - 1;
                    switch (WiFi.encryptionType(opt)) {
                        case ENC_TYPE_WEP:
                        case ENC_TYPE_TKIP:
                        case ENC_TYPE_CCMP:
                        case ENC_TYPE_AUTO:
                            if(_connect_enc(WiFi.SSID(opt))){
                                _disp_network_details();
                                return;
                            }
                            break;
                        case ENC_TYPE_NONE:
                            if(_connect_noenc(WiFi.SSID(opt))){
                                _disp_network_details();
                                return;
                            }
                            break;

                    opt_s = _prompt("Scan Again? y/n");
                    if(CHAROPT(opt_s[0], 'y')){ continue; }
                    else{ return; }
                  }
            }
        }
    }
}

bool ESPSerialWiFiManager::_wait_for_wifi(bool status){
    int c = 0;
    if(status) OL("Connecting to " + WiFi.SSID());
    while ((WiFi.status() == WL_IDLE_STATUS || WiFi.status() == WL_DISCONNECTED) && c < WIFI_WAIT_TIMEOUT) {
        delay(500);
        if(status) Serial.print(".");
        c++;
    }

    NL();
    int ws = WiFi.status();
    if (ws == WL_CONNECTED) {
        OFL("Connection Sucessful");
        return true;
    }
    else if (status && ws == WL_CONNECT_FAILED){
        OFL("Failed Connecting to AP");
    }
    else{
        OFL("Timed Out Connecting to AP");
    }
    NL();
    return false;
}

void ESPSerialWiFiManager::_disconnect(){
    if(WiFi.status() == WL_CONNECTED){
        OL("Disconnecting from " + WiFi.SSID() + "...");
        WiFi.disconnect();
        while(WiFi.status() == WL_CONNECTED){
            delay(100);
        }
    }
}

void ESPSerialWiFiManager::_disp_network_details(){
    OFL("=============================");
    OFL("Current Network Details:");
    OFL("=============================");
    if(status() != WL_CONNECTED){
        OFL("\nNot currently connected!\n");
    }
    else{
        OL("SSID: " + WiFi.SSID());

        OF("IP Address: ");
        OL(WiFi.localIP());

        // print your MAC address:
        byte mac[6];
        WiFi.macAddress(mac);
        OF("MAC address: ");
        Serial.print(mac[5],HEX);
        Serial.print(":");
        Serial.print(mac[4],HEX);
        Serial.print(":");
        Serial.print(mac[3],HEX);
        Serial.print(":");
        Serial.print(mac[2],HEX);
        Serial.print(":");
        Serial.print(mac[1],HEX);
        Serial.print(":");
        Serial.println(mac[0],HEX);

        OF("Subnet Mask: ");
        OL(WiFi.subnetMask());

        OF("Gateway: ");
        OL(WiFi.gatewayIP());

        OF("DNS 1: ");
        OL(WiFi.dnsIP(0));

        OF("DNS 2: ");
        OL(WiFi.dnsIP(1));
    }
    OFL("=============================");
}

void ESPSerialWiFiManager::_get_advanced(){
    memset(&_temp_config, 0, sizeof(esp_wifi_config_t));
    if(_prompt_bool(F("Advanced Config?"))){
        _temp_config.ip = _prompt_ip(F("Static IP"));
        _temp_config.gateway = _prompt_ip(F("Gateway"), IPAddress(_temp_config.ip[0], _temp_config.ip[1], _temp_config.ip[2], 1));
        _temp_config.subnet = _prompt_ip(F("Subnet Mask"), IPAddress(255, 255, 255, 0));

        _temp_config.dns1 = 0U;
        _temp_config.dns2 = 0U;
        if(_prompt_bool(F("Configure DNS?"))){
            _temp_config.dns1 = _prompt_ip(F("DNS 1"));
            _temp_config.dns2 = _prompt_ip(F("DNS 2"));
        }
        _temp_config.advanced = true;
    }
}

String ESPSerialWiFiManager::_prompt(String prompt, char mask, int timeout){
    static char cmd[PROMPT_INPUT_SIZE];
    static int count;
    static char tmp;
    memset(cmd, 0, PROMPT_INPUT_SIZE);
    count = 0;
    if(timeout > 0){
        NL(); OF("Timeout in "); O(timeout); OFL("s...");
    }

    int start = millis();

    O(prompt.c_str());
    OF("> ");

    while(true){
        if(Serial.available() > 0){
            tmp = Serial.read();
            if(tmp != '\n' && tmp != '\r'){
                cmd[count] = tmp;
                if(mask==' ')
                    Serial.write(tmp);
                else
                    Serial.write(mask);
                Serial.flush();
                count++;
            }
            else{
                _flush_serial();
                NL(); NL();
                return String(cmd);
            }
        }
        delay(1);
        if(timeout > 0 && (millis()-start) > (timeout * 1000)){
            return "-1";
        }
    }
}

int ESPSerialWiFiManager::_prompt_int(String prompt, int timeout){
    String res = _prompt(prompt, ' ', timeout);
    int res_i = res.toInt();
    //NOTE: toInt() returns 0 if cannot parse, therefore 0 is not a valid input
    return res_i;
}

IPAddress ESPSerialWiFiManager::_prompt_ip(String prompt, IPAddress def){
    while(true){
        String p(prompt);
        if(def != 0){
            p = p + " (Default: " + def.toString() + ")";
        }
        String ip_str = _prompt(p, ' ', 0);
        if(ip_str.length() == 0){
            return def;
        }
        IPAddress ip;
        if(ip.fromString(ip_str))
            return ip;
        else{
            OFL("Invalid IP entered! Please try again.\n");
        }
    }
}

bool ESPSerialWiFiManager::_prompt_bool(String prompt){
    String opt = _prompt(prompt + " y/n");
    return CHAROPT(opt[0], 'y');
}

int ESPSerialWiFiManager::_print_menu(String * menu_list, int menu_size, int timeout){
    int i, opt;
    while(true){
        for(i=0; i<menu_size; i++){
            O(i+1); O(": "); OL(menu_list[i]);
        }
        opt = _prompt_int("", timeout);

        if(timeout == 0 && (opt < 1 || opt > menu_size))
            OFL("Invalid Menu Option!");
        else
            return opt;
    }
}

void ESPSerialWiFiManager::run_menu(int timeout){
    bool first_run = true;
    static const uint8_t _main_menu_size = 7;
    static String _main_menu[_main_menu_size] = {
        F("Scan"),
        F("Enter SSID"),
        F("WPS Connect"),
        F("Disconnect"),
        F("Commit Config"),
        F("Display Network Details"),
        F("Quit")
        //"Disconnect"
    };

    static int i;
    NL();
    OFL("ESP Serial WiFi Manager");
    OFL("=======================");

    while(true){
        NL();
        OFL("================");
        OFL("MAIN MENU");
        OFL("================");
        i = _print_menu(_main_menu, _main_menu_size, first_run ? timeout : 0);
        if(i == -1) return; //timeout ocurred, exit
        first_run = false;
        switch(i){
            case 1:
                _scan_for_networks();
                break;
            case 2:
                if(_connect_manual())
                    _disp_network_details();
                break;
            case 3: //WPS Connect
                _connect_wps();
                break;
            case 4:
                if(WiFi.status() == WL_CONNECTED){
                    _disconnect();
                    OFL("Complete");
                    _reset_config();
                    _write_config();
                    OFL("Choose Commit Config to prevent reconnect on reboot.");
                }
                else{
                    OFL("Not currently connected...");
                }
                break;
            case 5:
                _commit_config();
                break;
            case 6:
                _disp_network_details();
                break;
            case 7:
                if(_dirty_config){
                    OFL("Your current config is unsaved.");
                    String opt = _prompt("Save Before Quit? y/n");
                    if(CHAROPT(opt[0], 'y')) _commit_config();
                }
                return;
        }
        delay(1);
    }
}
