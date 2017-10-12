# Retro11 - Playing with PDP11

This project aims to upcycle the PDP11 being displayed in the foyer of the computer science
institute of the FU Berlin. For this we fuse it with a modern microcontroller and give it control
over the existing tape drive in the PDP11. With some extra LED and LCD sugar on top of it we are
able to build a simple gaming platform for custom developed games.
The goal for this project is to finish the hardware and implement one example game which
lets two players compete against each other in reaction time.

## Retro11

This is the C-based RIOT application controlling the existing PDP11 tape drive,
an additional LCD and LED's. Apart from giving control over the existing hardware,
this application ships with a simple game to compete in reaction time, which is
explained further down.
To let players compete against each other it is necessary to communicate between
the gaming platforms. For this, the application implements a COAP client and server.


## Reaction Game

For this game you at least two players, both of them standing in front of one of the
PDP11 in the FU Berlin. First of all they are prompted to enter there nickname, which
is send to the central server to later on save the reaction time and display a highscore.
The game starts the tape drive motors of both nodes and as soon as they stop, the players
a requested to react as fastas they can and press a button. The time between the motor
stopping and the player reacting is the reaction time which is then used to determine
the winner.
You can lookup the highscore in the web application of [team2](https://github.com/fu-inet-swp17/team2)
of this years software project.


## COAP Client/Server

The communication between platforms is realized using the RESTful Constrained Application Protocol, [CoAP](https://tools.ietf.org/html/rfc7252), sending messages utilizing UDP and IPv6. 
One microcontroller is running a client applictation which is controling the game. The information needed is provided by a server application running on each microcontroller. 
Depending on the game state, the client polls the ressource containing the required information of both servers with a get-method until the servers provide the information to proceed to the next stage. In the first state the client requests the nicknames of the player, then request for the actual reaction game to start in the second state, goes on to request the reaction time in the third state and requests each machine to display if they won or lost in the last state.
Additionally both servers have a ressource which provides highscore information wrapped in [SenML](https://tools.ietf.org/html/draft-ietf-core-senml-08) format when requested by team2.


## Implementational particularities

Retro11 is a C-based RIOT application running on the SAMR21 XPro microcontroller.
First we started by creating the hardware platform and writing driver for each peripheral since they were not yet supported in RIOT itself. Each driver is implemented according to the example drivers RIOT is offering. The driver implementation was straightforward for all peripherals except the WS2811.
The WS2811 offers one digital in to receive color information and one digital out to send color information to the next WS2811 in line. To conclude, each WS2811 controls one LED and up to 1024 WS2811 can be chained together to control whole arrays of LEDS.
Since the WS2811 only offers a one wire protocol and no extra clocking it requires hard timings to differentiate between 0 and 1 in its protocol. For our project we bought two different kinds of WS2811 LEDs sets. The first is an already assembled LED striped which worked out of the box with our driver and the second is a set of seperate chips and LEDs to assemble by yourself. The second kit refused to work with our driver and after multiple days of debugging, it turned out that the controller requires a common annode, but the LEDs we received offered a common cathode. Unfortunately the shop we bought the LEDs at was unable to provide the correct LEDs in time to completly finish the project.

Concerning the application design itself, we first initialize all our drivers and then start four threads to control the various aspects of our application.
The first thread is the `MotorController` which listens for messages to start or stop the motor. Since we require the motors to run for a specific time, one can also pass a timeout after which the motor should be stopped.
The second thread is the `CoapServer` which handles communcation between each node and also publishes the final results to the RasperryPi of team2 to assemble a webviewable highscore.
The third thread is the game itself, which takes control over all peripherals and communicates with the `CoapServer` over a shared state variable. The game controls user input and output and provides the arcade like game experience.
The fourth and last thread is for debugging purposes and simply runs a shell with custom commands for hardware diagnosis and other debugging operations. It is only accessible by plugging in a computer to the USB port of the SAMR21 Xpro.

## Hardware

|Name|Function|Count|
|----|--------|-----|
|WS2811|LED RGB Controller|60|
|S74LS151|Button input multiplexer|1|
|L298H|Double H-Bridge for motor control|1|
|DOGL128B LCD|LCD|1|
|SAMR21 XPro|Microcontroller|1|
