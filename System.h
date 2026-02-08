#pragma once
#include <SystemTime.h>
#include <SystemConf.h>
#include <string>

/*
Safe area is minimal working system relying on following core modules:
- FS access (required to load config persistant settings)
- Config load (required for network conf/ failsafe mode flag)
- Network access (required for wireless device access)
- Webserver (required for OTA updates)
- OTA updater service

Failsafe or recovery mode will be enabled after any previous failed attempt to
boot by checking a flag indicating if previous boot was successful or not. The
flag value is read and reset immediately at the beginning of boot process. If
booting succeeds flag will be set back to normal at the end of boot process If
not it will remain unset, indicating system boot failure.
*/
class System {
public:
  static SystemTime time;
  static SystemConf cfgStore;
  static void coreInit();
  static void initFS();
  static void initLAN();
  static void loadLanProfile(std::string profileName);
  static void staticIpConfig(std::string sStaticIP, std::string sGateway,
                             std::string sSubnet, std::string sDns1,
                             std::string sDns2);
  static void activateWifiAP(std::string ssid, std::string pwd);
  static void activateWifiSTA(std::string ssid, std::string pwd);
  static void deepSleep(int sleepdelay);
};
