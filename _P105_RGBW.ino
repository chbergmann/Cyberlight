//#######################################################################################################
//#################################### Plugin 105: RGBW / Milight #######################################
//#######################################################################################################

#define PLUGIN_105
#define PLUGIN_ID_105         105
#define PLUGIN_NAME_105       "RGBW MiLight"

#include <Ticker.h>

boolean Plugin_105_init = false;
WiFiUDP Plugin_105_milightUDP;
int Plugin_105_FadingRate = 50; //   was 10hz
unsigned int Plugin_105_UDPCmd = 0;
unsigned int Plugin_105_UDPParameter = 0;
Ticker Plugin_105_Ticker;
int secondCount = 0;

struct Plugin_105_structPins
{
	unsigned long FadingTimer = 0;
	int CurrentLevel = 0;
	int FadingTargetLevel = 0;
	int FadingMMillisPerStep = 0;
	int FadingDirection = 0;
	int PinNo = 0;
} Plugin_105_Pins[4];

struct Plugin_105_structRGBFlasher
{
	unsigned int Count = 0;
	unsigned int OnOff = 0;
	unsigned int Freq = 0;
	unsigned int Red = 0;
	unsigned int Green = 0;
	unsigned int Blue = 0;
	unsigned int White = 0;
} Plugin_105_RGBFlasher;

struct Plugin_105_structMiLight
{
	float HueLevel = 0;
	float LumLevel = 0.5;
	float SatLevel = 1;
  int HueOffset = -5;
	boolean ColourOn = false;
	boolean WhiteOn = false;
  boolean RGBWhiteOn = false;
	unsigned int UDPPort = 0;
} Plugin_105_MiLight;

boolean Plugin_105(byte function, struct EventStruct *event, String& string)
{
	boolean success = false;

	switch (function)
	{
	case PLUGIN_DEVICE_ADD:
	{
		Device[++deviceCount].Number = PLUGIN_ID_105;
		Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
		Device[deviceCount].Custom = true;
		Device[deviceCount].VType = SENSOR_TYPE_DIMMER;
		Device[deviceCount].Ports = 0;
		break;
	}

	case PLUGIN_GET_DEVICENAME:
	{
		string = F(PLUGIN_NAME_105);
		break;
	}

	case PLUGIN_GET_DEVICEVALUENAMES:
	{
		break;
	}

	case PLUGIN_WEBFORM_LOAD:
	{
		char tmpString[128];
		sprintf_P(tmpString, PSTR("<TR><TD>Milight UDP Port:<TD><input type='text' name='plugin_105_port' value='%u'>"), ExtraTaskSettings.TaskDevicePluginConfigLong[0]);
		string += tmpString;
		sprintf_P(tmpString, PSTR("<TR><TD>Red Pin:<TD><input type='text' name='plugin_105_RedPin' value='%u'>"), Settings.plugin105_pinNo[0]);
		string += tmpString;
		sprintf_P(tmpString, PSTR("<TR><TD>Green Pin:<TD><input type='text' name='plugin_105_GreenPin' value='%u'>"), Settings.plugin105_pinNo[1]);
		string += tmpString;
		sprintf_P(tmpString, PSTR("<TR><TD>Blue Pin:<TD><input type='text' name='plugin_105_BluePin' value='%u'>"), Settings.plugin105_pinNo[2]);
		string += tmpString;
		sprintf_P(tmpString, PSTR("<TR><TD>White Pin:<TD><input type='text' name='plugin_105_WhitePin' value='%u'>"), Settings.plugin105_pinNo[3]);
    string += tmpString;
    sprintf_P(tmpString, PSTR("<TR><TD>Hue Offset:<TD><input type='text' name='plugin_105_HueOffset' value='%d'>"), ExtraTaskSettings.TaskDevicePluginConfigLong[1]);
		string += tmpString;
    string += "<TR><TD>Set color by time<TD><input type='checkbox' name='plugin_106_timehue'";
    if(Settings.plugin105_setColorByTime)
      string += " checked";
    string += ">";
    sprintf_P(tmpString, PSTR("<TR><TD>Hue offset at midnight:<TD><input type='text' name='plugin_106_hueoffset' value='%u'>"),
      Settings.plugin105_hueOffsetMidnight);
    string += tmpString;
		success = true;
		break;
	}

	case PLUGIN_WEBFORM_SAVE:
	{
		String plugin1 = WebServer.arg("plugin_105_port");
		ExtraTaskSettings.TaskDevicePluginConfigLong[0] = plugin1.toInt();
		String plugin2 = WebServer.arg("plugin_105_RedPin");
		Settings.plugin105_pinNo[0] = plugin2.toInt();
		String plugin3 = WebServer.arg("plugin_105_GreenPin");
		Settings.plugin105_pinNo[1] = plugin3.toInt();
		String plugin4 = WebServer.arg("plugin_105_BluePin");
		Settings.plugin105_pinNo[2] = plugin4.toInt();
		String plugin5 = WebServer.arg("plugin_105_WhitePin");
		Settings.plugin105_pinNo[3] = plugin5.toInt();
    String plugin6 = WebServer.arg("plugin_105_HueOffset");
    ExtraTaskSettings.TaskDevicePluginConfigLong[1] = plugin6.toInt();
		String plugin8 = WebServer.arg("plugin_106_timehue");
		if(plugin8 == "on")
			Settings.plugin105_setColorByTime = 1;
		else
			Settings.plugin105_setColorByTime = 0;

		String plugin9 = WebServer.arg("plugin_106_hueoffset");
		Settings.plugin105_hueOffsetMidnight = plugin9.toInt();

		Plugin_105_pinsetup();
		SaveTaskSettings(event->TaskIndex);
		SaveSettings();
		success = true;
		break;
	}

	case PLUGIN_INIT:
	{
		LoadTaskSettings(event->TaskIndex);

		//udp port
		Plugin_105_MiLight.UDPPort = ExtraTaskSettings.TaskDevicePluginConfigLong[0];
		Plugin_105_MiLight.HueOffset = ExtraTaskSettings.TaskDevicePluginConfigLong[1];
		if (Plugin_105_MiLight.UDPPort != 0)
		{
			if (Plugin_105_milightUDP.begin(Plugin_105_MiLight.UDPPort)) addLog(LOG_LEVEL_INFO, "INIT: Milight UDP");
		}

		//rgbw gpio pins
		boolean SetupTimer = false;

		Plugin_105_pinsetup();
		for (int PinIndex = 0; PinIndex < 4; PinIndex++)
		{
			if (Plugin_105_Pins[PinIndex].PinNo != 0)
			{
				SetupTimer = true;
			}
		}

		if (SetupTimer == true)
		{
			addLog(LOG_LEVEL_INFO, "INIT: Milight Fading Timer");
			Plugin_105_Ticker.attach_ms(20, Plugin_105_FadingTimer);
		}

		Plugin_105_init = true;
		success = true;
		break;
	}

	case PLUGIN_TEN_PER_SECOND:
	{
		if (Plugin_105_init)
		{

			// UDP events for milight emulation
			if (Plugin_105_MiLight.UDPPort != 0)
			{
				int packetSize = Plugin_105_milightUDP.parsePacket();
				if (packetSize)
				{
					char packetBuffer[128];
					int len = Plugin_105_milightUDP.read(packetBuffer, 128);
					if (len == 3 && packetBuffer[2] == 85)
					{
						/*
					  Serial.print("Commands received ");
					  Serial.print(int(packetBuffer[0]));
            Serial.print(", ");
					  Serial.println(int(packetBuffer[1]));
            //Serial.print(", ");
					  //Serial.print(int(packetBuffer[2]));
						*/
						Plugin_105_UDPCmd = packetBuffer[0];
						Plugin_105_UDPParameter = packetBuffer[1];
						Plugin_105_ProcessUDP();
					}
				}
			}

			//RGB flashing
			if (Plugin_105_RGBFlasher.Count > 0 && millis() > Plugin_105_RGBFlasher.Freq)
			{
				Plugin_105_RGBFlasher.Freq = millis() + 500; //half second flash rate
				Plugin_105_RGBFlasher.OnOff = 1 - Plugin_105_RGBFlasher.OnOff;
				if (Plugin_105_RGBFlasher.OnOff == 1)
				{
					analogWrite(Plugin_105_Pins[0].PinNo, Plugin_105_RGBFlasher.Red);
					analogWrite(Plugin_105_Pins[1].PinNo, Plugin_105_RGBFlasher.Green);
					analogWrite(Plugin_105_Pins[2].PinNo, Plugin_105_RGBFlasher.Blue);
				}
				else
				{
					analogWrite(Plugin_105_Pins[0].PinNo, 0);
					analogWrite(Plugin_105_Pins[1].PinNo, 0);
					analogWrite(Plugin_105_Pins[2].PinNo, 0);
					Plugin_105_RGBFlasher.Count = Plugin_105_RGBFlasher.Count - 1;
				}
				if (Plugin_105_RGBFlasher.Count == 0)
				{
					if (Plugin_105_MiLight.ColourOn == true)
					{
						addLog(LOG_LEVEL_INFO, "Restoring to colour...");
						analogWrite(Plugin_105_Pins[0].PinNo, Plugin_105_Pins[0].CurrentLevel);
						analogWrite(Plugin_105_Pins[1].PinNo, Plugin_105_Pins[1].CurrentLevel);
						analogWrite(Plugin_105_Pins[2].PinNo, Plugin_105_Pins[2].CurrentLevel);
					}
					addLog(LOG_LEVEL_INFO, "Flashing RGB complete");
				}
			}

			//Fading - moved to timer section


		}

		success = true;
		break;
	}

	case PLUGIN_ONCE_A_SECOND:
	{
		int i;

		secondCount++;
		if(secondCount >= 60)
		{
			secondCount = 0;
			Plugin_105_SetColors();
			Plugin_105_CheckPinChange();
		}

		if(secondCount % 5 == 0)
		{
			if(Plugin_105_CheckPinChange())
				SaveSettings();
		}
		break;
	}

	case PLUGIN_WRITE:
	{
		int Par[8];
		char command[80];
		command[0] = 0;
		char TmpStr1[80];
		TmpStr1[0] = 0;
//Serial.println(string);
		string.toCharArray(command, 80);
		Par[1] = 0;
		Par[2] = 0;
		Par[3] = 0;
		Par[4] = 0;
		Par[5] = 0;
		Par[6] = 0;
		Par[7] = 0;

		String tmpString = string;
		int argIndex = tmpString.indexOf(',');
		if (argIndex) tmpString = tmpString.substring(0, argIndex);

		if (GetArgv(command, TmpStr1, 2)) Par[1] = str2int(TmpStr1);
		if (GetArgv(command, TmpStr1, 3)) Par[2] = str2int(TmpStr1);
		if (GetArgv(command, TmpStr1, 4)) Par[3] = str2int(TmpStr1);
		if (GetArgv(command, TmpStr1, 5)) Par[4] = str2int(TmpStr1);
		if (GetArgv(command, TmpStr1, 6)) Par[5] = str2int(TmpStr1);
		if (GetArgv(command, TmpStr1, 7)) Par[6] = str2int(TmpStr1);
		if (GetArgv(command, TmpStr1, 8)) Par[7] = str2int(TmpStr1);

		//initialise LED Flashing if not flashing already
		if (tmpString.equalsIgnoreCase("RGBFLASH") && Plugin_105_RGBFlasher.Count == 0 && Par[1] <= 1023 && Par[2] <= 1023 && Par[3] <= 1023 && Par[4] > 0 && Par[4] <= 20)
		{
			success = true;
			Plugin_105_RGBFlasher.Red = Par[1];
			Plugin_105_RGBFlasher.Green = Par[2];
			Plugin_105_RGBFlasher.Blue = Par[3];
			Plugin_105_RGBFlasher.Count = Par[4];
			Plugin_105_RGBFlasher.OnOff = 0;
			Plugin_105_RGBFlasher.Freq = millis() + 500;

			//conclude any ongoing rgb fades
			for (int PinIndex = 0; PinIndex < 3; PinIndex++)
			{
				if (Plugin_105_Pins[PinIndex].FadingDirection != 0)
				{
					Plugin_105_Pins[PinIndex].FadingDirection = 0;
					Plugin_105_Pins[PinIndex].CurrentLevel = Plugin_105_Pins[PinIndex].FadingTargetLevel;
				}
			}

			if (printToWeb)
			{
				printWebString += F("RGB flashing for ");
				printWebString += Par[4];
				printWebString += F(" times.");
				printWebString += F("<BR>");
			}
			addLog(LOG_LEVEL_INFO, "Start PWM Flash");
		}

		//initialise LED Fading pin 0=r,1=g,2=b,3=w
		if (tmpString.equalsIgnoreCase("PWMFADE"))
		{
			success = true;
			if (Par[2] >= 0 && Par[2] <= 1023 && Par[1] >= 0 && Par[1] <= 3 && Par[3] > 0 && Par[3] < 30)
			{
				if (Par[1] == 3 || (Plugin_105_RGBFlasher.Count == 0 && Plugin_105_RGBFlasher.OnOff == 0)) //white pin or no flashing going so init fade
				{
					Plugin_105_Pins[Par[1]].FadingTargetLevel = Par[2];
					Plugin_105_Pins[Par[1]].FadingMMillisPerStep = 1000 / Plugin_105_FadingRate;
					Plugin_105_Pins[Par[1]].FadingDirection = (abs(Plugin_105_Pins[Par[1]].FadingTargetLevel - Plugin_105_Pins[Par[1]].CurrentLevel)) / (Plugin_105_FadingRate * Par[3]);
					if (Plugin_105_Pins[Par[1]].FadingDirection == 0) { Plugin_105_Pins[Par[1]].FadingDirection = 1; }
					if (Plugin_105_Pins[Par[1]].CurrentLevel == Plugin_105_Pins[Par[1]].FadingTargetLevel) { Plugin_105_Pins[Par[1]].FadingDirection = 0; }
					if (Plugin_105_Pins[Par[1]].CurrentLevel > Plugin_105_Pins[Par[1]].FadingTargetLevel) { Plugin_105_Pins[Par[1]].FadingDirection = Plugin_105_Pins[Par[1]].FadingDirection * -1; }
					Plugin_105_Pins[Par[1]].FadingTimer = millis();
					if (printToWeb)
					{
						printWebString += F("PWM fading over ");
						printWebString += Par[3];
						printWebString += F(" seconds.");
						printWebString += F("<BR>");
					}
					addLog(LOG_LEVEL_INFO, "Start PWM Fade");
				}
				else // currently flashing so set fade completed
				{
					Plugin_105_Pins[Par[1]].FadingTargetLevel = Par[2];
					Plugin_105_Pins[Par[1]].FadingDirection = 0;
					Plugin_105_Pins[Par[1]].CurrentLevel = Plugin_105_Pins[Par[1]].FadingTargetLevel;
				}
			}
		}

		break;
 		}
	}

	return success;
	}


	void Plugin_105_pinsetup()
	{
	  int i;
	  for(i=0; i < 4; i++)
	  {
			Plugin_105_Pins[i].PinNo = Settings.plugin105_pinNo[i];
			Plugin_105_Pins[i].CurrentLevel = Settings.plugin105_pinValue[i];
			Settings.PinBootStates[Settings.plugin105_pinNo[i]] = 0;
			setPinState(105, i, PIN_MODE_PWM, Plugin_105_Pins[i].CurrentLevel);
			pinMode(Plugin_105_Pins[i].PinNo, OUTPUT);
		}
		Plugin_105_SetColors();
	}

	/************************/
	/* handle fading timer */
	/***********************/
	void Plugin_105_FadingTimer()
	{
		//Fading
		for (int PinIndex = 0; PinIndex < 4; PinIndex++)
		{
			if (Plugin_105_Pins[PinIndex].FadingDirection != 0)
			{
				if (millis() > Plugin_105_Pins[PinIndex].FadingTimer)
				{
					Plugin_105_Pins[PinIndex].FadingTimer = millis() + Plugin_105_Pins[PinIndex].FadingMMillisPerStep;
					Plugin_105_Pins[PinIndex].CurrentLevel = Plugin_105_Pins[PinIndex].CurrentLevel + Plugin_105_Pins[PinIndex].FadingDirection;
					if (Plugin_105_Pins[PinIndex].CurrentLevel >= Plugin_105_Pins[PinIndex].FadingTargetLevel && Plugin_105_Pins[PinIndex].FadingDirection > 0)
					{
						Plugin_105_Pins[PinIndex].FadingDirection = 0;
						Plugin_105_Pins[PinIndex].CurrentLevel = Plugin_105_Pins[PinIndex].FadingTargetLevel;
						addLog(LOG_LEVEL_INFO, "Fade up complete");
					}
					if (Plugin_105_Pins[PinIndex].CurrentLevel <= Plugin_105_Pins[PinIndex].FadingTargetLevel && Plugin_105_Pins[PinIndex].FadingDirection < 0)
					{
						Plugin_105_Pins[PinIndex].FadingDirection = 0;
						Plugin_105_Pins[PinIndex].CurrentLevel = Plugin_105_Pins[PinIndex].FadingTargetLevel;
						addLog(LOG_LEVEL_INFO, "Fade down complete");
					}
					analogWrite(Plugin_105_Pins[PinIndex].PinNo, Plugin_105_Pins[PinIndex].CurrentLevel);
				}
			}
		}
	}

	/**********************************************************************/
	/* handle udp packets for milight emulation*/
	/**********************************************************************/
	void Plugin_105_ProcessUDP()
	{
		boolean MiLightUpdate = false;
		switch (int(Plugin_105_UDPCmd))
		{

		case 65:	 //off
		case 70:
		case 33:
		{
		  if (Plugin_105_RGBFlasher.Count == 0) //only change led colour if not flashing, selected colour will be applied after flashing concludes
      {
				int i;
				for(i=0; i<4; i++)
        	analogWrite(Plugin_105_Pins[i].PinNo, 0);
      }
			break;
		}
		case 66: 	 //on
		case 69:
		case 74:
		{
      if(Plugin_105_MiLight.ColourOn  == false && Plugin_105_MiLight.RGBWhiteOn == false)
      {
        Plugin_105_MiLight.RGBWhiteOn = true;
      }

      MiLightUpdate = true;
			break;
		}
		case 32: 	 //set colour
		case 64:
		{
			Plugin_105_MiLight.HueLevel = 256 - Plugin_105_UDPParameter + 192;
			Plugin_105_MiLight.HueLevel = (Plugin_105_MiLight.HueLevel / 256) * 360 + Plugin_105_MiLight.HueOffset;
			Plugin_105_MiLight.HueLevel = int(Plugin_105_MiLight.HueLevel) % 360;
			Plugin_105_MiLight.HueLevel = Plugin_105_MiLight.HueLevel / 360;
			MiLightUpdate = true;
			Plugin_105_MiLight.ColourOn = true;
      Plugin_105_MiLight.RGBWhiteOn = false;
			Plugin_105_MiLight.LumLevel = 0.5;
			break;
		}
		case 78:	//brightness
		{
			float brightness = Plugin_105_UDPParameter;
			Plugin_105_MiLight.LumLevel = (brightness - 3) / 22;

			MiLightUpdate = true;
			break;
		}
		case 35:	//increase brightness
		{
			Plugin_105_MiLight.LumLevel = Plugin_105_MiLight.LumLevel + 0.05;
			if (Plugin_105_MiLight.LumLevel > 1) Plugin_105_MiLight.LumLevel = 1;
			MiLightUpdate = true;
			break;
		}
		case 36:	//decrease brightness
		{
			Plugin_105_MiLight.LumLevel = Plugin_105_MiLight.LumLevel - 0.05;
			if (Plugin_105_MiLight.LumLevel < 0) Plugin_105_MiLight.LumLevel = 0;
			MiLightUpdate = true;
			break;
		}
		case 39:	//increase saturation
		{
			Plugin_105_MiLight.SatLevel = Plugin_105_MiLight.SatLevel + 0.05;
			if (Plugin_105_MiLight.SatLevel > 1) Plugin_105_MiLight.SatLevel = 1;
			MiLightUpdate = true;
			break;
		}
		case 40:	//decrease saturation
		{
			Plugin_105_MiLight.SatLevel = Plugin_105_MiLight.SatLevel - 0.05;
			if (Plugin_105_MiLight.SatLevel < 0) Plugin_105_MiLight.SatLevel = 0;
			MiLightUpdate = true;
			break;
		}
		case 193:	// Night
		{
			int i;
			for(i=0; i<4; i++)
				Plugin_105_Pins[i].CurrentLevel = 0;

      break;
		}
    case 194:	// White
    {
      Plugin_105_MiLight.ColourOn = false;
      Plugin_105_MiLight.RGBWhiteOn = true;
      MiLightUpdate = true;
      break;
    }
		}

		if (MiLightUpdate == true && Plugin_105_RGBFlasher.Count == 0) //only change led colour if not flashing, selected colour will be applied after flashing concludes
		{
			if (Plugin_105_MiLight.ColourOn == true)
			{
				Plugin_105_HSL2Rgb(Plugin_105_MiLight.HueLevel, Plugin_105_MiLight.SatLevel, Plugin_105_MiLight.LumLevel);
			}
			if (Plugin_105_MiLight.RGBWhiteOn == true)
     	{
        Plugin_105_Pins[3].CurrentLevel = (Plugin_105_MiLight.LumLevel * 1023);
     	}

		 	int i;
		 	for(i=0; i<4; i++)
      	analogWrite(Plugin_105_Pins[i].PinNo, Plugin_105_Pins[i].CurrentLevel);
/*
      Serial.print("Setting RGB To: ");
      Serial.print(Plugin_105_Pins[0].CurrentLevel);
      Serial.print(", ");
      Serial.print(Plugin_105_Pins[1].CurrentLevel);
      Serial.print(", ");
      Serial.print(Plugin_105_Pins[2].CurrentLevel);
      Serial.print(", ");
      Serial.println(Plugin_105_Pins[3].CurrentLevel);
*/
		}
	}

	float Plugin_105_Hue2RGB(float v1, float v2, float vH)
	{
		if (vH < 0) vH += 1;
		if (vH > 1) vH -= 1;
		if ((6 * vH) < 1) return (v1 + (v2 - v1) * 6 * vH);
		if ((2 * vH) < 1) return v2;
		if ((3 * vH) < 2) return v1 + (v2 - v1) * ((0.66666666666) - vH) * 6;
		return (v1);
	}
	void Plugin_105_HSL2Rgb(float h, float s, float l)
	{
		float holdval;
		float r = 0;
		float g = 0;
		float b = 0;
/*
    Serial.print("Setting HSL To: ");
    Serial.print(h);
    Serial.print(", ");
    Serial.print(s);
    Serial.print(", ");
    Serial.println(l);
*/
		if (s == 0)
		{
			r = 1;
			g = 1;
			b = 1;
		}
		else
		{
			float q;
			if (l < 0.5)
			{
				q = l * (1 + s);
			}
			else
			{
				q = l + s - (l * s);
			}

			float p = (2 * l) - q;

			holdval = h + 0.3333333333;
			r = Plugin_105_Hue2RGB(p, q, holdval);

			holdval = h;
			g = Plugin_105_Hue2RGB(p, q, holdval);

			holdval = h - 0.3333333333;
			b = Plugin_105_Hue2RGB(p, q, holdval);

		}

		r = r * 1023 + 0.5;
		g = g * 1023 + 0.5;
		b = b * 1023 + 0.5;

		Plugin_105_Pins[0].CurrentLevel = floor(r);
		Plugin_105_Pins[1].CurrentLevel = floor(g);
		Plugin_105_Pins[2].CurrentLevel = floor(b);

		//fail safes
		if (Plugin_105_Pins[0].CurrentLevel > 1023)  Plugin_105_Pins[0].CurrentLevel = 1023;
		if (Plugin_105_Pins[1].CurrentLevel > 1023)  Plugin_105_Pins[1].CurrentLevel = 1023;
		if (Plugin_105_Pins[2].CurrentLevel > 1023)  Plugin_105_Pins[2].CurrentLevel = 1023;
	}

bool Plugin_105_CheckPinChange()
{
	bool changed = false;
	int i;

	for(i=0; i<4; i++)
	{
		if(Plugin_105_Pins[i].CurrentLevel != Settings.plugin105_pinValue[i])
		{
			setPinState(PLUGIN_ID_105, Plugin_105_Pins[i].PinNo, PIN_MODE_PWM, Plugin_105_Pins[i].CurrentLevel);
			Settings.plugin105_pinValue[i] = Plugin_105_Pins[i].CurrentLevel;
			changed = true;
		}
	}
	return changed;
}

void Plugin_105_SetColors()
{
	if(Settings.plugin105_setColorByTime)
	{
		uint8_t timerval[8];
		Plugin_106_ReadTime(timerval);
		int minSinceMN = timerval[3] * 60 + timerval[2];
		int hue = ((minSinceMN * 360 / 1440) + Settings.plugin105_hueOffsetMidnight) % 360;
		Plugin_105_MiLight.HueLevel = 1 - (float)hue / 360;
		Plugin_105_HSL2Rgb(Plugin_105_MiLight.HueLevel, Plugin_105_MiLight.SatLevel, Plugin_105_MiLight.LumLevel);

		String logstr = "HUE=";
		logstr += Plugin_105_MiLight.HueLevel;
		addLog(LOG_LEVEL_INFO, logstr);
	}
	Plugin_105_ApplyColors();
}

void Plugin_105_SetRGBW(int r, int g, int b, int w)
{
	Plugin_105_Pins[0].CurrentLevel = r;
	Plugin_105_Pins[1].CurrentLevel = g;
	Plugin_105_Pins[2].CurrentLevel = b;
	Plugin_105_Pins[3].CurrentLevel = w;
	Plugin_105_SetColors();

	int red = Plugin_105_Pins[0].CurrentLevel / 4;
	int green = Plugin_105_Pins[1].CurrentLevel / 4;
	int blue = Plugin_105_Pins[2].CurrentLevel / 4;

  float min = red;
	if(green < min) min = green;
	if(blue < min) min = blue;

  float max = red;
	if(green > max) max = green;
	if(blue > max) max = blue;

  float hue = 0;
  if (max == red) {
      hue = (green - blue) / (max - min);

  } else if (max == green) {
      hue = 2.0 + (blue - red) / (max - min);

  } else {
      hue = 4.0 + (red - green) / (max - min);
  }

  hue = hue * 60;
  if (hue < 0) hue = hue + 360;

	Plugin_105_MiLight.HueLevel = hue / 360;

	Plugin_105_MiLight.LumLevel = (0.333 * Plugin_105_Pins[0].CurrentLevel +
			0.333 * Plugin_105_Pins[1].CurrentLevel +
			0.333 * Plugin_105_Pins[2].CurrentLevel) / 1023;

	Plugin_105_MiLight.SatLevel = 1;
	Plugin_105_ApplyColors();
}

int Plugin_105_GetRGBW(int pin)
{
	return Plugin_105_Pins[pin].CurrentLevel;
}

void Plugin_105_GetHSL(int *hue, int *sat, int *lum)
{
	*hue = Plugin_105_MiLight.HueLevel * 360;
	*lum = Plugin_105_MiLight.LumLevel * 1023;
	*sat = Plugin_105_MiLight.SatLevel * 1023;
}

void Plugin_105_SetColorsByHSL(int hue, int sat, int lum)
{
	float f = (hue /*+	Plugin_105_MiLight.HueOffset*/) % 360;
	Plugin_105_MiLight.HueLevel = f / 360;
	Plugin_105_MiLight.SatLevel = (float)sat / 1023;
	Plugin_105_MiLight.LumLevel = (float)lum / 1023;
	Plugin_105_HSL2Rgb(Plugin_105_MiLight.HueLevel, Plugin_105_MiLight.SatLevel, Plugin_105_MiLight.LumLevel);
	Plugin_105_ApplyColors();
}

void Plugin_105_ApplyColors()
{
 	int i;
 	for(i=0; i<4; i++)
	{
  		analogWrite(Plugin_105_Pins[i].PinNo, Plugin_105_Pins[i].CurrentLevel);
/*
		char tmp[100];
		sprintf_P(tmp, "Pin %d = %d\n", Plugin_105_Pins[i].PinNo, Plugin_105_Pins[i].CurrentLevel);
		addLog(LOG_LEVEL_INFO, tmp);*/
	}
}