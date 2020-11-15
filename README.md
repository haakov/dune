TTK22 Project: Elixir Web Server
======================================

Install instructions (Ubuntu):

Install Elixir: 
- `sudo apt install elixir`

Compile: 
- `mkdir dune && cd dune`
- `git clone https://github.com/haakov/dune source`
- `cd source/www/elixir_srv && mix local.hex`
- `mix deps.get`
- `mix compile`
- `cd ../../../build && cmake ../source`
- `make -j2`

Run DUNE: 
- `./dune -p Simulation -c lauv-xplore-2 -w ../source/www`

DUNE: Unified Navigation Environment
======================================

DUNE: Unified Navigation Environment is a runtime environment for unmanned systems on-board software. It is used to write generic embedded software at the heart of the system, e.g. code or control, navigation, communication, sensor and actuator access, etc. It provides an operating-system and architecture independent platform abstraction layer, written in C++, enhancing portability among different CPU architectures and operating systems.
