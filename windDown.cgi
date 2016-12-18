#!/usr/bin/python
from Adafruit_MotorHAT import Adafruit_MotorHAT, Adafruit_DCMotor, Adafruit_StepperMotor
 
import time
import atexit

# create a default object, no changes to I2C address or frequency
mh = Adafruit_MotorHAT(addr = 0x60)

# recommended for auto-disabling motors on shutdown!
def turnOffMotors():
        mh.getMotor(1).run(Adafruit_MotorHAT.RELEASE)
        mh.getMotor(2).run(Adafruit_MotorHAT.RELEASE)
        mh.getMotor(3).run(Adafruit_MotorHAT.RELEASE)
        mh.getMotor(4).run(Adafruit_MotorHAT.RELEASE)
 
atexit.register(turnOffMotors)

myStepper2 = mh.getStepper(200, 2)       # 200 steps/rev, motor port #2

myStepper2.setSpeed(200)                  # 30 RPM


end = time.time() + 45
while time.time() < end:
	myStepper2.step(100, Adafruit_MotorHAT.FORWARD, Adafruit_MotorHAT.DOUBLE)

