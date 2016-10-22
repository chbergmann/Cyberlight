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

  int huelevel, sat, lumlevel;
  Plugin_105_GetHSL(&huelevel, &sat, &lumlevel);

  String html = F("<html><head><title>Cyberlight</title>");
  html += F("<script type='text/javascript'>");
  html += F("function submit() { document.getElementById('colorForm').submit(); }");
  html += F("function submitH() { document.getElementById('hueForm').submit(); }");
  html += F("</script><style>");
  html += F("body { font-size:x-large; text-align:center; }");
  html += F("#R { background-color:red; }");
  html += F("#G { background-color:green; }");
  html += F("#B { background-color:blue; }");
  html += F("#W { background-color:#FFFFCC; }");
  html += F("#HUE { background: linear-gradient(90deg, #FF0000, #FFFF00, #00FF00, #00FFFF, #0000FF, #FF00FF, #FF0000) }");
  html += F("#LUM { background: linear-gradient(90deg, black, #FFFFCC) }");
  html += F("#byTime: { transform:scale(4); margin:20 }");
  html += F("#R, #G, #B, #W, #HUE, #LUM { width:99%; height:6%; margin:10; }");
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
  //html += F("<TR><TD colspan='2'><input value='Update' type='submit'/>");
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
