/**
 * @file       main.cpp
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 */

//#define BLYNK_DEBUG
#define BLYNK_PRINT stdout
// #define BLYNK_DEBUG 
#ifdef RASPBERRY
  #include <BlynkApiWiringPi.h>
#else
  #include <BlynkApiLinux.h>
#endif
#include <BlynkSocket.h>
#include <BlynkOptionsParser.h>
#include <iostream>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>

static BlynkTransportSocket _blynkTransport;
BlynkSocket Blynk(_blynkTransport);

float previousMillis = millis();
float currentMillis = 0;
float ElapsedTime = 0;
float ElapsedTimeM = 0;

const int motorButtonPin = 17;
const int connectionIndicatorPin = 18;
bool doorCooldown = false;
bool locked = true;
bool initComplete = false;

char mice_data[3];
int mouse_x = 0;
int mouse_y = 0;

BLYNK_CONNECTED() {
  Blynk.syncAll();
}

BLYNK_WRITE(V0) {
  int val = param.asInt();

  if (val == 1) {
    // printf("locked = true\n");
    locked = true;    
  }
  if  (val == 0) {
    // printf("locked = false\n");
    locked = false;    
  }
}

BLYNK_WRITE(V1) {
  int val = param.asInt();
  
  if (val == 1 && doorCooldown == false && locked == false) 
  {
    // THIS TRIGGER THE MOTOR
    digitalWrite(motorButtonPin, HIGH);
    previousMillis = millis();
    doorCooldown = true;

    Blynk.virtualWrite(V0, 1);
    locked = true;
  }
  else 
  {
    // THIS TRIGGER THE MOTOR
    digitalWrite(motorButtonPin, LOW);
  }
}

int mice_position(){

   int fd = open("/dev/input/mice", O_RDWR |O_NONBLOCK);
   if(fd == -1){
           printf("ERROR Opening %s\n", "/dev/input/mice");
       return -1;
   }
        
   printf("\n");
   read(fd, mice_data, sizeof(mice_data));
    
   close(fd);

   return 0; 
}

int update_mouse(){
  mice_position();
  mouse_x = mice_data[1];
  mouse_y = mice_data[2];
  return 0;
}

void setup() {
  pinMode(motorButtonPin, OUTPUT);
  pullUpDnControl(motorButtonPin, PUD_DOWN);
  pinMode(connectionIndicatorPin, OUTPUT);
  pullUpDnControl(connectionIndicatorPin, PUD_DOWN);

  digitalWrite(connectionIndicatorPin, 127);

  Blynk.virtualWrite(V0, 1);  
}

void loop() {
  Blynk.run();

  // update_mouse();
  // printf("%i\t%i", mouse_x, mouse_y);

  currentMillis = millis();
  float ElapsedTime = ((currentMillis - previousMillis)/1000);

  if(doorCooldown == true)
  {
    if(ElapsedTime > 15)
    {
      doorCooldown = false;
      digitalWrite(connectionIndicatorPin, HIGH);      
    }
    else if((int)ElapsedTime % 2 == 0) {
      digitalWrite(connectionIndicatorPin, LOW);
    }
    else {
      digitalWrite(connectionIndicatorPin, HIGH);
    }
  }
}

int main(int argc, char* argv[]) {
    const char *auth, *serv;
    uint16_t port;
    parse_options(argc, argv, auth, serv, port);

    Blynk.begin(auth, serv, port);

    setup();
    while(true) {
        loop();
    }
    digitalWrite(connectionIndicatorPin, LOW);
    
    return 0;
}

