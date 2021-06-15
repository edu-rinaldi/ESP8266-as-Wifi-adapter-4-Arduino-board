# ESP as Wifi module for an Arduino Board
This is a simple code for using ESP8266 board as a WiFi adapter for an Arduino board.

## Contributors
* Me (edu-rinaldi)
* [@gianmarcopicarella](https://github.com/gianmarcopicarella)

## Pin configuration
* **TX :** should be connected to RX pin on the arduino board
* **RX :** should be connected to TX pin on the arduino board
* **GPIO0 :** is used for the WPS configuration (example of use: connect it to a BTN)
* **3.3v :** must be connected to 3.3v
* **EN :** same as 3.3v
* **RST :** \
* **GPIO2 :** this pin is already used for controlling the led on the board
* **GND :** must be on GND

## WPS Configuration
Receiving (digital) **HIGH** value on GPIO0 for at least one second will put the board on *"WPS pairing mode"* (you will see blue led blinking).

## Commands
Commands are sent using the Serial connected using TX and RX pins.
Only 3 possible commands can be sent:
1. Status
2. HTTP Req.
3. Get MAC Address

### Status command syntax
```json
{
  "command": 0
}
```

This command will print on serial:
```json
{
  "response": "CONNECTED|NOTCONNECTED"
}
```

### HTTP Req. command syntax
```json
{
  "command": 1,
  "method": "GET|PUT",
  "payload": "{whatever json object as String}" 
}
```
**Note:** 
* only GET and PUT are supported, you can fork and implement POST or whatever
* put payload only if you use PUT method

This command will print on serial the response (if available):
```json
{
  "response": "{response as json}"
}
```

### Get MAC Address command syntax
```json
{
  "command": 2
}
```
This command will print on serial ESP's MAC Address:
```json
{
  "response": "MACADDRESS"
}
```

## Final notes
The code is thought for a specific application, but the base to use the ESP8266 in other projects is already present, but it is not free from bugs or errors.
Whoever wants to improve the code can do it
