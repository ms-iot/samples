LAST UPDATED 5/27/2015

To build the IoTivity stack with the security features enabled:

1) Build IoTivity with security enabled:
	$ cd <iotivity-base>
	$ scons resource SECURED=1

2) Verify functionality using secure sample apps:
	$ cd <iotivity-base>/out/<...>/release/resource/csdk/stack/samples/linux/secure
	$ export LD_LIBRARY_PATH=<iotivity-base>/out/<...>/release
	$ ./ocserverbasicops &
	$ ./occlientbasicops -t 1
        Message "INFO: occlientbasicops: Secure -- YES" indicates success!
	$ ./occlientbasicops -t 2
        Completion of 'GET' and 'PUT' query successfully indicates success!

