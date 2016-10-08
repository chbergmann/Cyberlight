void handle_cyberlight()
{
  String red = WebServer.arg("R");
  String grn = WebServer.arg("G");
  String blu = WebServer.arg("B");
  String whi = WebServer.arg("W");
  if(red != "")
  {
    Plugin_105_SetRGBW(red.toInt(), grn.toInt(), blu.toInt(), whi.toInt());
  }

  String html = F("<html><head><title>Cyberlight</title>");
  html += F("<script type='text/javascript'>");
  html += F("function submit() { document.getElementById('colorForm').submit(); }");
  html += F("</script></head>");
  html += F("<body><h1>Cyberlight</h1>");
  html += F("<form method='post'><table>");
  html += print_slider(0, "R");
  html += print_slider(1, "G");
  html += print_slider(2, "B");
  html += print_slider(3, "W");
  //html += F("<TR><TD colspan='2'><input value='Update' type='submit'/>");
  html += F("</table><br><a href='espeasy'>Config</a>");
  html += F("</form></body></html>");

  WebServer.send(200, "text/html", html);
}

String print_slider(int pin, String name)
{
  String html = "<TR><TD bgcolor='";
  switch(pin)
  {
    case 0: html += "FF0000"; break;
    case 1: html += "00FF00"; break;
    case 2: html += "0000FF"; break;
    case 3: html += "FFFFCC"; break;
  }
  html += "'>";
  html += name;
  html += F("</TD><TD><input id='R' min='0' max='1023' type='range' name='");
  html += name;
  html += F("' value='");
  html += Plugin_105_GetRGBW(pin);
  html += F("' onchange='submit()'/></td><tr/>");
  return html;
}
