# PAM4-characterization-code

Please refrence Om Aggarwal - MetaROCK meeting notes for PAM4 in the 28nm drive to learn more about this code

metarockino code must be flashed onto Arduino UNO with the following steps:
upload sketch to arduino (this is needed only if you want to update communication with arduino, which is almost never needed):

/home/zhicai/arduino-1.8.19/arduino --upload --board arduino:sam:arduino_due_x --port /dev/ttyACM0 arduino/metarock/metarock.ino

testpam4 code needs to do these following steps in order to be run:

1. cd ldrd_28nm_testing/
2. cd build/
3. make
4. cd ..
5. cd run/
6. ../build/testPAM4 270 (integer is for the frequency you want to run the clock at, right now its configured for 10mhz)

steps 2 and 3 must be completed everytime you change code, cd.. to move back directories
