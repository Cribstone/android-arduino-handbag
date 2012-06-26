#include <string.h>
#include <Print.h>

void sendField(Print& strm, const char *fieldData, boolean isFinalField = false) {
  if ((fieldData == NULL)) {
    return;
  }

  if ((fieldData[0] == '[') || (strpbrk(fieldData, ";\n") != NULL)) {
    strm.write("[");
    strm.print(strlen(fieldData));
    strm.write("]");
  }

  strm.write(fieldData);

  // TODO: Handle this better?
  if (isFinalField) {
    strm.write("\n");
    delay(100); // TODO: Move elsewhere?
  } else {
    strm.write(";");
  }

}

unsigned int lastWidgetId = 0;

void setText(Print& strm, int widgetId, const char *labelText, byte fontSize = 0, byte alignment = 0) {

  sendField(strm, "widget");

  sendField(strm, "label");

  // TODO: Handle other types in sendField?
  strm.print(widgetId);
  strm.write(";");

  strm.print(fontSize);
  strm.write(";");

  strm.print(alignment);
  strm.write(";");

  sendField(strm, labelText, true);

}

unsigned int addLabel(Print& strm, const char *labelText, byte fontSize = 0, byte alignment = 0) {

  unsigned int widgetId = ++lastWidgetId;

  // Note: Takes advantage that widgets are auto-created if the Id is new.
  setText(strm, widgetId, labelText, fontSize, alignment);

  return widgetId;

}


#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(169, 254, 254, 169);

EthernetServer server(0xba9);

void setup() {

  Serial.begin(9600);

  Ethernet.begin(mac, ip);

  server.begin();

  Serial.println("start");
}

void loop() {
  EthernetClient client = server.available();

  if (client) {

    Serial.println("connected.");

    addLabel(client, "A Label!", 35, 1);

    addLabel(client, "Further words", 35, 1);

    client.write("widget;dialog;Hello!\n");
    delay(100);

    addLabel(client, "[\nhello there;\n]", 35, 1);

    addLabel(client, "MORE", 100, 1);

    unsigned int analogWidgetId = addLabel(client, "0", 35, 1);

    Serial.println("sent");

    while (client.connected()) {

      unsigned int value = analogRead(A0);

      // Hacky itoa for analog range:
      char result[5];
      byte offset = 0;
      if (value > 1000) {
        result[offset++] = '1';
      }
      if (value > 100) {
        result[offset++] = ((value / 100) % 10) + '0';
      }
      if (value > 10) {
        result[offset++] = ((value / 10) % 10) + '0';
      }
      result[offset++] = (value % 10) + '0';
      result[offset++] = '\0';

      setText(client, analogWidgetId, result);


      delay(1000);

      if (client.available() > 0) {
        client.read();
      }
      delay(10);
    }

    Serial.println("not connected");

    client.stop();

    Serial.println("stop.");

  }

}
