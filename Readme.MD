# btbutton - Use cheap bluetooth buttons to control stuff

I needed to make a button to turn on and off the Christmas lights.  I
already do that with X10 boxes, using a Raspberry Pi connected to
an old CP-230 X10 computer interface.  I thought, gee, getting one of
those cheap Bluetooth camera shutter buttons (I got a package of two for 
$8US on Amazon) should be easy, no? Well...

To make a long story short, after poking around with udev, dbus, and 
brute-force /dev shenanigans, as well as select(), poll(), epoll(), and 
various event libraries, I came up with the combination of libudev and 
poll() as the simplest and most reliable alternative.

If you have a Raspberry Pi 3, you'll already have Bluetooth hardware
built-in, otherwise you'll need a Bluetooth USB adapter, got my for $14US
at MicroCenter.  You'll need to pair and trust your button using
bluetoothctl:

$ sudo bluetoothctl
[bluetooth]# pair xx:xx:xx:xx:xx:xx
[bluetooth]# trust xx:xx:xx:xx:xx:xx
[bluetooth]# connect xx:xx:xx:xx:xx:xx

Where xx:xx:xx:xx:xx:xx is your button's device address.  You can find 
it like this:

[bluetooth]# scan on


If you don't alreadyhave libudev, you'll need to install it:

$ sudo apt-get install libudev-dev


Clone the repository to your Pi, then compile the program:

$ make

The Makefile is dirt-simple, it only compiles and cleans.

Run the program like this:

$ sudo ./btbutton

If your button is already paired and connected, btbutton will print
a message to that effect along with the device path, e.g., /dev/hidraw0.
If not, you can use bluetoothctl to issue the connect command, or just
make sure the button is turned on and press a button.  btbutton will
print 'add' and 'remove' events when the button connects and disconnects.

In its present state, btbutton will print the hex values comprising the
code sent by the device when a button is pressed.  My 'button' has two 
buttons, one labeled "Android", the other "IOS".  The commented-out logic
recognizes each down-press, and uses the IOS button for 'on', and the 
Android button for 'off'.  You need to do some coding here to replace 
the printfs with code that handles your intended object of control, 
GPIO, I2C, etc.  I have a homebrew X10 library's function calls  inserted 
here for my particular use.

I'm posting this code to capture the trades I went through to integrate 
a simple yet reliable solution.  Comments and critique are welcome to make 
it correct-er or more instructive.  If you want to make it into something 
else, fork it.


To-Dos:
- Command-line input of button serial number.  Right now, btbutton will only
work reliably with one paired HID keyboard device.
