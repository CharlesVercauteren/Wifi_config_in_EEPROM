# Wifi_config_in_EEPROM

Het SSID en wachtwoord voor het wifi-netwerk wordt gelezen uit EEPROM. Om deze waarden te configureren zal een drukknop, aangesloten ts D2 en GND, ons in het configuratiemenu brengen indien we deze ingedrukt houden bij een reset of inschakelen van de voeding. We vragen dan een SSID, wachtwoord, DHCP/Fixed en eventueel het vaste IP-adres voor het draadloze netwerk en bewaren deze in de EEPROM van de processor. Bij een volgende herstart of reset zullen dan deze waarden gebruikt worden.<br>

English.<br>
The SSID and password for the wifi-network will be read out of EEPROM. To configure these values a pushbutton, connected between D2 and GND, will start the configurationmenu when activated during powerup or reset.You will be prompted for an SSID, password, name, DHCP/Fixed and if necessary the IP-address for the wifi-network. Values will be saved in EEPROM and used on next powerup or reset.
