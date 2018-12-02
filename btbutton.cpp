
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libudev.h>
#include <poll.h>


//poll array indexes for each input:
#define UDEV 0
#define BT   1

void err(const char *msg)
{
	printf("%s\n",msg);
	exit(1);
}

int main(int argc, char * argv[])
{
	//udev variables:
	struct udev *udev;
	struct udev_device *dev;
	struct udev_monitor *mon;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;

	//poll variables
	struct pollfd fds[2];
	int numfds = 2;
	int ret;

	//get udev environment:
	udev = udev_new();
	if (!udev) err("error: Can't create udev\n");
	
	//get the currently connected device:
	enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "hidraw");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);
	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *path;
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);
		if (dev) {
			printf("Device Node Path already exists: %s\n", udev_device_get_devnode(dev));
			fds[BT].fd = open(udev_device_get_devnode(dev), O_RDONLY | O_NONBLOCK);
		}
		else fds[BT].fd = -1;
	}
	udev_enumerate_unref(enumerate);

	//set up to monitor udev hidraw events:
	mon = udev_monitor_new_from_netlink(udev, "udev");
	udev_monitor_filter_add_match_subsystem_devtype(mon, "hidraw", NULL);
	udev_monitor_enable_receiving(mon);
	fds[UDEV].fd = udev_monitor_get_fd(mon);

	//set up polling:
	fds[UDEV].events = POLLIN;
	fds[UDEV].revents = 0;
	fds[BT].events = POLLIN;
	fds[BT].revents = 0;

	while(true) {
		ret = poll(fds, numfds, -1);
		if (ret > 0) {
			if (fds[UDEV].revents & POLLIN) { //udev event
				dev = udev_monitor_receive_device(mon);
				if (dev) {
					printf("udev: action=%s\n",udev_device_get_action(dev));
					if (strcmp(udev_device_get_action(dev), "add") == 0) {
						printf("bt: opening %s...\n",udev_device_get_devnode(dev));
						fds[BT].fd = open(udev_device_get_devnode(dev), O_RDONLY | O_NONBLOCK);
					}
					if (strcmp(udev_device_get_action(dev), "remove") == 0) {
						printf("bt: closing %s...\n",udev_device_get_devnode(dev));
						close(fds[BT].fd);
						fds[BT].fd = -1;
					}
				}
			}
			if (!(fds[BT].revents & POLLNVAL)) { //data available from button
				if (fds[BT].revents & POLLIN) {
					char buf[256];
					int r = read(fds[BT].fd, buf, 255);
					if (r > 0) {
						//this logic uses one button for 'on', the other for 'off'.
						//replace the printfs with statements to control whatever you want:
						//if (buf[0] == 3 && buf[2] == 0xA )
						//	printf("on\n");
						//else if (buf[0] ==1 && buf[3] == 0x28)
						//	printf("off\n");
						//else {
							//use this loop to print out the codes sent by each button.
							//Mine sent two codes for each press, one for button-down, the other for button-up.
							for (int i=0; i<r; i++) printf("%X ",buf[i]); printf("\n");
						//}
						buf[0] = '\0';
					}
				}
			}
		}
	}
	return(0);
}
