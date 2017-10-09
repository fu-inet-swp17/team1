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

## COAP Client/Server


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

## Hardware

|Name|Function|Count|
|----|--------|-----|
|WS2811|LED RGB Controller|60|
|S74LS151|Button input multiplexer|1|
|L298H|Double H-Bridge for motor control|1|
|DOGL128B LCD|LCD|1|
|SAM R21 XPro|Microcontroller|1|
