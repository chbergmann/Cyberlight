//#######################################################################################################
//######################################## Cyberlight website ###########################################
//#######################################################################################################

boolean cyberlight_wakeup = false;

void handle_cyberlight()
{
  String red = WebServer.arg("R");
  String grn = WebServer.arg("G");
  String blu = WebServer.arg("B");
  String whi = WebServer.arg("W");
  String byTime = WebServer.arg("byTime");
  String hue = WebServer.arg("HUE");
  String lum = WebServer.arg("LUM");
  boolean setByTime = Settings.plugin105_setColorByTime;

  if(red != "")
  {
    Settings.plugin105_setColorByTime = false;
    Plugin_105_SetRGBW(red.toInt(), grn.toInt(), blu.toInt(), whi.toInt());
    Settings.plugin105_setColorByTime = setByTime;
  }

  if(hue != "")
  {
    Plugin_105_SetColorsByHSL(hue.toInt(), 1023, lum.toInt());
    setByTime = (byTime == "on");

    if(setByTime != Settings.plugin105_setColorByTime)
    {
      Settings.plugin105_setColorByTime = setByTime;
      Plugin_105_SetColors();
      SaveSettings();
    }
  }

  if(WebServer.arg("onButton") == "ON")
  {
      Plugin_105_wakeup();
      cyberlight_wakeup = false;
  }
  if(WebServer.arg("offButton") == "OFF")
  {
      cyberlight_wakeup = true;
      Plugin_105_Fade(0, 0, Settings.CyberlightSettings.fadeOut_sec);
  }

  int huelevel, sat, lumlevel;
  Plugin_105_GetHSL(&huelevel, &sat, &lumlevel);

  String html = F("<html><head><title>Cyberlight</title>");
  html += F("<script type='text/javascript'>");
  html += F("function submit() { document.getElementById('colorForm').submit(); }");
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
  html += F("<input id='HUE' min='0' max='360' type='range' name='HUE' value='");
  html += huelevel;
  html += F("' onchange='submitH()'/><br>");
  html += F("<input id='LUM' min='0' max='1023' type='range' name='LUM' value='");
  html += lumlevel;
  html += F("' onchange='submitH()'/><hr>");
  html += Plugin_106_GetTime();
  html += F("</form></p><p>");

  html += F("<form id='alarmclockForm' method='post'>");
  html += F("<input id='button' type='submit' name='onButton' value='ON'/>");
  html += F("<input id='button' type='submit' name='offButton' value='OFF'/>");
  html += F("</form></p><a href='clconfig'>Config</a>");
  html += F("</body></html>");

  WebServer.send(200, "text/html", html);
}

String print_slider(int pin, String name)
{
  String html = F("<input id='");
  html += name;
  html += F("' min='0' max='1023' type='range' name='");
  html += name;
  html += F("' value='");
  html += Plugin_105_GetRGBW(pin);
  html += F("' onchange='submit()'/><br>");
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
  html += F("#W, #button { background-color:#FFFFCC; }");
  html += F("#HUE { background: linear-gradient(90deg, #FF0000, #FFFF00, #00FF00, #00FFFF, #0000FF, #FF00FF, #FF0000) }");
  html += F("#LUM { background: linear-gradient(90deg, black, #FFFFCC) }");
  html += F("#byTime: { transform:scale(4); margin:20 }");
  html += F("#R, #G, #B, #W, #HUE, #LUM { width:99%; height:6%; margin:10; }");
  html += F("input { font-size:xx-large; text-align:right; }");
  html += F("input[type='checkbox']{ transform: scale(3); }");
  html += F("#Time { width:10%; }");
  html += F("table { width:100%; font-size:xx-large; }");
  html += F("a, #button {padding:5px 15px; background-color:#0077dd; color:#fff; border:solid 1px #fff; text-decoration:none}");
  html += F("</style>");
  return html;
}

void handle_clconfig()
{
  String action = WebServer.arg("action");
  if(action == "Save")
  {
    Settings.CyberlightSettings.wakeup_h = WebServer.arg("Wake_h").toInt();
    Settings.CyberlightSettings.wakeup_min = WebServer.arg("Wake_min").toInt();
    Settings.plugin105_setColorByTime = (WebServer.arg("byTime") == "on");
    Settings.CyberlightSettings.fadeIn_sec = WebServer.arg("fadeIn_sec").toInt();
    Settings.CyberlightSettings.fadeOut_sec = WebServer.arg("fadeOut_sec").toInt();
    Settings.CyberlightSettings.fadeIn_extra_sec = WebServer.arg("fadeIn_extra_sec").toInt();
    Settings.CyberlightSettings.fadeIn_extra_from_h = WebServer.arg("fadeIn_extra_from_h").toInt();
    Settings.CyberlightSettings.fadeIn_extra_from_min = WebServer.arg("fadeIn_extra_from_min").toInt();
    Settings.CyberlightSettings.fadeIn_extra_to_h = WebServer.arg("fadeIn_extra_to_h").toInt();
    Settings.CyberlightSettings.fadeIn_extra_to_min = WebServer.arg("fadeIn_extra_to_min").toInt();
    SaveSettings();
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
  html += F("<table>");

  html += F("<tr><td>Wake up at</td><td><input type='text' id='Time' name='Wake_h' value='");
  html += Settings.CyberlightSettings.wakeup_h;
  html += F("'/>:<input type='text' id='Time' name='Wake_min' value='");
  if(Settings.CyberlightSettings.wakeup_min < 10)
    html += F("0");
  html += Settings.CyberlightSettings.wakeup_min;
  html += F("'/></td></tr>");

  html += F("<tr><td>Set colors by time</td>");
  html += F("<td><input id='byTime' type='checkbox' name='byTime'");
  if(Settings.plugin105_setColorByTime)
    html += " checked";
  html += F("/></td></tr>");

  html += F("<tr><td>Fade in</td><td><input type='text' id='fadeIn_sec' name='fadeIn_sec' value='");
  html += Settings.CyberlightSettings.fadeIn_sec;
  html += F("'/> seconds</td></tr>");

  html += F("<tr><td>Fade out</td><td><input type='text' id='fadeOut_sec' name='fadeOut_sec' value='");
  html += Settings.CyberlightSettings.fadeOut_sec;
  html += F("'/> seconds</td></tr>");

  html += F("<tr><td colspan='2'><br>Use this fade in time in time interval:</td></tr>");
  html += F("<tr><td>Fade in</td><td><input type='text' name='fadeIn_extra_sec' value='");
  html += Settings.CyberlightSettings.fadeIn_extra_sec;
  html += F("'/> seconds</td></tr>");

  html += F("<tr><td>From</td>");
  html += F("<td><input type='text' id='Time' name='fadeIn_extra_from_h' value='");
  html += Settings.CyberlightSettings.fadeIn_extra_from_h;
  html += F("'/>:<input type='text' id='Time' name='fadeIn_extra_from_min' value='");
  if(Settings.CyberlightSettings.fadeIn_extra_from_min < 10)
    html += F("0");
  html += Settings.CyberlightSettings.fadeIn_extra_from_min;
  html += F("'/></td></tr>");

  html += F("<tr><td>To</td>");
  html += F("<td><input type='text' id='Time' name='fadeIn_extra_to_h' value='");
  html += Settings.CyberlightSettings.fadeIn_extra_to_h;
  html += F("'/>:<input type='text' id='Time' name='fadeIn_extra_to_min' value='");
  if(Settings.CyberlightSettings.fadeIn_extra_to_min < 10)
    html += F("0");
  html += Settings.CyberlightSettings.fadeIn_extra_to_min;
  html += F("'/></td></tr></table><br>");

  html += F("<a href='.'>Back</a> ");
  html += F("<input id='button' type='submit' name='action' value='Save'/></form>");
  html += F("</div></body></html>");

  WebServer.send(200, "text/html", html);
}
