#ifndef __ROBODUINOMOTORDRIVER_H__
#define __ROBODUINOMOTORDRIVER_H__

#include <Arduino.h>
#include <Wire.h>

// Define Slave I2C Address
#define SLAVE_ADDR 0x3D

// Define Slave answer size
#define MOTOR1   0x01
#define MOTOR2   0x02
#define MOTOR3   0x03
#define MOTOR4   0x04

#define FORWARD   0x09
#define BACKWARD  0x10
#define NOTHING   0xAA


class ROBODUINOMOTORDRIVER {
   
    public:
        void begin(); // Mainly it begins Wire library
        //Runs the motor forward or backward
        void run(char motor_name,char direction,int speed);
        //Stops the motor
        void stop();
        

};


#endif