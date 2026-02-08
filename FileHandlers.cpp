/**
 * Admin service to change esp settings remotely
 */
#include <FileHandlers.h>
#include <FsUtils.h>
#include <LittleFS.h>
#include <LogStore.h>

void mergeJson(JsonVariant dst, JsonVariantConst src) {
  if (src.is<JsonObjectConst>()) {
    for (JsonPairConst kvp : src.as<JsonObjectConst>()) {
      if (dst[kvp.key()])
        mergeJson(dst[kvp.key()], kvp.value());
      else
        dst[kvp.key()] = kvp.value();
    }
  } else {
    dst.set(src);
  }
}

/*
 * FileHandler
 */

FileHandler::FileHandler(std::string fileName) : fileName(fileName) {}

void FileHandler::createFile() {}
void FileHandler::deleteFile() {}
// void FileHandler::openFile(std::string file) {}
// void FileHandler::closeFile() {}
std::string FileHandler::readContent() {
  LogStore::info("[FileHandler::readFile] file " + fileName);
  std::string fileContent = FsUtils::readFile(LittleFS, fileName.c_str());
  return fileContent;
}
void FileHandler::writeContent() {}

/*
 * JsonFileHandler
 */

JsonFileHandler::JsonFileHandler(std::string fileName) : FileHandler(fileName) {
  load();
}
/*
 *  load json from file in memory
 */
void JsonFileHandler::load() {
  LogStore::info("[JsonFileHandler::load] Load json content from file " +
                 std::string(fileName));
  std::string fileContent = readContent();
  // convert to a json object
  DeserializationError err = deserializeJson(jsFileContent, fileContent);

  // Test if parsing succeeds.
  if (!err) {
    LogStore::info("[JsonFileHandler::load] Done parsing file");
  } else {
    LogStore::info("[JsonFileHandler::load] json parsing failed: ");
    Serial.println(err.f_str());
  }
}

/*
 * update json
 */
void JsonFileHandler::merge(std::string newContent) {
  JsonDocument jsNewContent;
  DeserializationError err = deserializeJson(jsNewContent, newContent);

  // Test if parsing succeeds.
  if (!err) {
    LogStore::info("[mergeInJsonFile] Success parsing json");
  } else {
    LogStore::info("[mergeInJsonFile] Error: json parsing failed");
    Serial.println(err.f_str());
  }

  // merge objects
  mergeJson(jsFileContent, jsNewContent);
}

/*
 * write pending changes to original file
 */
void JsonFileHandler::flush() {
  // reserialize and write back to original file
  std::string sFileContentCopy = "";
  serializeJsonPretty(jsFileContent, sFileContentCopy);
  LogStore::info("[JsonFileHandler::flush] dump " + fileName +
                 " content:" + sFileContentCopy);
  // TODO
  // return sFileContentCopy;
}
