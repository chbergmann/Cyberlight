//#######################################################################################################
//#################################### Plugin 007: ExtWiredAnalog #######################################
//#######################################################################################################

/*********************************************************************************************\
 * This plugin provides support for 4 extra analog inputs, using the PCF8591 (NXP/Philips)
 * Support            : www.esp8266.nu
 * Date               : Oct 2016
 * Compatibility      : R004
 * Syntax             : "ExtWiredRTC <Par1:Port>, <Par2:Variable>"
 *********************************************************************************************
 * Technical description:
 *
 * De PCF8583 is a IO Expander chip that connects through the I2C bus
 * Basic I2C address = 0x51
 \*********************************************************************************************/
#define PLUGIN_106
#define PLUGIN_ID_106         106
#define PLUGIN_NAME_106       "Real Time Clock - PCF8583"
#define PLUGIN_VALUENAME1_106 "Hundredth second"
#define PLUGIN_VALUENAME2_106 "Second"
#define PLUGIN_VALUENAME3_106 "Minute"
#define PLUGIN_VALUENAME4_106 "Hour"
#define PLUGIN_VALUENAME5_106 "Weekday"
#define PLUGIN_VALUENAME6_106 "Day"
#define PLUGIN_VALUENAME7_106 "Month"
#define PLUGIN_VALUENAME8_106 "Year"
#define I2CADDR 0x51

const char weekdays[7][4] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

#define BCD_TO_BIN(x) ((x >> 4) * 10 + (x & 0x0F))
#define BIN_TO_BCD(x) (((x/10) << 4) + (x % 10))

uint8_t Plugin_106_i2c_read8(uint8_t a)
{
  uint8_t ret;

  Wire.beginTransmission(I2CADDR); // start transmission to device
  Wire.write(a); // sends register address to read from
  Wire.endTransmission(); // end transmission

  Wire.beginTransmission(I2CADDR); // start transmission to device
  Wire.requestFrom(I2CADDR, 1);// send data n-bytes read
  ret = Wire.read(); // receive DATA
  Wire.endTransmission(); // end transmission

  return ret;
}

void Plugin_106_i2c_write8(uint8_t a, uint8_t val)
{
  Wire.beginTransmission(I2CADDR); // start transmission to device
  Wire.write(a); // sends register address to read from
  Wire.endTransmission(); // end transmission

  Wire.beginTransmission(I2CADDR); // start transmission to device
  Wire.write(val);
  Wire.endTransmission(); // end transmission
}

void Plugin_106_ReadTime(uint8_t* timerval)
{
    String log = F("RTC  : Time: ");
    int i;
    uint8_t i2cval[7];
    memset(timerval, 0, 8);

    Wire.beginTransmission(I2CADDR); // start transmission to device
    Wire.write(0); // sends register address to read from
    Wire.endTransmission(); // end transmission
    Wire.requestFrom(I2CADDR, 7);
    if (Wire.available())
    {
      for(i=0; i<7; i++)
      {
        char hex[5];
        i2cval[i] = Wire.read();
        sprintf_P(hex, PSTR("%x,"), i2cval[i]);
        log += String(hex);
      }

      timerval[0] = BCD_TO_BIN(i2cval[1]);  // 100th seconds
      timerval[1] = BCD_TO_BIN(i2cval[2]);  // seconds
      timerval[2] = BCD_TO_BIN(i2cval[3]);  // minutes
      timerval[3] = ((i2cval[4] >> 4) & 3) * 10 + (i2cval[4] & 0x0F); // hours
      timerval[4] = (i2cval[6] >> 5);       // weekday
      if(timerval[4] >= 7)
        timerval[4] = 0;
      timerval[5] = ((i2cval[5] & 0x30) >> 4) * 10 + (i2cval[5] & 0x0F);  // day
      timerval[6] = ((i2cval[6] & 0x10) >> 4) * 10 + (i2cval[6] & 0x0F);  // month
      timerval[7] = (i2cval[5] >> 6);       // years after last leap year
  }
  //addLog(LOG_LEVEL_INFO,log);
}

void Plugin_106_WriteTime(uint8_t* timerval)
{
    String log = F("RTC  : Write Time: ");
    int i;
    uint8_t i2cval[6];

    i2cval[0] = BIN_TO_BCD(timerval[0]);
    i2cval[1] = BIN_TO_BCD(timerval[1]);
    i2cval[2] = BIN_TO_BCD(timerval[2]);
    i2cval[3] = BIN_TO_BCD(timerval[3]);
    i2cval[4] = (BIN_TO_BCD(timerval[7]) << 6) + BIN_TO_BCD(timerval[5]);
    i2cval[5] = (BIN_TO_BCD(timerval[4]) << 5) + BIN_TO_BCD(timerval[6]);

    Wire.beginTransmission(I2CADDR); // start transmission to device
    Wire.write(1); // sends register address to read from
    Wire.write(i2cval, 6);
    Wire.endTransmission(); // end transmission

    for(i=0; i<6; i++)
    {
      char hex[5];
      sprintf_P(hex, PSTR("%x,"), i2cval[i]);
      log += String(hex);
    }

    //addLog(LOG_LEVEL_INFO,log);
}

boolean Plugin_106(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_106;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
    		Device[deviceCount].Custom = true;
        break;
      }

  	case PLUGIN_INIT:
    	{
    		LoadTaskSettings(event->TaskIndex);
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_106);
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        uint8_t timerval[8];
        Plugin_106_ReadTime(timerval);

        char timestr[256];
        sprintf_P(timestr, PSTR("<TR><TD>Time:</TD><TD><input type='text' name='plugin_106_hour' value='%u'/>:"), timerval[3]);
        string += timestr;
        sprintf_P(timestr, PSTR("<input type='text' name='plugin_106_min' value='%u'/>:"), timerval[2]);
        string += timestr;
        sprintf_P(timestr, PSTR("<input type='text' name='plugin_106_sec' value='%u'/></TD></TR>"), timerval[1]);
        string += timestr;

        sprintf_P(timestr, PSTR("<TR><TD>Date:</TD><TD><input type='text' name='plugin_106_weekday' value='%s'/> "), weekdays[timerval[4]]);
        string += timestr;
        sprintf_P(timestr, PSTR("<input type='text' name='plugin_106_day' value='%u'/>."), timerval[5]);
        string += timestr;
        sprintf_P(timestr, PSTR("<input type='text' name='plugin_106_month' value='%u'/></TD></TR>"), timerval[6]);
        string += timestr;

        sprintf_P(timestr, PSTR("<TR><TD>Leap year:</TD><TD>+<input type='text' name='plugin_106_year' value='%u'/></TD></TR>"), timerval[7]);
        string += timestr;

        success = true;
        break;
      }

    	case PLUGIN_WEBFORM_SAVE:
      	{
          uint8_t timerval[8];
          int i;
      		String plugin1 = WebServer.arg("plugin_106_hour");
      		timerval[3] = plugin1.toInt();
      		String plugin2 = WebServer.arg("plugin_106_min");
      		timerval[2] = plugin2.toInt();
      		String plugin3 = WebServer.arg("plugin_106_sec");
      		timerval[1] = plugin3.toInt();
          timerval[0] = 0;
      		String plugin4 = WebServer.arg("plugin_106_weekday");
      		timerval[4] = 0;
          for(i=0; i<7; i++)
          {
            if(String(weekdays[i]) == plugin4)
            {
              timerval[4] = i;
              break;
            }
          }
      		String plugin5 = WebServer.arg("plugin_106_day");
      		timerval[5] = plugin5.toInt();
          String plugin6 = WebServer.arg("plugin_106_month");
          timerval[6] = plugin6.toInt();
          String plugin7 = WebServer.arg("plugin_106_year");
          timerval[7] = plugin7.toInt();

          Plugin_106_WriteTime(timerval);
      		success = true;
      		break;
      	}

    case PLUGIN_WEBFORM_SHOW_VALUES:
      {
        string += Plugin_106_GetTime();
        success = true;
        break;
      }
  }
  return success;
}

String Plugin_106_GetTime()
{
  uint8_t timerval[8];
  char timestr[100];

  Plugin_106_ReadTime(timerval);
  if(timerval[5] == 0)
    return "RTC not found !";
  else
  {
    sprintf_P(timestr, PSTR("%d:%02d:%02d %s %d.%d"), timerval[3], timerval[2], timerval[1],
      weekdays[timerval[4]], timerval[5], timerval[6]);
    return timestr;
  }
}
