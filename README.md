# Retro11 - Playing with PDP11

The idea of this project is to use the existing tape hardware of the PDP11, fuse
itwith a modern microcontroller, add some LCD/LED sugar on top of it and build
an open gaming platform. The hardware is naturally very limited but still
provides clonky feeling of 50 year old PDP11.

## Retro11

This is the C-based RIOT application controlling the existing PDP11 tape hardware,
an additional LCD and some LED's. For now it only enables the users to play a simple
reaction game against one another, but it can also be seen as an open platform
providing users with the opportunity to develop multiplayer games running on bare
metal hardware.

## CoapClient

This is a C-based RIOT application acting as the network controller for all
existing gaming stations. It is wirelessly connected to all Retro11 stations,
announces new games and decides who is the winning player.
