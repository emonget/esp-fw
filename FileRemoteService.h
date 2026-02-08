#include <ArduinoJson.h>
#include <HTTPSServer.hpp>
#include <LogStore.h>
#include <WebServer.h>
#include <WebsocketHandler.hpp>
#include <defaults.h>
#include <string>

using namespace httpsserver;

class FileRemoteService : public WebsocketHandler {
  // STATIC FIELDS
  static WebsocketNode
      *webSocketNode; // ws route registration on secured webserver
  static std::map<int, FileRemoteService *>
      instances; // dedicated instance for each connected client
  // this will be called each time new client connects to the /logs websocket
  static WebsocketHandler *onCreate();

  // FIELDS
  std::string clientKey = 0;

  // STATIC METHODS
public:
  static void init();
  // METHODS
private:
  FileRemoteService(std::string clientKey);

public:
  // This method is called when a message arrives
  void onPacket(WebsocketInputStreambuf *input);
  // Handler function on connection close
  void onClose();
  // extract json structure
  void extractMsg(std::string rawMsg);
  // Admin operations
  void dumpConfig();
  void updateSettings(); // update settings temporarily
  void saveSettings();   // making changes permanent
};

/**************************
 *** STATIC DEFINITIONS ***
 **************************/
std::map<int, FileRemoteService *> FileRemoteService::instances;

WebsocketNode *FileRemoteService::webSocketNode =
    new WebsocketNode("/fs", &FileRemoteService::onCreate);

// each time new client connects to the /gpio websocket a new service
// instance is created
WebsocketHandler *FileRemoteService::onCreate() {
  int clientNb = FileRemoteService::instances.size();
  FileRemoteService *instance = NULL;
  if (clientNb < MAX_CLIENTS) {
    instance = new FileRemoteService(clientNb);
    FileRemoteService::instances.insert({clientNb, instance});
  } else {
    LogStore::info(
        "[FileRemoteService::onCreate] max number of client reached #" +
        std::to_string(clientNb));
  }
  // will be cast to WebsocketHandler*
  return instance;
}

/**************************
 *** INSTANCE DEFINITIONS ***
 **************************/

FileRemoteService::FileRemoteService(std::string clientKey)
    : WebsocketHandler(), clientKey(clientKey) {
  LogStore::info("[FileRemoteService] Client #" + clientKey + " connected");
}

void FileRemoteService::init() {
  LogStore::info("[FileRemoteService::init]");
  // register ws service to main secured server
  WebServer::instance().secureServer.registerNode(webSocketNode);
}

// Finally, passing messages around. If we receive something, we send it to all
// other clients
void FileRemoteService::onPacket(WebsocketInputStreambuf *inbuf) {
  // Get the input message
  std::ostringstream ss;
  std::string msg;
  ss << inbuf;
  msg = ss.str();
  // this->send("[LogRemoteService::onPacket]", SEND_TYPE_TEXT);
  // readFile(CONFIG_FILE);
  extractMsg(msg);
}

void FileRemoteService::extractMsg(std::string incomingMsg) {
  // std::cout << "[GpioRemoteService::unpackMsg] Incoming message " <<
  // incomingMsg
  //           << std::endl;
  JsonDocument root;
  // convert to a json object
  DeserializationError error = deserializeJson(root, incomingMsg);

  // root level props
  std::string cmd = root["cmd"];   // for operation not tied to any pin
  int msgRefId = root["msgRefId"]; // in case reply is needed
  JsonObject jsFilesBatch = root["filesBatch"];
  LogStore::info("[FileRemoteService::extractMsg] msgRefID " +
                 std::to_string(msgRefId));

  std::string replyMsg("Default reply from FileRemoteService");

  // supported admin commands
  if (cmd == "getConfig") {
    LogStore::info("[FileRemoteService::extractMsg] [getConfig] ***TODO***");
  } else if (cmd == "setConfig") {
    LogStore::info("[FileRemoteService::extractMsg] [setConfig] ***TODO***");
  } else if (cmd == "saveConfig") {
    // making settings permanent
    LogStore::info("[FileRemoteService::extractMsg] [saveConfig] ***TODO***");
  }

  // hash files batch
  for (JsonPair kv : jsFilesBatch) {
    // key/value
    int pin = std::stoi(kv.key().c_str());
    // extract pin data
    JsonObject jsFileData = kv.value().as<JsonObject>();

    // GpioPin *pinInstance = GpioFactory::pinFind(pin);

    // returned value of pin operation if applicable
    // std::string *sPinRes = NULL;

    const char *fileOp = jsFileData["op"];
    const char *fileName = jsFileData["fileName"];
    // supported types: json
    const char *fileType = jsFileData["fileType"];
    switch (*fileOp) {
    case 'c': // create file
    {
      // std::string sPinData;
      // serializeJson(jsPinData, sPinData);
      // pinInstance = GpioFactory::pinAlloc(pin, sPinData); // [static]
      break;
    }
    case 'r': // read
    {
      LogStore::info("[Core::ConfigLoader] read file content from filesystem " +
                     std::string(fileName));

      std::string fileContent = dumpJsonFile(fileName);
      // and send to client
      this->send(fileContent, SEND_TYPE_TEXT);
      break;
    }
    case 'w': // write
    {
      const char *fileContent = jsFileData["fileContent"];
      // int pinVal = jsPinData["val"];
      // pinInstance->write(pinVal);
      break;
    }
    case 'd': // delete
    {
      // GpioFactory::pinFree(pin); // [static]
      break;
    }
    }
  }
  // optional reply depending on pin operation
  // if (msgRefId) {
  //   std::string replyMsg("TODO");
  // }
  if (replyMsg.length()) {
    String dataOut(replyMsg.c_str());
    // webSocket.textAll(dataOut);
  }
}

// When the websocket is closing, we remove the client from the array
void FileRemoteService::onClose() {
  LogStore::info("[FileRemoteService] Client #" + clientKey + " disconnected");
  // for(int i = 0; i < MAX_CLIENTS; i++) {
  //   if (activeClients[i] == this) {
  //     activeClients[i] = nullptr;
  //   }
  // }
}
