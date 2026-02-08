#include <ArduinoJson.h>
#include <LogConsumers.h>
#include <LogStore.h>
#include <string>

#define SERVICE_NAME "cfg"

class RemoteConfigService : public MessageListener {
public:
  static RemoteConfigService &instance();

private:
  RemoteConfigService();
  void loraConf(std::string msg);

public:
  std::string onServiceCall(std::string incomingMsg, int clientId = -1);
};

/**************************
 *** STATIC DEFINITIONS ***
 **************************/

RemoteConfigService &RemoteConfigService::instance() {
  static RemoteConfigService singleton;
  return singleton;
}

RemoteConfigService::RemoteConfigService()
    : MessageListener(SERVICE_NAME) {}

std::string RemoteConfigService::onServiceCall(std::string incomingMsg, int clientId) {
  LogStore::info("[RemoteConfigService::onServiceCall] ");
  JsonDocument root;
  DeserializationError error = deserializeJson(root, incomingMsg);

  // extract wrapMsg and send over LORA
  std::string dataIn = root["svcIn"];
  std::string cat = dataIn["cat"];
  if (cat == "loraCfg") {
    loraConf(incomingMsg);
  }
  LoraInterface::instance().sendText(wrapMsg);
  // default empty reply
  return "";
}

void RemoteConfigService::loraConf(std::string msg) {
  JsonDocument root;
  DeserializationError error = deserializeJson(root, incomingMsg);
  std::string dataIn = root["svcIn"];
  JsonArray confBatch = dataIn["confBatch"];
  for (JsonObject conf : confBatch) {
    int reg = conf["reg"];
    int val = conf["val"];
    LogStore::info("[RemoteConfigService::loraConf] reg: " +
                   std::to_string(reg) + " val: " + std::to_string(val));
  }
}

#undef SERVICE_NAME