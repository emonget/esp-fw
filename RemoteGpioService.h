/**
 * Web socket service providing remote GPIO control
 * Usage example from browser:
 * let ws = new WebSocket(`ws://${window.location.hostname}/gpio`)
 * let gpio = { pin: 15, pwmChan: 0, val: 18, type: 0 };
 * let msg = {gpios: [gpio]};
 * webSocket.send(JSON.stringify(msg))
 */

#include <ArduinoJson.h>
#include <GpioFactory.h>
#include <GpioPin.h>
#include <LogStore.h>
#include <ApiModule.h>

#define API_SERVICE_NAME "GPIO"

class GpioRemoteService : public ApiModule {
public:
  static GpioRemoteService &instance();
  // for how long pins are maintained in state before restored to default value
  // '0' disables restoring default pin state
  static int resetCycles;
  // internal cycle counter used for pin reset and benchmark
  static int cyclesCount;

private:
  GpioRemoteService();

public:
  std::string onApiCall(Packet &msg);

  static void loop();
};

/**************************
 *** STATIC DEFINITIONS ***
 **************************/

int GpioRemoteService::resetCycles(0);
int GpioRemoteService::cyclesCount(0);

GpioRemoteService &GpioRemoteService::instance() {
  static GpioRemoteService singleton;
  return singleton;
}

void GpioRemoteService::loop() {
  if (GpioRemoteService::resetCycles == 0 ||
      GpioRemoteService::cyclesCount <= GpioRemoteService::resetCycles) {
    GpioRemoteService::cyclesCount++;
  } else {
    // if no keep alive signal received after a while
    GpioFactory::resetPinsDefaults();
  }
}

/*
 * pin operation requirements
 * required  |  instance exists | Static/Instance level  |  pinData | auto mode
 * ----------+------------------+------------------------+----------+--------------------------+
 * alloc               N                 S                     x       pin conf
 * provided read                Y                 I pin instance exists write Y
 * I                     x       pin instance exists free                Y S
 * dump                Y                 I
 */

GpioRemoteService::GpioRemoteService() : ApiModule(API_SERVICE_NAME) {
  LogStore::info("[GpioRemoteService] constructor");
}

// void GpioRemoteService::extractMsg(std::string rawMsg) {
std::string GpioRemoteService::onApiCall(Packet &msg) {
  LogStore::info("[GpioRemoteService] onCall ");

  JsonDocument root;
  // convert to a json object
  DeserializationError error = deserializeJson(root, rawMsg);

  // root level props
  std::string cmd = root["cmd"];     // for operation not tied to any pin
  int timestamp = root["timestamp"]; // purpose being to identify message in
                                     // case response is needed
  JsonObject jsPinsBatch = root["pinsBatch"];
  std::string replyMsg("");

  // KeepAlive signal to:
  // - maintain latest pin state
  // - make sure service is still alive
  // - get cycles count stats to benchmark ESP usage

  if (cmd == "keepAlive") {
    replyMsg = "MSG_ID #" + std::to_string(timestamp) + " CLI_ID #TODO" +
               " cycles #" + std::to_string(cyclesCount);
    cyclesCount = 0; // will maintain latest pin state
  } else if (cmd == "config") {
    JsonObject config = root["config"];
    resetCycles = config["resetCycles"];
    LogStore::info("[GpioRemoteService::unpackMsg] Pins reset cycle set to " +
                   std::to_string(resetCycles));
  }

  // hash pins batch and passdown pin data to corresponding instance
  // or make static call
  for (JsonPair kv : jsPinsBatch) {
    // key/value
    int pin = std::stoi(kv.key().c_str());
    // extract pin data
    JsonObject jsPinData = kv.value().as<JsonObject>();

    GpioPin *pinInstance = GpioFactory::pinFind(pin);

    // returned value of pin operation if applicable
    std::string *sPinRes = NULL;

    const char *pinOp = jsPinData["op"];
    switch (*pinOp) {
    case 'a': // alloc
    {
      std::string sPinData;
      serializeJson(jsPinData, sPinData);
      pinInstance = GpioFactory::pinAssign(pin, sPinData); // [static]
      break;
    }
    case 'r': // read
    {
      // Gpio::pinRead(pin);
      std::string sPinVal(std::to_string(pinInstance->read()));
      sPinRes = &sPinVal;
      break;
    }
    case 'w': // write
    {
      // std::cout << "[GpioRemoteService::unpackMsg] pin write " << std::endl;
      int pinVal = jsPinData["val"];
      // Gpio::pinWrite(pin, data);
      pinInstance->write(pinVal);
      break;
    }
    case 'f': // free
    {
      GpioFactory::pinFree(pin); // [static]
      break;
    }
    case 'd': // dump
    {
      // Gpio::pinDump(pin);
      std::string sPinVal(pinInstance->dump());
      sPinRes = &sPinVal;
      break;
    }
    default: // missing pin op => auto pin action
    {
      LogStore::info("[GpioRemoteService::unpackMsg] no OP provided defaulting "
                     "to pinAuto mode ");
      // pack/encode/serialize for pin data forwarding
      std::string sPinData;
      serializeJson(jsPinData, sPinData);
      int retVal = GpioFactory::pinAuto(pin, sPinData);
      if (retVal == -1) {
        LogStore::info("FAILED auto mode for pin " + std::to_string(pin));
      }
      std::string sPinVal(std::to_string(retVal));
      sPinRes = &sPinVal;
      // std::string sPinVal(std::to_string());
      // sPinRes = &sPinVal;
    }
    }
  }

  return replyMsg;
}

#undef API_SERVICE_NAME
