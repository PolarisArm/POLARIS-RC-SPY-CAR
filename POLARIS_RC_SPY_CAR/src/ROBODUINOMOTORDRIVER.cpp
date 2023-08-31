#include <ROBODUINOMOTORDRIVER.h>


void ROBODUINOMOTORDRIVER::begin(){
    pinMode(A3,OUTPUT);
    Wire.begin();
    digitalWrite(A3,HIGH);
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.endTransmission();
    delayMicroseconds(10000);

}

void ROBODUINOMOTORDRIVER::run(char motor_name,char direction,int speed){
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(motor_name);
    Wire.write(direction);
    Wire.write(speed);
    Wire.write(0x00);
    Wire.endTransmission();
    delay(1);
}

void ROBODUINOMOTORDRIVER::stop(){
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.endTransmission();
    delay(1);
}