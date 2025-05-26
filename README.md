# ultrasonic_sensor_gesture_detection
Arduino code for detecting basic hand gestures using HC-SR04 ultrasonic distance sensor array. 

![image](https://github.com/user-attachments/assets/857f8a34-5d09-4f38-84e8-94c185794c7c)

This code utilizes three HC-SR04 ultrasonic distance sensor modules placed linearly with a fixed distance between them to recognize finger/hand gestures. 

The defined gestures are:

- Tap : object approaches and recedes within 500 ms
- Tap and hold: object approaches, holds and recedes
- Swipe left: object approaches and moves left before receding
- Swipe right: object approaches and moves right before receding

The code measures distances from 3 sensors. If a nearby object is detected it's position is tracked. If object is tracked more than minimum time limit, based on the motion of the object one of the 4 gestures is reported.

To setup:

Connect 3 sensor modules to 5V supply and ground pins of the arduino module. 
Connect Trigger and echo pins of each module to digital pins 2 to 7 as shown in the figure.
Adjust their placement and distance. Update the "sensorSpacing" variable (in cm) in the code as needed. 
Connect Arduino to host device. Compile and flash the program.
Use serial monitor or a console tool with serial connection (e.g. Terra Term) to monitor outputs. Make sure baud rate is set correctly. 
