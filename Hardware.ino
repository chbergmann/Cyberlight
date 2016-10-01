/********************************************************************************************\
* Initialize specific hardware setings (only global ones, others are set through devices)
\*********************************************************************************************/

void hardwareInit()
{
  LoadPinStates();
  int i;
  for(i=0; i<PINSTATE_TABLE_MAX; i++)
  {
    if(pinStates[i].plugin > 0)
    {
      Serial.printf("Setting pin %d to %d\r\n", pinStates[i].index, pinStates[i].value);

      if(pinStates[i].mode == PIN_MODE_OUTPUT)
      {
        pinMode(pinStates[i].index,OUTPUT);
        digitalWrite(pinStates[i].index, pinStates[i].value);
        Settings.PinBootStates[pinStates[i].index] = 0;
      }
      else if(pinStates[i].mode == PIN_MODE_PWM)
      {
        pinMode(pinStates[i].index,OUTPUT);
        analogWrite(pinStates[i].index, pinStates[i].value);
        Settings.PinBootStates[pinStates[i].index] = 0;
      }
    }
  }

  // set GPIO pins state if not set to default
  for (byte x=0; x < 17; x++)
    if (Settings.PinBootStates[x] != 0)
      switch(Settings.PinBootStates[x])
      {
        case 1:
          pinMode(x,OUTPUT);
          digitalWrite(x,LOW);
          setPinState(1, x, PIN_MODE_OUTPUT, LOW);
          break;
        case 2:
          pinMode(x,OUTPUT);
          digitalWrite(x,HIGH);
          setPinState(1, x, PIN_MODE_OUTPUT, HIGH);
          break;
        case 3:
          pinMode(x,INPUT_PULLUP);
          setPinState(1, x, PIN_MODE_INPUT, 0);
          break;
      }

  // configure hardware pins according to eeprom settings.
  if (Settings.Pin_i2c_sda != -1)
  {
    String log = F("INIT : I2C");
    addLog(LOG_LEVEL_INFO, log);
    Wire.begin(Settings.Pin_i2c_sda, Settings.Pin_i2c_scl);
    #if ESP_CORE >= 210
      if(Settings.WireClockStretchLimit)
      {
        String log = F("INIT : I2C custom clockstretchlimit:");
        log += Settings.WireClockStretchLimit;
        addLog(LOG_LEVEL_INFO, log);
        Wire.setClockStretchLimit(Settings.WireClockStretchLimit);
      }
    #endif
  }

  // I2C Watchdog boot status check
  if (Settings.WDI2CAddress != 0)
  {
    delay(500);
    Wire.beginTransmission(Settings.WDI2CAddress);
    Wire.write(0x83);             // command to set pointer
    Wire.write(17);               // pointer value to status byte
    Wire.endTransmission();

    Wire.requestFrom(Settings.WDI2CAddress, (uint8_t)1);
    if (Wire.available())
    {
      byte status = Wire.read();
      if (status & 0x1)
      {
        String log = F("INIT : Reset by WD!");
        addLog(LOG_LEVEL_ERROR, log);
        lastBootCause = BOOT_CAUSE_EXT_WD;
      }
    }
  }
}
