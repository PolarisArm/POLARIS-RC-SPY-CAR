#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <SPI.h>
#include "RF24.h"
#include <ROBODUINOMOTORDRIVER.h>
#include <SoftwareSerial.h>

const uint64_t address = 0XE8E8F0F0E1LL;

#define CE_PIN  9
#define CSN_PIN 10
#define RF_STATUS_LED 3

ROBODUINOMOTORDRIVER motor;
SoftwareSerial espSerial(7,8);
RF24 radio(CE_PIN, CSN_PIN);

struct recivedData{
    byte ch0;
    byte ch1;
    byte ch2;
    byte ch3;
    byte ch4;
    byte ch5;
    byte ch6;
    byte ch7;
};

struct recivedData data;


unsigned long lastRecivedTime = 0;
int BAT             = A0;
float BAT_VALUE     = 0;
int R1              = 10000;
int R2              = 6800;
int SPEED           = 0;
int MIN_SPEED       = 20;

int FB,LR,CAM_LR,CAM_UD,POT1,POT2 = 0;
int motor_speed = 0;

Servo espServo_UD;
Servo espServo_LR;



void print_reciver_channel();
void resetData();
void forward(int motor_speed);
void backward(int motor_speed);
void left(int motor_speed);
void right(int motor_speed);
void forward_right(int motor_speed);
void forward_left(int motor_speed);
void backward_right(int motor_speed);
void backward_left(int motor_speed);


void setup() 
{
  pinMode(RF_STATUS_LED,OUTPUT);
  motor.begin();  
  Serial.begin(9600);
  espSerial.begin(115200);
  resetData();

  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  Serial.println(F("radio hardware is  responding!!"));

  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN);
  radio.openReadingPipe(1,address);
  radio.startListening();
  if(radio.available()){
    Serial.println(F("radio available!!"));
  }  
  espServo_LR.attach(6);
  espServo_UD.attach(5);
}

void loop() 
{
  BAT_VALUE = (((analogRead(BAT) * 5.0)/1023)*(R1+R2))/R2;
  while(radio.available())
  {
        radio.read(&data,sizeof(recivedData));
        digitalWrite(RF_STATUS_LED,HIGH);
        lastRecivedTime = millis();

  }
  
  unsigned long now = millis();

  if(now - lastRecivedTime > 150){resetData();} // IF NOT IT WILL JUST RUN OR FLY AWAY

 //print_reciver_channel();
  espSerial.print(BAT_VALUE);
  espSerial.print(',');
  espSerial.println('L');

  SPEED = data.ch5;
  
  espServo_LR.write(map(data.ch4,0,255,30,180));
  espServo_UD.write(map(data.ch2,0,255,20,150));


  if(data.ch0 > 245 && data.ch1 > 50 && data.ch1 < 245)
  {
    forward(SPEED);
  }
 else if(data.ch0 < 50 && data.ch1 > 50 && data.ch1 < 245)
  {    
    backward(SPEED);
  }
  
  if(data.ch1 > 225 && data.ch0 > 50 && data.ch0 < 245)
  {
    left(SPEED);        
  }
  else if(data.ch1 < 50  && data.ch0 > 50 && data.ch0 < 245)
  {
    right(SPEED);
  }
 
  if(data.ch0 > 245 && data.ch1 < 50)
  {
    forward_right(SPEED);
  }

  else if(data.ch0 > 245 && data.ch1 > 245)
  {
    forward_left(SPEED);
  }

  else if(data.ch0 < 50 && data.ch1 < 50)
  {
    backward_left(SPEED);

  }
 else if(data.ch0 < 50 && data.ch1 > 245)
  {
    backward_right(SPEED);
  }
  
  if(data.ch0 > 115 && data.ch0 < 125 && data.ch1 > 120 && data.ch1 <  128)
  {
    motor_speed = 0;
    motor.stop();
  }
}

void forward(int motor_speed)
{
    motor.run(MOTOR1,FORWARD,motor_speed);
    motor.run(MOTOR2,FORWARD,motor_speed);
    motor.run(MOTOR3,FORWARD,motor_speed);
    motor.run(MOTOR4,FORWARD,motor_speed);
}

void backward(int motor_speed)
{
    motor.run(MOTOR1,BACKWARD,motor_speed);
    motor.run(MOTOR2,BACKWARD,motor_speed);
    motor.run(MOTOR3,BACKWARD,motor_speed);
    motor.run(MOTOR4,BACKWARD,motor_speed);
}

void left(int motor_speed)
{
    motor.run(MOTOR1,FORWARD,motor_speed);
    motor.run(MOTOR2,BACKWARD,motor_speed/1.4);
    motor.run(MOTOR3,FORWARD,motor_speed);
    motor.run(MOTOR4,BACKWARD,motor_speed/1.4); 
   
}

void right(int motor_speed)
{
    motor.run(MOTOR1,BACKWARD,motor_speed/1.4);
    motor.run(MOTOR2,FORWARD,motor_speed);
    motor.run(MOTOR3,BACKWARD,motor_speed/1.4);
    motor.run(MOTOR4,FORWARD,motor_speed);
}

 void forward_right(int motor_speed)
{

    motor.run(MOTOR1,FORWARD,motor_speed/2.2);
    motor.run(MOTOR2,FORWARD,motor_speed);
    motor.run(MOTOR3,FORWARD,motor_speed/2.2);
    motor.run(MOTOR4,FORWARD,motor_speed);

}

void forward_left(int motor_speed)
{
    motor.run(MOTOR1,FORWARD,motor_speed);
    motor.run(MOTOR2,FORWARD,(motor_speed/2.2));
    motor.run(MOTOR3,FORWARD,motor_speed);
    motor.run(MOTOR4,FORWARD,motor_speed/2.2);
}

void backward_left(int motor_speed)
{
    motor.run(MOTOR1,BACKWARD,motor_speed/2.2);
    motor.run(MOTOR2,BACKWARD,motor_speed);
    motor.run(MOTOR3,BACKWARD,motor_speed/2.2);
    motor.run(MOTOR4,BACKWARD,motor_speed);
}

void backward_right(int motor_speed)
{
    motor.run(MOTOR1,BACKWARD,motor_speed);
    motor.run(MOTOR2,BACKWARD,motor_speed/2.2);
    motor.run(MOTOR3,BACKWARD,motor_speed);
    motor.run(MOTOR4,BACKWARD,motor_speed/2.2);
} 

void resetData()
{
  data.ch0 = 127;
  data.ch1 = 127;
  data.ch2 = 127;
  data.ch3 = 124;
  data.ch4 = 57;
  data.ch5 = 0;
  data.ch6 = 0;
  data.ch7 = 0;
  digitalWrite(RF_STATUS_LED,LOW);
}


void print_reciver_channel()
{
  Serial.print("FR: ");
  Serial.print(data.ch0);
  Serial.print("\t");
  Serial.print("LR: ");
  Serial.print(data.ch1);
  Serial.print("\t");
  Serial.print("CAM_LR: ");
  Serial.print(data.ch2);
  Serial.print("\t");
  Serial.print("CAM_UD: ");
  Serial.print(data.ch3);
  Serial.print("\t");
  Serial.print("POT1: ");
  Serial.print(data.ch4);
  Serial.print("\t");
  Serial.print("POT2: ");
  Serial.print(data.ch5);
  Serial.print("\t");
  Serial.print("BAT: ");
  Serial.print(BAT_VALUE);
  Serial.print("\t");
  Serial.println();
}

