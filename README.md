# nodemcu-MqttControlLedStrip

ESP32 NodeMCU

#### How to test
1. Update the file `secret.h` with your wifi credentials. Note: The mqtt credentials are currently unused, as the code is using [shiftr.io](https://shiftr.io/try) as mqtt broker.

```
const char* ssid     = "ssid";
const char* password = "password";

const char* mqttClientId  = "mqttClient";
const char* mqttUsername = "mqttUser";
const char* mqttPassword = "mqttPassword";
```

2. You can view the mqtt traffic [shiftr.io](https://shiftr.io/try).
