//#######################################################################################################
//######################################## Cyberlight website ###########################################
//#######################################################################################################

boolean cyberlight_wakeup = false;
String timestr = "zeitlos";
String modestr = "M0";

void handle_cyberlight()
{
  String red = WebServer.arg("R");
  String grn = WebServer.arg("G");
  String blu = WebServer.arg("B");
  String whi = WebServer.arg("W");
  String byTime = WebServer.arg("byTime");
  String hue = WebServer.arg("HUE");
  String lum = WebServer.arg("LUM");
  String rgb = WebServer.arg("RGB");

  if(red != "")
  {
    Plugin_105_SetRGBW(red.toInt(), grn.toInt(), blu.toInt(), whi.toInt());
	SetMode("M0");
  }

  if(rgb != "")
  {
    int rgb_i = (int) strtol( &rgb[1], NULL, 16);
    int r = (rgb_i & 0xFF0000) >> 16;
    int g = (rgb_i & 0xFF00) >> 8;
    int b = (rgb_i & 0xFF);
    Plugin_105_SetRGB(r, g, b);
	SetMode("M0");
  }

  if(hue != "")
  {
    Plugin_105_SetColorsByHSL(hue.toInt(), 255, lum.toInt());
	SetMode("M0");
  }

  if(WebServer.arg("xButton") == "Fade1")
  {
	  SetMode("M1");
  }
  if(WebServer.arg("xButton") == "Fade2")
  {
	  SetMode("M2");
  }
  if(WebServer.arg("xButton") == "Fade3")
  {
	  SetMode("M3");
  }
  if(WebServer.arg("xButton") == "Fade4")
  {
	  SetMode("M5");
  }
  if(WebServer.arg("xButton") == "Time")
  {
	  SetMode("M4");
  }

  int huelevel, sat, lumlevel;
  Plugin_105_GetHSL(&huelevel, &sat, &lumlevel);

  String html = F("<html><head><title>Cyberlight</title>");
  html += F("<script type='text/javascript'>");
  html += F("function submit() { document.getElementById('colorForm').submit(); }");
  html += F("function submitCP() { document.getElementById('colpickForm').submit(); }");
  html += F("function submitH() { document.getElementById('hueForm').submit(); }");
  html += F("</script>");
  html += print_style();
  html += F("</head>");
  html += F("<body><h1>Cyberlight</h1>");
  html += F("<p><form id='colorForm' method='post'>");
  html += print_slider(0, "R");
  html += print_slider(1, "G");
  html += print_slider(2, "B");
  html += print_slider(3, "W");
  html += F("</form></p><hr><p><form id='hueForm' method='post'>");
  html += F("<div id='HUE'><input id='HUE' min='0' max='360' type='range' name='HUE' value='");
  html += huelevel;
  html += F("' onchange='submitH()'/></div>");
  html += F("<div id='LUM'><input id='LUM' min='0' max='255' type='range' name='LUM' value='");
  html += lumlevel;
  html += F("' onchange='submitH()'/></div>");
  html += F("</form>");

  html += F("<form id='colpickForm' method='post'>");
  html += F("<input id='RGB' type='color' name='RGB' value='");
  char col[100];
  sprintf(col, "#%02X%02X%02X", Plugin_105_GetRGBW(0), Plugin_105_GetRGBW(1), Plugin_105_GetRGBW(2));
  html += col;
  html += F("' onchange='submitCP()'/></form></p><hr>");

  html += timestr;
  html += F("<form id='alarmclockForm' method='post'>");

  html += F("<input type='submit' name='xButton' value='Fade1' id='button_");
  if(modestr == "M1")
	html += "on'/>";
  else
	html += "off'/>";

  html += F("<input type='submit' name='xButton' value='Fade2' id='button_");
  if(modestr == "M2")
	html += "on'/>";
  else
	html += "off'/>";

  html += F("<input type='submit' name='xButton' value='Fade3' id='button_");
  if(modestr == "M3")
	html += "on'/>";
  else
	html += "off'/>";

  html += F("<input type='submit' name='xButton' value='Fade4' id='button_");
  if(modestr == "M5")
	html += "on'/>";
  else
	html += "off'/>";

  html += F("<input type='submit' name='xButton' value='Time' id='button_");
  if(modestr == "M4")
    html += "on'/>";
  else
	html += "off'/>";

  html += F("</form></p><a href='clconfig'>Config</a>");
  html += F("</body></html>");

  WebServer.send(200, "text/html", html);
}

String print_slider(int pin, String name)
{
  String html = F("<div id='");
  html += name;
  html += F("'><input id='");
  html += name;
  html += F("' min='0' max='255' type='range' name='");
  html += name;
  html += F("' value='");
  html += Plugin_105_GetRGBW(pin);
  html += F("' onchange='submit()'/></div>");
  return html;
}

String print_style()
{
  String html = F("<style>");
  html += F("body { font-size:xx-large; text-align:center; }");
  html += F("h1 { font-size:xx-large; }");
  html += F("#left { text-align:left; }");
  html += F("#R { background-color:red; }");
  html += F("#G { background-color:green; }");
  html += F("#B { background-color:blue; }");
  html += F("#W { background-color:#FFFFCC; }");
  html += F("#HUE { background: linear-gradient(90deg, #FF0000, #FFFF00, #00FF00, #00FFFF, #0000FF, #FF00FF, #FF0000) }");
 // html += F("#HUEr { background: linear-gradient(90deg, #FF0000, #FF00FF, #0000FF, #00FFFF, #00FF00, #FFFF00, #FF0000) }");
  html += F("#LUM { background: linear-gradient(90deg, black, #FFFFCC) }");
  html += F("#byTime: { transform:scale(4); margin:20px; }");
  html += F("#R, #G, #B, #W, #HUE, #HUEr, #LUM { height:1em; padding-top:0.5em; padding-bottom:0.5em; }");
  html += F("input { font-size:xx-large; text-align:center;  }");
  html += F("input[type='range'] { height:1em; width:100%; margin:0; padding:0; }");
//  html += F("input[type='checkbox']{ transform: scale(3); margin-left:15px; }");
//  html += F("input[type='submit']{ width:15%; margin:1em; text-align:center; }");
  html += F("input[type='text']{ width:3em; }");
  html += F("#Time { width:10%; }");
  html += F("table { width:100%; font-size:xx-large; }");
  html += F("a, #button, #button_off, #button_on {padding:5px 15px; margin:1em; background-color:#0077dd; color:#fff; border:solid 1px #fff; text-decoration:none}");
  html += F("#button_off {background-color:gray; border:solid 1px; }");
  html += F("#button_on {background-color:#0077dd; }");
  html += F("</style>");
  return html;
}

void handle_clconfig()
{
  String huestr = WebServer.arg("HUE");
  String byTime = WebServer.arg("byTime");
  if(huestr != "")
  {
    Settings.plugin105_hueOffsetMidnight = WebServer.arg("HUE").toInt();
    Serial.println("CD\nH" + huestr + "\nSV");
  }

  if(byTime == "Set by time")
  {
	  SetMode("M4");
  }
  if(byTime == "Timer off")
  {
	  SetMode("M0");
  }

  String timestr2;
  if(WebServer.arg("setTime") == "Set")
  {
	  const char weekdays[7][4] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
	  String wday = WebServer.arg("wday");
	  String wday2 = "00";
	  int i;
	  for(i=0; i<7; i++)
	  {
		  if(wday == String(weekdays[i]))
		  {
			  wday2 = "0" + String(i);
			  break;
		  }
	  }

	  timestr2 = WebServer.arg("sec") + WebServer.arg("min") + WebServer.arg("hour") +
			  wday2 + WebServer.arg("mday") + WebServer.arg("mon") + "00";
	  Serial.println("ST" + timestr2);
  }

  String html = F("<html><head><title>Cyberlight Config</title>");
  html += F("<script type='text/javascript'>");
  html += F("function submit() { document.getElementById('colorForm').submit(); }");
  html += F("</script>");
  html += print_style();
  html += F("</head>");
  html += F("<body><h1>Cyberlight Config</h1>");
  html += F("<div id='left'>");
  html += F("<p><a href='espeasy'>System Settings</a></p>");
  html += F("<p><form id='configForm' method='post'>");
  html += F("<input type='submit' name='byTime' id='button_");
  if(modestr == "M4")
    html += "on' value='Timer off'/>";
  else
	html += "off' value='Set by time'/>";

  html += "<br><br>Color at midnight:<br>";
  html += F("<div id='HUE'><input id='HUE' min='0' max='255' type='range' name='HUE' value='");
  html += Settings.plugin105_hueOffsetMidnight;
  html += F("' onchange='submit()'/></div>");
  html += F("</form>");

  if(timestr2 == "")
  {
	  html += F("<p><form id='timeForm' method='post'>");
	  html += F("<table><tr><td>Time</td><td>");
	  html += F("<input type='text' name='hour' value='");
	  html += timestr.substring(0,2);
	  html += F("'/></td><td>:<input type='text' name='min' value='");
	  html += timestr.substring(3,5);
	  html += F("'/></td><td>:<input type='text' name='sec' value='");
	  html += timestr.substring(6,8);
	  html += F("'/></td><td><input type='text' name='wday' value='");
	  html += timestr.substring(9,12);
	  html += F("'/></td><td><input type='text' name='mday' value='");
	  html += timestr.substring(13,15);
	  html += F("'/></td><td>.<input type='text' name='mon' value='");
	  html += timestr.substring(16,18);
	  html += F("'/></td><td><input type='submit' id='button' name='setTime' value='Set'></td></tr></table></form>");
  }
  else
  {
	  html += F("Time changed.<br>");
  }
  html += F("<a href='.'>Back</a> ");
  html += F("</div></body></html>");

  WebServer.send(200, "text/html", html);
}

void parse_STM8_cyberlight(const char *Command)
{
	if(Command[1] < '0' || Command[1] > '9')
		return;
/*
	if(Command[0] == 'T')
		timestr = String(Command);
	else
		timestr += " " + String(Command);
*/

	switch(Command[0])
	{
	case 'T':
		timestr = String(Command + 1);
		break;
	case 'M':
		modestr = String(Command);
		break;
	case 'R':
		Plugin_105_SetRGBW(0, atoi(Command+1));
		break;
	case 'G':
		Plugin_105_SetRGBW(1, atoi(Command+1));
		break;
	case 'B':
		Plugin_105_SetRGBW(2, atoi(Command+1));
		break;
	case 'W':
		Plugin_105_SetRGBW(3, atoi(Command+1));
		break;
	case 'H':
		Settings.plugin105_hueOffsetMidnight = atoi(Command+1);
		break;
	case 'D':
	case 'F':
		break;
	}
}

void SetMode(String mode)
{
	modestr = mode;
	Serial.println(mode + "\nSV");
}
