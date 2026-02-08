#include <CommonObj.h>
#define API_MODULE "PWR"
/*
Monitor power from input PIN
API module providing access to current power level
Trigger PWR event on low power
*/
class PowerMonitor : ApiModule {
  PowerMonitor(int inputPin);
  std::string onApiCall(Packet &msg);
};