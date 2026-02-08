/*
Interface between ESP and hardware pins.
GpioInterface can both handle and trigger events:
- input event PinInputEvt manages pin states
- output event PinStateChange notifies pin state change

                    +---------------+
PinInputEvt  -----> |     GPIO      | -----> pin state (R/W)
PinOutputEvt <----- |   Interface   | <----- pin state change interrupt
                    +---------------+

Supported pin operations:
- pin assignment: type regular/PWM, mode: read/write
- pin monitoring: interrupt mode, callback on state change
- read or write pin value
- sustain previous pin state

This allows GpioInterface to be controlled through different means:
- Direct static call => SYNC
- GPIO API => SYNC or ASYNC
*/

class GpioInterface: public ApiModule {
  int refreshMinDelay = 1000;
  int sustainMaxDelay = 1000;
  int lastRefreshMs = 0;
  int lastSustainMs = 0;
  //   int lastSustainCycle;
  void onApiCall(std::string evtData);
  void resetDefaultState();
  void loop(int currentCycle);
};
/*
INPUT EVENT:
PinInputEvt
{
    type: assign/write/read/sustain,
    batch: {
        1: pinData,
        2: pinData,
        ...
    }
}
- write => pinData = pinVal
- read => pinData = ""
- assign
pinData
{
    mode: R/W/M,    (read/write/monitor)
    type: REG/PWM,  (Regular/PWM)
    default:        (default value)
    rst:            (reset default)
}
- sustain: no batch required
*/
void GpioInterface::onApiCall(std::string evtData) {
  JsonDocument pinInputEvt;
  // convert to a json object
  DeserializationError error = deserializeJson(pinInputEvt, evtData);

  std::string type = pinInputEvt["type"];
  std::string pinsBatch = pinInputEvt["batch"];
  {
    int pinId;
    pinInstance = ;
    if (type == "assign") {

      JsonObject pinData = ;
    } else if (pinInstance != = nullptr) {
      if (type == "read") {
        pinsBatch[pinId] = pinInstance->read();
      } else if (type == "write") {
        int pinVal = ;
        pinInstance->write(pinVal);
      }
    } else if (type == "sustain") {
      lastSustainCycle = ;
    }
  }
  else {
    LogStore::info("[GpioInterface::onEvent] invalid pin action or can't "
                   "perform pin action on unassigned pin");
  }
};

void resetDefaultState() {
  LogStore::info("[GpioInterface::onEvent] reset pin state to default");
};
/*
Main loop
*/
void loop(int currentCycle) {
  int currentMs = millis();
  int elapsedRefreshMs = currentMs - lastRefreshMs;
  if (elapsedRefreshMs > refreshMinDelay) {
    // check for any pin state change on observed pins
    // trigger PinStateChangeEvt event
    // {
    //     type: state,
    //     pin: {
    //         id: pinId,
    //         state: H/L, (High/Low)
    //     }
    // }
    lastRefreshMs = currentMs;
  }

  int elapsedSustainMs = currentMs - lastSustainMs;
  if (elapsedSustainMs > sustainMaxDelay) {
    resetDefaultState();
    lastRefreshMs = currentMs;
  }
};