// Wifi_config_EEPROM
//
// January 2021 by Charles Vercauteren 
// Version 0.92
//
// Het SSID en wachtwoord voor het wifi-netwerk wordt gelezen uit EEPROM. Om deze waarden
// te configureren zal een drukknop, aangesloten ts D2 en GND, ons in het configuratiemenu
// brengen indien we deze ingedrukt houden bij een reset of inschakelen van de voeding.
// We vragen dan een SSID, wachtwoord, DHCP/Fixed en eventueel het vaste IP-adres
// voor het draadloze netwerk en bewaren deze in de EEPROM van de processor. 
// Bij een volgende herstart of reset zullen dan deze waarden gebruikt worden.
//
// English:
// The SSID and password for the wifi-network will be read out of EEPROM. To configure these
// values a pushbutton, connected between D2 and GND, will start the configurationmenu when 
// activated during powerup or reset.
// You will be prompted for an SSID, password, name, DHCP/Fixed and if necessary the IP-address
// for the wifi-network. Values will be saved in EEPROM and used on next powerup or reset.
//
// Credits:
// Created 13 July 2010 by dlf (Metodo2 srl)
// Modified 31 May 2012 by Tom Igoe

 // --- Wifi/EEPROM includes begin ---

#include <WiFiNINA.h>
#include <EEPROM.h>
#include <IPAddress.h>

// Seriële
#define MAX_CHAR 32               // Max. karakters voor ssid, wachtwoord en naam
#define CR '\r'                   // Er kan gebruik gemaakt worden van onderstaande karakters om
#define LF '\n'                   // invoer via seriële te beëindigen, maar niet beiden !!!
bool dataAvailable = false;       // Data op seriële aanwezig
bool ssidOK = false;              // SSID ingelezen
bool passwordOK = false;          // Wachtwoord ingelezen
bool hostnameOK = false;          // Naam ingelezen
bool dhcpOK = false;              // Keuze DHCP/Fixed ingelezen
bool fixedIpOK = false;           // Fixed IP-adres is ingelezen
char receivedChars[MAX_CHAR];     // Aantal ontvangen karakters op seriële

// EEPROM
int ssidAddress = 0 ;             // EEPROM adres ssid
int passwordAddress = MAX_CHAR;   // EEPROM adres wachtwoord
int hostnameAddress = 2*MAX_CHAR; // EEPROM adres naam
int dhcpModeAddress = 3*MAX_CHAR; // EEPROM adres dhcp(1)/fixed(0) keuze
int fixedIpAddress = 3*MAX_CHAR+1;// EEPROM adres voor vast IP-adres

String ssid = "ssid";             // Netwerk SSID
String password = "wachtwoord";   // Netwerk wachctwoord (WPA of WEP-sleutel)
String hostname = "hostname";     // Netwerk/bordje/... naam
byte dhcp = 0;                    // Default gebruiken we dhcp
String fixedIp = "127.0.0.1";

// IP
bool dhcpMode = false;
IPAddress ip;        // Laat commando "WiFi.connect(ip)" in "runOnce" weg indien DHCP
int status = WL_IDLE_STATUS;      // Wifi radio status

// Programma
bool configMode = false;          // Configuratiemode actief
bool runOnce = true;              // Eenmalig uit te voeren instructies in "loop"

// --- Wifi/EEPROM includes einde ---


void setup() {
  // --- Wifi/EEPROM setup begin ---
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // Don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // Drukknop configuratie ingedrukt ?
  pinMode(2,INPUT_PULLUP);
  if (digitalRead(2)==LOW) { configMode = true; Serial.println("Config knop ingedrukt.");}
  // --- Wifi/EEPROM setup end ---
}

void loop() {
  // --- Wifi/EEPROM loop begin ---
  if (configMode) {
    // Configmode
    // ----------
    // Opvragen SSID/wachtwoord/...
    static bool ssidRunOnce = false;      // Vraag naar SSID, éénmalig tonen
    static bool passwordRunOnce = true;   // Vraag naar wachtwoord, éénmalig tonen
    static bool hostnameRunOnce = true;   // Vraag naar naam, éénmalig tonen
    static bool dhcpRunOnce = true;       // Vraag naar dhcp/fixed keuze, éénmalig tonen
    static bool fixedIpRunOnce = true;    // Vraag naar IP-adres, éénmaling tonen
      
    if (!ssidRunOnce)     { Serial.print("\n  SSID: "); ssidRunOnce = true;  }
    if (!passwordRunOnce) { Serial.print("\n  Password: "); passwordRunOnce = true; }
    if (!hostnameRunOnce) { Serial.print("\n  Name: "); hostnameRunOnce = true; }
    if (!dhcpRunOnce)     { Serial.print("\n  DHCP/Fixed IP-address (D/F): "); dhcpRunOnce = true; }
    if (!fixedIpRunOnce)  { Serial.print("\n  IP-address (e.g. 192.168.0.1): "); fixedIpRunOnce = true; }

    recvString();
    if (dataAvailable) {
      if (!ssidOK) {
        // Lees ssid 
        ssid = String(receivedChars);
        ssidOK = true;
        dataAvailable = false;
        passwordRunOnce = false;
      }
      else if (!passwordOK) {
        // Lees wachtwoord
        password = String(receivedChars);
        passwordOK = true;
        dataAvailable = false;

        hostnameRunOnce = false;
      }
      else if (!hostnameOK) {
        // Lees naam
        hostname = String(receivedChars);
        hostnameOK = true;
        dataAvailable = false;

        dhcpRunOnce = false;
      }
      else if (!dhcpOK) {
        // DHCP mode ?
        if (String(receivedChars)=="D" || String(receivedChars)=="d") { 
          Serial.println("DHCP mode");
          dhcpMode = true;
          fixedIpOK = true;         // Geen ip-adres opvragen
          configMode = false;
          }
        else { 
          dhcpMode = false; 
          fixedIpRunOnce = false; 
          }
          
        dataAvailable = false;
        dhcpOK = true;
      }
      else if (!dhcpMode) {
        fixedIp = String(receivedChars);
        fixedIpOK = true;
        dataAvailable = false;

        configMode = false;
      }
    }

    
    if (ssidOK && passwordOK && hostnameOK && dhcpOK && fixedIpOK) {
      Serial.println("Bewaren SSID en wachtwoord in EEPROM.");
      Serial.print("  SSID = "); Serial.println(ssid);
      Serial.print("  Password = "); Serial.println(password);
      Serial.print("  Name = "); Serial.println(hostname);
      if (dhcpMode) { Serial.println("  DHCP mode"); }
      else { Serial.println("  Fixed IP-address"); }
      Serial.println(); Serial.println("Leaving config mode.");

      saveSsidToEeprom(ssid);
      savePasswordToEeprom(password);
      saveHostnameToEeprom(hostname);
      saveDhcpModeToEeprom(dhcpMode);
      if (!dhcpMode) { 
        saveIpAddressToEeprom(fixedIp);
      }

      Serial.println();
    }
  }
  else {
    if (runOnce) {
      Serial.println();
      Serial.println("RunOnce");
      // Connect to wifi
      // ---------------
      // Lees ssid/wachtwoord/naam
      ssid="";
      for (int i=0; i<MAX_CHAR; i++) { ssid+=(char)EEPROM.read(ssidAddress+i); }
      password="";
      for (int i=0; i<MAX_CHAR; i++) { password+=(char)EEPROM.read(passwordAddress+i); }
      hostname="";
      for (int i=0; i<MAX_CHAR; i++) { hostname+=(char)EEPROM.read(hostnameAddress+i); }
      if (EEPROM.read(dhcpModeAddress)==0) { 
        dhcpMode = false; 
        fixedIp = "";
        for (int i=0; i<MAX_CHAR; i++) { fixedIp+=(char)EEPROM.read(fixedIpAddress+i); }
        Serial.println("Fixed mode.");
        Serial.println(fixedIp);
        }
      else { 
        dhcpMode = true; 
        Serial.println("Dhcp mode.");
      }

      Serial.println("In EEPROM:");
      Serial.print("  SSID = "); Serial.println(ssid);
      Serial.print("  Password = "); Serial.println("********");
      Serial.print("  Name = "); Serial.println(hostname);
      Serial.print("  DHCP = ");
      if (dhcpMode) { Serial.println("Yes"); }
      else { 
        Serial.println("No");  
        Serial.print("  IP: "); Serial.println(fixedIp);
        }
      
      // Maak verbinding met wifi netwerk
      // --> Laat onderstaand commando weg indien DHCP gewenst is. <--
      if (!dhcpMode) { 
        ip.fromString(fixedIp.c_str());
        WiFi.config(ip);
      }
      while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network:
        status = WiFi.begin(ssid.c_str(), password.c_str());
        // Wacht 10 seconden en probeer opnieuw
        delay(10000);
      }

      // We zijn verbonden, print wat info
      Serial.println("You're connected to the network");
      printCurrentNet();
      printWifiData();


      
      runOnce = false;
      Serial.println("Starting loop.");

    }
    // --- Wifi/EEPROM loop einde ---
    else {
      // Plaats hier uw programmacode
      // ----------------------------
      //Serial.println("Looping.");
      delay(1000);
      // ----------------------------   
    }
  }
}

// --- EEPROM functies begin ---

void saveSsidToEeprom(String name) {
  for (int i=0; i<MAX_CHAR; i++) {
    EEPROM.write(ssidAddress+i,name.charAt(i));
  }
}

void savePasswordToEeprom(String name) {
  for (int i=0; i<MAX_CHAR; i++) {
    EEPROM.write(passwordAddress+i,name.charAt(i));
  }
}

void saveHostnameToEeprom(String name) {
  for (int i=0; i<MAX_CHAR; i++) {
    EEPROM.write(hostnameAddress+i,name.charAt(i));
  }
}

void saveDhcpModeToEeprom(bool mode) {
  if (mode) { 
    EEPROM.write(dhcpModeAddress, 1); 
    }
  else { 
    EEPROM.write(dhcpModeAddress, 0); 
  }
}

void saveIpAddressToEeprom(String name) {
  for (int i=0; i<MAX_CHAR; i++) {
    EEPROM.write(fixedIpAddress+i,name.charAt(i));
  }
}

// --- EEPROM functies einde ---


// --- Wifi functies begin ---
void printWifiData() {
  // Print IP-adres:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // Print MAC-adres
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

void recvString() {
  // Controleer of nieuwe data op seriële aanwezig is.
  // Zoja, lees deze in. END_MARKER aanwezig, sluit dan de string af met '\0' 
  // om geldige C-string van te maken en zet "dataAvailable" true.
  // Controleer en houd het aantal karakters kleiner dan MAX_CHAR
  
  static byte ndx = 0;
  char rc;

  while (Serial.available() > 0 && dataAvailable == false) {
    rc = Serial.read();
    Serial.print(rc);       //echo karakter

    if (rc != CR && rc != LF) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= MAX_CHAR) {
        ndx = MAX_CHAR - 1;
      }
    }
    else {
     receivedChars[ndx] = '\0'; // terminate the string
     ndx = 0;
     dataAvailable = true;
    }
  }
}
// --- Wifi functies einde ---
