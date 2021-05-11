# Environment sensor to be used in OCARIoT pilots

Configuration for each sensor in each Pilot are available on its own branch (synced  with `master`)

Branch naming:
```
  deploy-[pilot_id]-[sensorNr]
```

 Pilot Ids:
 * `EA` - Ellinogermaniki Agogi, Greece
 * `CVE` - Colegio Virgen de Europa, Spain

## Libraries to be added from Arduino Library Manager

* Adafruit_AM2315
* Adafruit_SleepyDog_Library
* ArduinoJson
* WiFi101

### Import as zip (available in `libs` folder)
* ArduinoHttpClient
* NTPClient-master
