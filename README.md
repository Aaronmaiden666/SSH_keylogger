# SSH_keylogger for GNU/Linux


### Summary
This is featurable SSH and keyboard keylogger for Linux-systems. **Needs root-privileges**.

### Motivation
My main idea is to create programm product, which will be able to detect and interact with SSH-connections to/from your Linux PC/server. This interaction is to detect new ssh-connections in runtime or scan already opened connections at the program start moment and then hidely and safely attach to their processes and log all commands, sending in these connections. The main use case i see administrators of Linux systems, who need to detect unlegal activity by ssh to/from their systems.


I will be glad to discuss new ideas, suggestions, criticism, best practices and other help in the development of architecture, the choice of libraries and new features. I would like to combine in one project all means of keylogging.

### Already implemented features:
 - Automatically scan (every 3 seconds) all SSH-connections (incoming and outgoing) and logging their entire commands into ```/tmp/.keylog/**.log```.
 - Detect new connections (incoming or outgoing) by SSH and start logging them using **strace** for both.
 - Detect new connections (incoming or outgoing) by SSH and start logging them using **ptrace** for both.
 - Detect closing the connections and stop logging them.
 - As additional feature is implemented **keyboard logging** by scanning system files: ```/etc/input/event*```.

I want to implement the way to send log information to administrator of local network or to owner of the PC by email, by some specific protocol over tcp or something else. I am still not sure. Perhaps it will have a way to indicate remote connection to PC by SSH. We will see...

I use C++14/17 for coding but I am still studying. I do it in out of work time and the speed is not such rapid as I dream. You are welcome for contributing together.

### How to compile and run

```bash
cd SSH_keylloger/
mkdir build && cd build
cmake .. && cmake --build .
sudo ./keylogger --help
```

You should start keylloger with ```--help``` flag at the first time to know about available options:

```
Options:
  --help                      produce help message
  -k [ --kboard ] arg (=ON)   [ON/OFF] run logging all detected 
                              keyboards(default option)
  -m [ --mode ] arg (=strace) [strace/ptrace] run ssh keylogger via STRACE or 
                              PRACE(ptrace in developing now) mechanism
```

#### Requirements:
 - CMake
 - GCC
 - Boost (regex, filesystem)
 - Administrative privilege (Ha-Ha)
 
 
**NOTE:** This code uses ```strace``` and ```ptrace``` for logging depends on arguments you use to start program. If you start with ```strace```-argument, it is not enough hide. You can see ```strace``` command in ```ps```. But when you start with ```ptrace```, it is actually hide. Choose it by youself ;)
