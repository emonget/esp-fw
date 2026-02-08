#include <Arduino.h>
#include <ArduinoJson.h>
#include <FsUtils.h>
#include <LittleFS.h>
#include <LogStore.h>
#include <System.h>
#include <WiFi.h>
#include <defaults.h>
#include <utils.h>

SystemTime System::time;
SystemConf System::cfgStore;

void System::coreInit() {
  LogStore::info("[System::init] Filesystem");
  initFS();
  LogStore::info("[System::init] LAN");
  initLAN();
  // activateWifiSTA();
}

void System::initFS() {
  if (!LittleFS.begin(true)) {
    LogStore::info("[Core::FS] error while mounting filesystem");
  }
  LogStore::info("[Core::FS] mounted successfully!");

  FsUtils::listDir(LittleFS, "/", 0);

  LogStore::info("[Core::FS] Done");
}

void System::initLAN() {
  std::string apProfile = cfgStore.configContent()["usedApProfile"];
  std::string staProfile = cfgStore.configContent()["usedStaProfile"];
  LogStore::info("[System::initLAN] using LAN profiles STA: " + staProfile +
                 ", AP: " + apProfile);
  std::string apBaseSSID =
      cfgStore.configContent()["networkProfiles"][apProfile]["ssid"];
  std::string apPWD =
      cfgStore.configContent()["networkProfiles"][apProfile]["pwd"];
  std::string deviceId = cfgStore.configContent()["deviceId"];
  std::string apFullSSID = apBaseSSID + "-" + deviceId;
  activateWifiAP(apFullSSID, apPWD);
  loadLanProfile(staProfile);
}

void System::loadLanProfile(std::string profileName) {
  LogStore::info("[System::loadLanProfile] loading network profile: " +
                 profileName);
  std::string ssid =
      cfgStore.configContent()["networkProfiles"][profileName]["ssid"];
  std::string pwd =
      cfgStore.configContent()["networkProfiles"][profileName]["pwd"];
  std::string staticIp =
      cfgStore.configContent()["networkProfiles"][profileName]["staticIP"];
  LogStore::dbg("[System::loadLanProfile] SSID: " + ssid);
  if (staticIp.length() && staticIp != "null") {
    std::string gateway =
        cfgStore.configContent()["networkProfiles"][profileName]["gateway"];
    std::string subnet =
        cfgStore.configContent()["networkProfiles"][profileName]["subnet"];
    std::string primaryDns =
        cfgStore.configContent()["networkProfiles"][profileName]["primaryDNS"];
    std::string secondaryDns =
        cfgStore
            .configContent()["networkProfiles"][profileName]["secondaryDNS"];
    staticIpConfig(staticIp, gateway, subnet, primaryDns, secondaryDns);
  }
  activateWifiSTA(ssid, pwd);
  // // NOT WORKING
  // JsonObject networkProfiles =
  // conf.configContent()["networkProfiles"]; if
  // (!networkProfiles.isNull()) {
  //   LogStore::dbg("[System::loadLanProfile] not null");
  //   JsonObject profileContent = networkProfiles[profileName];
  //   std::string contentDump = "";
  //   serializeJsonPretty(networkProfiles, contentDump);
  //   LogStore::dbg("[System::loadLanProfile] dump content " +
  //   contentDump);
  // } else {
  //   LogStore::info(
  //       "[System::loadLanProfile] couldn't read network profiles");
  // }
}

void System::staticIpConfig(std::string sStaticIP, std::string sGateway,
                            std::string sSubnet, std::string sDns1,
                            std::string sDns2) {
  LogStore::info("[System::staticIpConfig] LAN settings \n IP: " + sStaticIP +
                 "\n gateway: " + sGateway + "\n subnet: " + sSubnet +
                 "\n dns1: " + sDns1 + "\n dns2: " + sDns2);
  std::vector<int> viStaticIP =
      strVectToIntVect(splitFromCharDelim(sStaticIP, '.'));
  std::vector<int> viGateway =
      strVectToIntVect(splitFromCharDelim(sGateway, '.'));
  std::vector<int> viSubnet =
      strVectToIntVect(splitFromCharDelim(sSubnet, '.'));
  std::vector<int> viDns1 = strVectToIntVect(splitFromCharDelim(sDns1, '.'));
  std::vector<int> viDns2 = strVectToIntVect(splitFromCharDelim(sDns2, '.'));

  IPAddress ip(viStaticIP.at(0), viStaticIP.at(1), viStaticIP.at(2),
               viStaticIP.at(3));
  IPAddress gateway(viGateway.at(0), viGateway.at(1), viGateway.at(2),
                    viGateway.at(3));
  IPAddress subnet(viSubnet.at(0), viSubnet.at(1), viSubnet.at(2),
                   viSubnet.at(3));
  // optional
  IPAddress dns1(viDns1.at(0), viDns1.at(1), viDns1.at(2), viDns1.at(3));
  IPAddress dns2(viDns2.at(0), viDns2.at(1), viDns2.at(2), viDns2.at(3));
  // Configures static IP address
  if (!WiFi.config(ip, gateway, subnet, dns1, dns2)) {
    LogStore::info(
        "[System::staticIpConfig] Failed to configure static IP address");
  }
}

// turn on wifi client
void System::activateWifiSTA(std::string ssid, std::string pwd) {
  int retryCounter = 10;
  LogStore::info("[System::activateWifiSTA] Connecting to " +
                 std::string(ssid.c_str()));
  // any wifi conf must be called prior to this
  WiFi.begin(ssid.c_str(), pwd.c_str());

  // wait for connection
  while (retryCounter > 0 && WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retryCounter--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    std::string ipAddr(WiFi.localIP().toString().c_str());
    LogStore::info("[System::activateWifiSTA] Connected with address: " +
                   ipAddr);
  } else {
    LogStore::info("[System::activateWifiSTA] Could not connect to LAN");
  }
}

// turn on wifi access point
void System::activateWifiAP(std::string ssid, std::string pwd) {
  LogStore::info("[System::activateWifiAP] Setting up access point " + ssid);
  WiFi.softAP(ssid.c_str(), pwd.c_str());
  std::string ipAddress(WiFi.softAPIP().toString().c_str());
  LogStore::info("[System::activateWifiAP] Hotspot " + ssid + " available at " +
                 ipAddress);
}

void System::deepSleep(int sleepdelay) {
  LogStore::info("[System::deepSleep] Going deep sleep in " +
                 std::to_string(sleepdelay));

  esp_deep_sleep_start();
}