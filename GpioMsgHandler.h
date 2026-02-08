#define MSG_HANDLER_ID "GPIO"
/*
* Gpio messages manages GPIO pins remotely:
- pin assignment: type regular/PWM, mode: read/write
- read or write pin value
Optional return value for pin assignment and pin control depending
on if client requested confirmation or if an error occured
2 operation modes:
- sync: pin action will be done
- async: using pin events
Return value for pin read

*/

class GpioMsgHandler : public MessageListener {
public:
  static GpioMsgHandler &instance();

private:
  GpioMsgHandler();

public:
  std::string onCall(std::string rawMsg, std::string clientKey = "");
  void onPinAssign();
  void onPinControl();
  std::string onPinRead();
};

/**************************
 *** STATIC DEFINITIONS ***
 **************************/

GpioMsgHandler &GpioMsgHandler::instance() {
  static GpioMsgHandler singleton;
  return singleton;
}

GpioMsgHandler::GpioMsgHandler() : MessageListener(MSG_HANDLER_ID) {
  LogStore::info("[GpioMsgHandler] constructor");
}

std::string GpioMsgHandler::onCall(std::string rawMsg, std::string clientKey) {
  LogStore::dbg("[GpioMsgHandler::onCall] " + rawMsg);

  JsonDocument msgRoot;
  // convert to a json object
  DeserializationError error = deserializeJson(msgRoot, rawMsg);

  // msg root level props
  JsonObject msgData = msgRoot["data"];
  std::string type = msgData["type"];
  std::string pinsBatch = msgData["batch"];

  if (type == "assign") {
    return onPinAssignment(pinsBatch);
  } else if (type == "control") {
    std::string evtWrap = msgData["evtWrap"];
    return onPinControl(pinsBatch);
  } else if (type == "read") {
    std::string evtWrap = msgData["evtWrap"];
    return onPinRead(pinsBatch);
  } else if (type == "monitor") {
    std::string evtWrap = msgData["evtWrap"];
    return onPinRead(pinsBatch);
  }
  return ("");
}

/*
 Pin monitoring creates TriggerPin that will watch
 pin state at regular interval and trigger an event
 when pin state change.
 Any client subscribed to PinEvt will be notified
*/
std::string onPinMonitor(){

};

#undef MSG_HANDLER_ID