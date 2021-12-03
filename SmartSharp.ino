#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>

#define startupflash false
#define ssid "SmartSharp"
#define password  "getsmartstaysharp"
#define pushButton 0
#define motor 4
#define ledPin 5
#define NUM_LEDS 2
#define serialDebug false
#define disableMotor false

ESP8266WebServer server(80);
CRGB leds[NUM_LEDS];

int enabled = 0;
int ledMode = 2;
int ledSubMode = 0;
int ledTick = 0;
int lockedMode = 2;
int unlockedMode = 2;
int resetleds = 0;
int updateleds = 1;
int latch = 0;
uint8_t led_r, led_g, led_b;

void setup() {
 
  delay( 3000 ); // power-up safety delay
  //FastLED.addLeds<NEOPIXEL, ledPin>(leds, NUM_LEDS);
  FastLED.addLeds<WS2811, ledPin, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

#if serialDebug
  Serial.begin(115200);
#endif

#if startupflash
  leds[0] = CRGB::Red;
  FastLED.show();
#endif

  WiFi.softAP(ssid, password);
  server.on("/", handleRoot);
  server.on("/lock", lock);
  server.on("/unlock", unlock);
  server.on("/ledrgb", ledrgb);
  server.on("/ledmode", ledmode);
  server.on("/runmotor", runmotor);
  server.on("/lockedmode", lockedmode);
  server.on("/unlockedmode", unlockedmode);
  server.begin();

#if startupflash
  leds[0] = CRGB::Black;
  FastLED.show();
  delay(100);
  leds[1] = CRGB::Green;
  FastLED.show();
#endif
  
  
  pinMode(pushButton, OUTPUT);    //UGASNEŠ PULLUP, esp se mal drugač obnaša od atmela
  digitalWrite(pushButton, LOW);
  //pinMode(ledPin, OUTPUT);
  pinMode(motor, OUTPUT);

#if startupflash
  leds[0] = CRGB::Black;
  FastLED.show();
  delay(100);
  leds[0] = CRGB::Blue;
  leds[1] = CRGB::Blue;
  FastLED.show();
  delay(200);
  leds[0] = CRGB::White;
  leds[1] = CRGB::White;
  FastLED.show();
  delay(100);
  leds[0] = CRGB::Black;
  leds[1] = CRGB::Black;
  FastLED.show();
#endif

}

void loop(){
 
  int buttonState = digitalRead(pushButton);
  if(buttonState){
    if(enabled){
      #if not disableMotor
        digitalWrite(motor, 1);
      #endif
      switch(unlockedMode){
        case 0:
          break;
        case 1:
          if(updateleds){
            leds[0] = CRGB::Green;
            leds[1] = CRGB::Green;
            FastLED.show();
            updateleds = 0;
          }else{
            resetleds = 1;  
          }
          break;
        case 2:
          if(!latch){
            ledSubMode = 0;
            latch = 1;  
          }
          switch(ledSubMode){
              case 0:
                led_r = 0;
                led_g = 0;
                led_b = 0;
                ledSubMode++;
                //no break
              case 1:
                led_g++;
                if(ledTick>240){
                  ledSubMode++;
                  ledTick = 0;
                }
                leds[0] = CRGB(led_r, led_g, led_b);
                leds[1] = CRGB(led_r, led_g, led_b);
                break;
              case 2:
                led_r++;
                led_b++;
                if(ledTick>200){
                  ledSubMode++;
                  ledTick = 0;
                }
                leds[0] = CRGB(led_r, led_g, led_b);
                leds[1] = CRGB(led_r, led_g, led_b);
                break;
              case 3:
                if(ledTick>50){
                  leds[0] = CRGB::Black;
                  leds[1] = CRGB::Black;
                  ledTick = 0;
                  ledSubMode++;
                }
                break;
              case 4:
                if(ledTick>50){
                  leds[0] = CRGB(led_r, led_g, led_b);
                  leds[1] = CRGB(led_r, led_g, led_b);
                  led_g--;
                  led_b--;
                  ledTick = 0;
                  ledSubMode--;
                }
                break;
                
          }
          FastLED.show();
          break;
      }
    }else{
      switch(lockedMode){
        case 0:
          break;
        case 1:
          if(updateleds){
            leds[0] = CRGB::Red;
            leds[1] = CRGB::Red;
            FastLED.show();
            updateleds = 0;
          }else{
            resetleds = 1;  
          }
          break;
        case 2:
          if(!latch){
            latch = 1;
            ledSubMode = 0;  
          }
          switch(ledSubMode){
            case 0:
              if(ledTick>200){
                leds[0] = CRGB::Red;
                leds[1] = CRGB::Red;
                updateleds = 1;
                ledTick = 0;
                ledSubMode++;
              }
              break;
            case 1:
              if(ledTick>200){
                leds[0] = CRGB::Black;
                leds[1] = CRGB::Black;
                updateleds = 1;
                ledTick = 0;
                ledSubMode--;
              }
              break;
          }
          if(updateleds){
            FastLED.show();
            updateleds = 0;
          }
          break;

      }
    }  
  }else{
    digitalWrite(motor, 0);
    if(resetleds){
      leds[0] = CRGB::Black;
      leds[1] = CRGB::Black;
      FastLED.show();
      resetleds = 0;
    }
    updateleds = 1;
    latch = 0;
    switch(ledMode){
      case 0:
        break;
      case 1:
        if(ledTick>750){
          leds[0] = CRGB::Blue;
          leds[1] = CRGB::Red;
          ledTick = 0;
          FastLED.show();
        }else if(ledTick>500){
          leds[0] = CRGB::Red;
          leds[1] = CRGB::Blue;
          FastLED.show();
        }else{
        }
        break;
      case 2:
        if(ledTick>5000){
          leds[0] = CRGB::Blue;
          leds[1] = CRGB::Blue;
          ledTick = 0;
          FastLED.show();
        }else if(ledTick>100){
          leds[0] = CRGB::Black;
          leds[1] = CRGB::Black;
          FastLED.show();
        }
        break;
       case 3:
        if(ledTick>240){
            ledSubMode++;
            ledTick = 0;
        }else{
          switch(ledSubMode){
            case 7:
              ledSubMode = 0;
              //no, no break here either
            case 0:
              led_r = 240;
              led_g = 0;
              led_b = 0;
              ledSubMode++;
              //no, no break!
            case 1:
              led_g++;
              break;
            case 2:
              led_r--;
              break;
            case 3:
              led_b++;
              break;
            case 4:
              led_g--;
              break;
            case 5:
              led_r++;
              break;
            case 6:
              led_b--;
              break;
           
          }

          #if serialDebug
            Serial.print("R: ");
            Serial.print(led_r);
            Serial.print(" G: ");
            Serial.print(led_g);
            Serial.print(" B: ");
            Serial.print(led_b);
            Serial.print(" SUBMODE: ");
            Serial.print(ledSubMode);
            Serial.println();
            delay(1);      
          #endif
          
          leds[0] = CRGB(led_r, led_g, led_b);
          leds[1] = CRGB(led_r, led_g, led_b);
          FastLED.show();
  
        }
        break;
    }
  }
  //digitalWrite(4, buttonState && enabled);




  ledTick++;
  server.handleClient();
  delay(1);
}


void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}

void lock(){
  enabled = 0;  
  server.send(200, "text/html", "<h1>OK. Locked.</h1>");

}

void unlock(){
  enabled = 1;
  server.send(200, "text/html", "<h1>OK. Unlocked.</h1>");
}

void ledrgb(){
  uint8_t r = server.arg("r").toInt();
  uint8_t g = server.arg("g").toInt();
  uint8_t b = server.arg("b").toInt();
  //int led = server.arg("led").toInt();

  if(server.hasArg("led")){
    leds[server.arg("led").toInt()%NUM_LEDS] = CRGB(r, g, b);
  }else{
    CRGB t = CRGB(r, g, b);
    for(int i=0; i<NUM_LEDS; i++){
      leds[i] = t;
    }
  }
  FastLED.show();

  char response[64];
  snprintf(response, 64, "r: %d g: %d b: %d hasArg(led): %d led: %d", r, g, b, server.hasArg("led"), server.arg("led").toInt());
  server.send(200, "text/html", response);
}

void ledmode(){
  ledMode = server.arg("mode").toInt();
  server.send(200, "text/html", "<h1>OK</h1>");
}

void lockedmode(){
  lockedMode = server.arg("mode").toInt();
  server.send(200, "text/html", "<h1>OK</h1>");
}

void unlockedmode(){
  unlockedMode = server.arg("mode").toInt();
  server.send(200, "text/html", "<h1>OK</h1>");
}

void runmotor(){
  int time = server.arg("milis").toInt();
  time += 1000*(server.arg("seconds").toInt());
  digitalWrite(motor, 1);
  server.send(200, "text/html", "<h1>OK</h1>");
  delay(time);
  digitalWrite(motor, 0);
}
