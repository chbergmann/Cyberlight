
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
  String wake_h = WebServer.arg("Wake_h");
  String wake_m = WebServer.arg("Wake_min");
  String fade = WebServer.arg("Fade");
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

  if(wake_h != "" && wake_m != "")
  {
    Settings.cyberlight_wakeup_h = wake_h.toInt();
    Settings.cyberlight_wakeup_min = wake_m.toInt();
    Settings.cyberlight_fade_sec = fade.toInt();
    cyberlight_wakeup = true;
    int i;
    for(i=0; i<4; i++)
      Plugin_105_Fade(i, 0, 1);
  }

  int huelevel, sat, lumlevel;
  Plugin_105_GetHSL(&huelevel, &sat, &lumlevel);

  String html = F("<html><head><title>Cyberlight</title>");
  html += F("<script type='text/javascript'>");
  html += F("function submit() { document.getElementById('colorForm').submit(); }");
  html += F("function submitH() { document.getElementById('hueForm').submit(); }");
  html += F("</script><style>");
  html += F("body { font-size:xx-large; text-align:center; }");
  html += F("h1 { font-size:xx-large; }");
  html += F("#R { background-color:red; }");
  html += F("#G { background-color:green; }");
  html += F("#B { background-color:blue; }");
  html += F("#W { background-color:#FFFFCC; }");
  html += F("#HUE { background: linear-gradient(90deg, #FF0000, #FFFF00, #00FF00, #00FFFF, #0000FF, #FF00FF, #FF0000) }");
  html += F("#LUM { background: linear-gradient(90deg, black, #FFFFCC) }");
  html += F("#byTime: { transform:scale(4); margin:20 }");
  html += F("#R, #G, #B, #W, #HUE, #LUM { width:99%; height:6%; margin:10; }");
  html += F("input { font-size:x-large; }");
  html += F("#Wake { width:5%; text-align:right; }");
  html += F("</style></head>");
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
  html += F(" - Set colors by time ");
  html += F("<input id='byTime' type='checkbox' name='byTime'");
  if(Settings.plugin105_setColorByTime)
    html += " checked";

  html += F(" onchange='submitH()'/>");
  html += F("</form></p><p>");
  html += F("<form id='alarmclockForm' method='post'>");
  html += F("Wake up at <input type='text' id='Wake' name='Wake_h' value='");
  html += Settings.cyberlight_wakeup_h;
  html += F("'/>:<input type='text' id='Wake' name='Wake_min' value='");
  html += Settings.cyberlight_wakeup_min;
  html += F("'/> Fade in in <input type='text' id='Wake' name='Fade' value='");
  html += Settings.cyberlight_fade_sec;
  html += F("'/> seconds <input type='submit' value='Good night !'>");
  html += F("</form></p><a href='espeasy'>Config</a>");
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
