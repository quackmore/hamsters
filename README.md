# HAMSTERS

## Summary

This is a totally useless project, developed just to use and test [ESPBOT](https://github.com/quackmore/espbot_2.0) and [esp8266-library](https://github.com/quackmore/esp8266-library).
The APP will randomly move two digital output pin, one virtually connected to a wheel an the other one to a doorbell.
My hamsters had a lot of fun with it.

## Building the APP

(build commands are available as VS tasks)
- Configure the environment running the command "./gen_env.sh"
- Build user1.bin running the command "source ./env.sh && make -e APP=1 all"
- Build user2.bin running the command "source ./env.sh && make -e APP=2 all"

## Setup the device

(flash commands are available as VS tasks)
- Erase the flash running the command "source ./env.sh && make flash_erase"
- Init the flash (flashing SDK params) running the command "source ./env.sh && make flash_init"
- Flash the boot running the command "source ./env.sh && make flash_boot"

## License

The library comes with a [BEER-WARE] license.

Enjoy.
