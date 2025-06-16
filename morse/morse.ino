//FOLOSIND PORCESOR (OLD BOOTLOADER)
//permisiune temporara pt port sudo chmod a+rw /dev/ttyUSB0
#include <ctype.h>
#define PIN_BUZZER 9
#define LED_ROSU 6
#define LED_GALBEN 4
#define LED_ALBASTRU 2
#define TIME 250 //ms
#define FRECV 220

bool playing = false;
String buffer_input = "";
int switch_led = 1; // pt comanda sos

const char* tabelMorseCod[36] =
{
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.",
  "....", "..", ".---", "-.-", ".-..", "--", "-.",
  "---", ".--.", "--.-", ".-.", "...", "-", "..-",
  "...-", ".--", "-..-", "-.--", "--..", // A-Z
  "-----", ".----", "..---", "...--", "....-", ".....",
  "-....", "--...", "---..", "----."  // 0-9
};

void setup() {
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(LED_ROSU, OUTPUT);
  pinMode(LED_GALBEN, OUTPUT);
  pinMode(LED_ALBASTRU, OUTPUT);
  Serial.begin(9600);
  Serial.println("Ready :D");
}

void signal(int timp, int sos_check)
{
  if(sos_check == 0)
  {
    digitalWrite(LED_GALBEN, HIGH);
    tone(PIN_BUZZER, FRECV);
    delay(TIME * timp);
    digitalWrite(LED_GALBEN, LOW);
    noTone(PIN_BUZZER);
  }
  else
  {
    if(switch_led == 1)
      digitalWrite(LED_ALBASTRU, HIGH);
    else
      digitalWrite(LED_ROSU, HIGH);
    tone(PIN_BUZZER, FRECV);
    delay(TIME * timp);
    digitalWrite(LED_ROSU, LOW);
    digitalWrite(LED_ALBASTRU, LOW);
    noTone(PIN_BUZZER);
    
    switch_led *= -1;
  }

  
}

int charToMorse(char c)
{
  c = toupper(c);
  if(c >= 'A' && c <= 'Z') return c - 'A';
  if(c >= '0' && c <= '9') return c - '0' + 26;
  return -1; //eroare
}

void playMorseString(String morse, int sos_check)
{
  for(int i = 0; i < morse.length(); i++)
  {
    char c = morse[i]; // morse.charAt(i);
    if(c == '.')
      signal(1, sos_check);
    else if(c == '-')
      signal(3, sos_check);
    else if(c == ' ')
      delay(TIME * 3);
    delay(TIME); // spatiu intre semne
  }
}

void playInput(String msg)
{
  for(int i = 0; i < msg.length(); i++)
  {
    char c = msg[i];
    if(c == ' ')
    {
      delay(TIME * 7); //spatiu intre cuvinte
      continue;
    }

    int idx = charToMorse(c);
    if(idx != -1)
    {
      playMorseString(tabelMorseCod[idx], 0);
      delay(TIME * 3);
    }
  }
}

void comanda(String cmd)
{
  cmd.trim();
  cmd.toLowerCase();

  if(cmd == "sos")
  {
    playing = true;
    while(playing)
    {
      Serial.println("SOS Signal");
      playMorseString("... --- ...", 1);
      delay(TIME * 7);
      if(Serial.available())
      {
        String stopcmd = Serial.readStringUntil('\n');
        stopcmd.trim();
        if(stopcmd == "stop")
          playing = false;
      }

      digitalWrite(LED_ALBASTRU, LOW);
      digitalWrite(LED_ROSU, LOW);
    }
  } else if(cmd.startsWith("text:"))
  {
    String msg = cmd.substring(5);
    playInput(msg);
    Serial.println("Codare finalizata");
  } else if(cmd.startsWith("cod:"))
  {
    String raw = cmd.substring(4);
    playMorseString(raw, 0);
    Serial.println("Transmitere raw finalizata");
  } else
  {
    Serial.println("Comanda nu exista!.");
  }
}

void loop() {
  if(Serial.available())
  {
    char c = Serial.read();
    if(c == '\n')
    {
      comanda(buffer_input);
      buffer_input = "";
    }
    else
    {
      buffer_input += c;
    }
  }
}
