/*
* Regular/PWM pin types
* Observer/Controller => read/write modes
* Observer => pinEvt
* pinEvt => Controller

PinEvt modes: 
- CTRL handled to perform pin operation
- MON triggered from monitored pins to notify pin state change

CTRL event operations:
- assign pin (R/W/M: read/write/monitor)
- read pin value
- write pin value

EVT struct for CTRL
{
  type: PIN,
  mode: CTRL,
  oper: assign
}
PinEvt
assign
{

}
read
{

}
write
{

}
monitor
{

}

PinEvt
------
 read   \     +--------------+
 write ---->  | GPIO Control | 
              +--------------+

 +-------------+
 | PinObserver | --> monitor EVT
 +-------------+

*/

// base class holding pin state
class PinState {
  int pinId;
  int pinVal;
  static std::map<int, PinState *> pinAssignments;
  static getPinInstance(int pinId);
};
/*
* Watching pin state and triggering PinEvt when state change
* Read mode
*/
class PinStateObserver : public PinState {
  int watchRate; // how often pin state is read
  static std::vector<PinStateObserver *> *observers;
  int read();
  public:
  
};

/*
* Controlling pin state by listening to pin control events
* Write mode
*/
class PinStateController : public PinState {
  static std::vector<PinStateController *> *controllers;
  void write(int val);
};


class PwmPinStateObserver : public PinStateObserver {};
// PWM write access
class PwmPinStateController : public PinStateController {};

/*
 * PinStateObserver
 */
int PinStateObserver::read() {
  std::string pinEvt("");
  // pushEvent(pinEvt, EventType.PinEvt);
};
