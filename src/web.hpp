#include<Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
ESP8266WebServer server(80);
void handleRoot() {
  server.send(200, "text/html", R"(
 <meta charset="utf-8">
 <meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
body{
    margin: 0px;
    background-color: #111;
}
.c{
    font-size: 40pt;
    margin: auto;
    text-align: center;
    background-color: #333;
    color: #eee;
    padding: 5pt;
}
.a{
    margin: auto;
    display: block;
    text-align: center;
    max-width: 500pt;
height: 100%;
background-color: #333;
}
</style>
<div class="a">
<div class="c">
    🔌Energy Meter⚡
</div>
<div class="c">Usage:</div>
<div class="c"><span id="w"></span> W</div>
<div class="c"><span id="kwh"></span> kWh</div>
</div>
<script>
var d = document;
var $ = (e)=>d.getElementById(e);
function loadDoc() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function(e) {
    if (this.readyState == 4 && this.status == 200) {
p=JSON.parse(xhttp.response);
     $("kwh").innerHTML = Math.round(p.meter[2].value/1000);
     $("w").innerHTML = Math.round(p.meter[5].value);
  setTimeout(loadDoc,1000);
    
    }
  };
  xhttp.open("GET", "/s", true);
  xhttp.send();
}
document.onload=loadDoc();
</script>
  )");
}
void ondata(){
    server.send(200,"application/octet-stream",downloadbuffer,downloadbuffersize);
}
void onsmldata(){
    server.send(200,"application/json",data.c_str());
}
void web_setup(){
    Serial.println("start web");
    server.on("/", handleRoot);  //Associate handler function to path
    server.on("/d",ondata);
    server.on("/s",onsmldata);
    server.begin();
}
void web_loop(){
    server.handleClient();
}