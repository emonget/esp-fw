/*
MessageNotificationHandler is handling client request for event notification.
For each new client subscribing to specific event it will create corresponding
instance of MessageEventNotifier
(e.g. LogEventRemoteListener for LOGS or
PinEventRemoteListener for PINS)

- LogEventRemoteListener:LogEventListener:EventHandler
- PinEventRemoteListener:PinEventListener:EventHandler
MessageNotificationHandler

2 remote mechanism
- POLLING: external client performs action:
- NOTIFICATION: device push event notification to registered clients

POLLING mode usecase examples:
- LOG/PIN data pull,
- PIN control,
- FS operation (change setting in file)

NOTIFICATION mode usecase examples:
- subscribe log notification
- subscribe pin state change
*/
// HPP

#pragma once
#define API_MODULE "APICommon"

#include <ApiModule.h>
#include <ArduinoJson.h>
#include <map>
#include <string>
// CPP
#include <EventMessageNotifier.h>
#include <HTTPSServer.hpp>
#include <LogStore.h>
#include <LoraInterface.h>
#include <LinkInterface.h>
#include <WsInterface.h>

/*
 *
 */
class CommonApiServices : public ApiModule {
public:
  static CommonApiServices &instance();

private:
  CommonApiServices();

public:
  std::string onApiCall(Packet &msg);

  // API services
  std::string onRepeatMessage(std::string msgContent);
};

/**************************
 *** STATIC DEFINITIONS ***
 **************************/

CommonApiServices &CommonApiServices::instance() {
  static CommonApiServices singleton;
  return singleton;
}

CommonApiServices::CommonApiServices() : ApiModule(API_MODULE) {
  LogStore::info("[CommonApiServices] constructor");
}

// void CommonApiServices::extractMsg(std::string rawMsg) {
std::string CommonApiServices::onApiCall(Packet &msg) {
  LogStore::info("[CommonApiServices::onApiCall] " + apiCallMsg);
  // std::string apiCommand = apiInput["cmd"];
  // repeat wrapped message over LORA interface
  if (msg.cmd == "repeat") {
    LoraInterface::instance().sendText(msg.data);
  }
  return ("");
}

/*
ECHO, REPEATER, GATEWAY
API service both available on WS and LORA interface
Its sole purpose is to echo message on LORA interface
or proxy message received on WS over LORA interface


Recursive repeating example
Browser tells Device#1 to forward wrapped message over LORA (Gateway)
Device#1 sends wrapped message over LORA telling Device#2 to ECHO wrapped
message over same interface
Device#2 repeat wrapped message over LORA

    CLIENT      |      DEVICE #1            |       DEVICE #2
                |                           |
                |     +--------------+      |         +--------------+
           --- WS --> | MSG Repeater | --- LORA -->   | MSG Repeater |
            (lvl 0)   +--------------+  msgContent(#1)+--------------+
                |     +--------------+      |             |
                |     |  API Test    | <--- LORA ----------
                |     +--------------+  msgContent(#2) (=testMsg)
                |                           |
{// level 0 sent from browser over WS
  "api": {
    "name": "MsgRepeaterApi",
    "input": {
      "msgContent": { // repeat level 1 (forwarded by device#1)
        "dst": "device#2", // prevent device#1 from receiving its own message
        "api": {
          "name": "MsgRepeaterApi",
          "input": {
            "msgContent": { // repeat level 2 (to be echoed by device#2)
              "dst": "device#1", // prevent device#2 from receiving its own
message "api": { "name": "TestedAPI", "input": {} // API input data
              }
            }
          }
        }
      }
    }
  }
}*/
std::string CommonApiServices::onRepeatMessage(std::string msgContent) {}

#undef API_MODULE