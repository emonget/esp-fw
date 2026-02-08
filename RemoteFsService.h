/**
 * Web socket service providing remote filesystem operations
 */

#include <ArduinoJson.h>
#include <HTTPSServer.hpp>
#include <LogStore.h>
#include <WebServer.h>
#include <WebsocketHandler.hpp>
#include <defaults.h>
#include <string>

using namespace httpsserver;

std::string dumpJsonFile(const char * fileName) {
  std::string fileContent = FsUtils::readFile(LittleFS, fileName);
  JsonDocument jsFileContent;
  DeserializationError error = deserializeJson(jsFileContent, fileContent);

  // Test if parsing succeeds.
  if (!error) {
    LogStore::info("[FsRemoteService::handleJsonFile] Done reading file content");
  } else {
    LogStore::info("deserializeJson() failed: ");
    Serial.println(error.f_str());
  }

  // reserialize back
  std::string sFileContentCopy = "";
  serializeJsonPretty(jsFileContent, sFileContentCopy);
  return sFileContentCopy;
}

class FsRemoteService : public WebsocketHandler {
  // STATIC FIELDS
  static WebsocketNode
      *webSocketNode; // ws route registration on secured webserver
  static std::map<int, FsRemoteService *>
      instances; // dedicated instance for each connected client
  // this will be called each time new client connects to the /logs websocket
  static WebsocketHandler *onCreate();

  // FIELDS
  int clientId = 0;

  // STATIC METHODS
public:
  static void init();
  // METHODS
private:
  FsRemoteService(int clientId);

public:
  // This method is called when a message arrives
  void onPacket(WebsocketInputStreambuf *input);
  // Handler function on connection close
  void onClose();
  // extract json structure
  void extractMsg(std::string rawMsg);
  // FS operations
  void createFile();
  void deleteFile();
  //   void openFile(std::string file);
  //   void closeFile();
  void readFile(std::string file);
  void writeFile();
};

/**************************
 *** STATIC DEFINITIONS ***
 **************************/
std::map<int, FsRemoteService *> FsRemoteService::instances;

WebsocketNode *FsRemoteService::webSocketNode =
    new WebsocketNode("/fs", &FsRemoteService::onCreate);

// each time new client connects to the /gpio websocket a new service
// instance is created
WebsocketHandler *FsRemoteService::onCreate() {
  int clientNb = FsRemoteService::instances.size();
  FsRemoteService *instance = NULL;
  if (clientNb < MAX_CLIENTS) {
    instance = new FsRemoteService(clientNb);
    FsRemoteService::instances.insert({clientNb, instance});
  } else {
    LogStore::info(
        "[FsRemoteService::onCreate] max number of client reached #" +
        std::to_string(clientNb));
  }
  // will be cast to WebsocketHandler*
  return instance;
}

/**************************
 *** INSTANCE DEFINITIONS ***
 **************************/

FsRemoteService::FsRemoteService(int clientId)
    : WebsocketHandler(), clientId(clientId) {
  LogStore::info("[FsRemoteService] Client #" + std::to_string(clientId) +
                 " connected");
}

void FsRemoteService::init() {
  LogStore::info("[FsRemoteService::init]");
  // register ws service to main secured server
  WebServer::instance().secureServer.registerNode(webSocketNode);
}

// Finally, passing messages around. If we receive something, we send it to all
// other clients
void FsRemoteService::onPacket(WebsocketInputStreambuf *inbuf) {
  // Get the input message
  std::ostringstream ss;
  std::string msg;
  ss << inbuf;
  msg = ss.str();
  // this->send("[LogRemoteService::onPacket]", SEND_TYPE_TEXT);
  // readFile(CONFIG_FILE);
  extractMsg(msg);
}

void FsRemoteService::extractMsg(std::string incomingMsg) {
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
  LogStore::info("[FsRemoteService::extractMsg] msgRefID " +
                 std::to_string(msgRefId));

  std::string replyMsg("Default reply from FsRemoteService");

  // supported fs commands
  // list files
  if (cmd == "ls") {
    LogStore::info("[FsRemoteService::extractMsg] [TODO] list all files");
  }
  else if (cmd == "cat") {
    LogStore::info("[FsRemoteService::extractMsg] [TODO] dump file content");
  } 
  else if (cmd == "rm") {
    LogStore::info("[FsRemoteService::extractMsg] [TODO] remove file");
  }
  else if (cmd == "touch") {
    LogStore::info("[FsRemoteService::extractMsg] [TODO] create file");
  }
  else if (cmd == "write") {
    LogStore::info("[FsRemoteService::extractMsg] [TODO] write content to file");
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
void FsRemoteService::onClose() {
  LogStore::info("[FsRemoteService] Client #" + std::to_string(clientId) +
                 " disconnected");
  // for(int i = 0; i < MAX_CLIENTS; i++) {
  //   if (activeClients[i] == this) {
  //     activeClients[i] = nullptr;
  //   }
  // }
}

/*
 *   FS operations
 */

void FsRemoteService::createFile() {}
void FsRemoteService::deleteFile() {}
// void FsRemoteService::openFile(std::string file) {}
// void FsRemoteService::closeFile() {}
void FsRemoteService::readFile(std::string file) {
  LogStore::info("[FsRemoteService::readFile] file " + file);
  std::string fileContent =
      "TODO read file content"; // readFile(LittleFS, file);
  this->send(fileContent, SEND_TYPE_TEXT);
}
void FsRemoteService::writeFile() {
  // std::cout << "[LogRemoteService] received message: " << msg << std::endl;

  // Send it back to every client
  // for (int i = 0; i < MAX_CLIENTS; i++) {
  //   if (activeClients[i] != nullptr) {
  //     activeClients[i]->send(msg, SEND_TYPE_TEXT);
  //   }
  // }

  // aggregate all logs stored in buffer

  //   StaticJsonDocument<5000> doc;
  //   JsonArray logs = doc.createNestedArray("logs");
  //   for (auto const &item : LogStore::logBuffer) {
  //     LogData logData = item.second;
  //     // logs += logData.timestamp;
  //     logs.add(logData.log);
  //   }
  //   std::string output("");
  //   serializeJsonPretty(doc, output);
  //   this->send(output, SEND_TYPE_TEXT);
  //   // clear log buffer
  //   LogStore::logBuffer.clear();
}
