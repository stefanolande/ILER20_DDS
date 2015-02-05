
#include <EEPROM.h>

void setup()
{  
  String str = "14185000";
  for (int i = 0; i < 8; i++){
    EEPROM.write(i, String(str.charAt(i)).toInt());
  }
}

void loop()
{

}


