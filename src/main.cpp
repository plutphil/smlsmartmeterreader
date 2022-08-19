#include <Arduino.h>
#include "config.hpp"
#include "Sensor.hpp"


#include "ota.hpp"
char *downloadbuffer = new char[1];
size_t downloadbuffersize = 1;
String data = "{\"na\":0}";
#include "web.hpp"
#include "mqtt.hpp"
String tojson(Sensor *sensor, sml_file *file)
{
  String out = "{\"meter\":[ ";
  for (int i = 0; i < file->messages_len; i++)
  {
    sml_message *message = file->messages[i];
    if (*message->message_body->tag == SML_MESSAGE_GET_LIST_RESPONSE)
    {
      sml_list *entry;
      sml_get_list_response *body;
      body = (sml_get_list_response *)message->message_body->data;
      for (entry = body->val_list; entry != NULL; entry = entry->next)
      {
        if (!entry->value)
        { // do not crash on null value
          continue;
        }
        char obisIdentifier[32];
        char buffer[255];

        sprintf(obisIdentifier, "%d-%d:%d.%d.%d/%d",
                entry->obj_name->str[0], entry->obj_name->str[1],
                entry->obj_name->str[2], entry->obj_name->str[3],
                entry->obj_name->str[4], entry->obj_name->str[5]);
        out+="{";
        out+=String("\"obis\":\"")+obisIdentifier+"\"";
        if (((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_INTEGER) ||
            ((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_UNSIGNED))
        {
           out+=String(",\"type\":\"int\"");
          double value = sml_value_to_double(entry->value);
          int scaler = (entry->scaler) ? *entry->scaler : 0;
          int prec = -scaler;
          if (prec < 0)
            prec = 0;
          value = value * pow(10, scaler);
          sprintf(buffer, "%.*f", prec, value);
          //out+=(entryTopic + "    value\n"+ buffer)+"\n";
          out += String(",\"value\":")+buffer;
           sprintf(buffer, "%d-%d : %d.%d.%d * %d  #  %.*f ",
                  entry->obj_name->str[0], entry->obj_name->str[1],
                  entry->obj_name->str[2], entry->obj_name->str[3],
                  entry->obj_name->str[4], entry->obj_name->str[5], prec, value);
          out+=String(",\"info\":\"")+buffer+"\"";
          const char *unit = NULL;
          if (entry->unit && // do not crash on null (unit is optional)
              (unit = dlms_get_unit((unsigned char)*entry->unit)) != NULL)
          out+=String(",\"unit\":\"")+unit+"\"";
        }
        else if (!sensor->config->numeric_only){
          if (entry->value->type == SML_TYPE_OCTET_STRING)
          {
            char *value;
            sml_value_to_strhex(entry->value, &value, true);
           out+=String(",\"type\":\"string\"");
            out+=String(",\"value\":\"")+value+"\"";
            free(value);
          }
          else if (entry->value->type == SML_TYPE_BOOLEAN)
          {
           out+=String(",\"type\":\"bool\"");
            out+=String(",\"value\":")+(entry->value->data.boolean ? "true" : "false");
          }
        }
        out+="},";
      }
    }
  }
  out[out.length()-1]=']';
  out+="}";
  return out;
}
String publishstring(Sensor *sensor, sml_file *file)
{
  String out = "{";
  for (int i = 0; i < file->messages_len; i++)
  {
    sml_message *message = file->messages[i];
    if (*message->message_body->tag == SML_MESSAGE_GET_LIST_RESPONSE)
    {
      sml_list *entry;
      sml_get_list_response *body;
      body = (sml_get_list_response *)message->message_body->data;
      for (entry = body->val_list; entry != NULL; entry = entry->next)
      {
        if (!entry->value)
        { // do not crash on null value
          continue;
        }

        char obisIdentifier[32];
        char buffer[255];

        sprintf(obisIdentifier, "%d-%d:%d.%d.%d/%d",
                entry->obj_name->str[0], entry->obj_name->str[1],
                entry->obj_name->str[2], entry->obj_name->str[3],
                entry->obj_name->str[4], entry->obj_name->str[5]);

        String entryTopic = String("sensor/") + (sensor->config->name) + "/obis/" + obisIdentifier + "/";

        if (((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_INTEGER) ||
            ((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_UNSIGNED))
        {
          double value = sml_value_to_double(entry->value);
          int scaler = (entry->scaler) ? *entry->scaler : 0;
          int prec = -scaler;
          if (prec < 0)
            prec = 0;
          value = value * pow(10, scaler);
          sprintf(buffer, "%.*f", prec, value);
          //out+=(entryTopic + "    value\n"+ buffer)+"\n";
          sprintf(buffer, "%d-%d : %d.%d.%d * %d  #  %.*f ",
                  entry->obj_name->str[0], entry->obj_name->str[1],
                  entry->obj_name->str[2], entry->obj_name->str[3],
                  entry->obj_name->str[4], entry->obj_name->str[5], prec, value);
          out += (buffer);
          const char *unit = NULL;
          if (entry->unit && // do not crash on null (unit is optional)
              (unit = dlms_get_unit((unsigned char)*entry->unit)) != NULL)
            out += unit;
          out += ("\n");
        }
        else if (!sensor->config->numeric_only)
        {
          if (entry->value->type == SML_TYPE_OCTET_STRING)
          {
            char *value;
            sml_value_to_strhex(entry->value, &value, true);
            out += (entryTopic + "   value\n" + value) + "\n";
            free(value);
          }
          else if (entry->value->type == SML_TYPE_BOOLEAN)
          {
            out += (entryTopic + "   value\n" + (entry->value->data.boolean ? "true" : "false")) + "\n";
          }
        }
      }
    }
  }
  return out;
}
Sensor *sensor;
void process_message(byte *buffer, size_t len, Sensor *sensor)
{
  // Parse
  sml_file *file = sml_file_parse(buffer + 8, len - 16);

  delete[] downloadbuffer;
  downloadbuffer = new char[len];
  memcpy(downloadbuffer, buffer, len);
  downloadbuffersize = len;
  //DEBUG_SML_FILE(file);
  data = tojson(sensor, file);
  if (client.connected()) {
    client.beginPublish("esp8266-energy-meter/energy-meter/state",data.length(),false);
    client.print(data.c_str());
    client.endPublish();
  }
  //publisher.publish(sensor, file);
  // free the malloc'd memory
  sml_file_free(file);
}
void setup()
{
  Serial.begin(74880);  
  ota_setup();
  web_setup();
  mqtt_setup();
  sensor = new Sensor(&SENSOR_CONFIGS[0], process_message);
}
long timer10s = millis();
void loop()
{
  if(timer10s+10000<millis()){
    timer10s = millis();
    uint8_t rssi = WiFi.RSSI();
    char buff[100];
    sprintf(buff,"%i",rssi);
    client.beginPublish("esp8266-energy-meter/rssi/state",strlen(buff),false);
    client.print(buff);
    client.endPublish();
  }
  ota_loop();
  web_loop();
  sensor->loop();
  mqtt_loop();
  yield();
}