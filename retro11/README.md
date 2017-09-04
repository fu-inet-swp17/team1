# Retro11

## Compiling

Please make sure you have the complete RIOT toolchain installed on your system.
Don't forget to set `MACHINE` in `include/retro11_conf.h`. The first `MACHINE` starts with `M0` and increases by 1 with every new machine you add to the network.

  # Checkout RIOT
  $ git submodule init
  $ git submodule update

  # Just compile
  $ BOARD=samr21-xpro make all

  # Compile, Flash and open term
  $ BOARD=samr21-xpro make all term flash

## License

[GPL3](https://www.gnu.org/licenses/gpl-3.0.de.html)
