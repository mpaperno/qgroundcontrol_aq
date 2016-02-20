QGroundControl for AutoQuad Linux User Notes
============================================

To "install" QGC for AQ, simply unpack the distribution archive into a folder of your choice.  
A few more steps may be necessary, depending on your current Linux setup. These are described below.

Setting Up Text-to-Speech (TTS)
-------------------------------

If QGC was build with TTS enabled (default), you will need to have the Festival TTS system installed. 
The example below is for a typical Ubuntu/Debian/Mint/etc system. You need to perform the following steps as root user.

    sudo apt-get install festival festvox-en1

This will install the TTS system and a good British English male voice.  For a few other options you can enter:

    sudo apt-cache search festvox-*

And then `sudo apt-get install festvox-[voice-name]` to install it.  `festvox-us1` is a good American English female voice 
and `festvox-us2` is an AE male voice. I found the `en1` voice most understandable and pleasant though.

For more information and instructions for Festival setup do a Google search for your specific Linux distribution 
(eg. "linux festival voices setup ubuntu").  You can specify default voice and speed (and other options)
in the `/etc/festival.scm` file.


UDEV Rules
----------

On many Linux installations, access to USB devices is limited to the root user by default.  This is controlled by the "udev" service.  
If you want to be able to run QGC as a non-root user, you will very likely need to install some udev rules.  
There are three rule files included with QGC, found in the `files/etc` subfolder. These should work for common serial adapters 
from FTDI and Silicone Labs (CP2102), as well as the USB devices provided by the AutoQuad M4 board (STM32-based devices from ST Micro).  

To install these rules copy them to your `/etc/udev/rules.d` directory. For example, run the following command, 
replacing `[path_to_QGC]` with the actual QGC installation folder:

    sudo cp [path_to_QGC]/files/etc/*.rules /etc/udev/rules.d/

If the device was already plugged in, be sure to unplug and then replace it after updating the udev rules and before starting QGC.



=====================================
