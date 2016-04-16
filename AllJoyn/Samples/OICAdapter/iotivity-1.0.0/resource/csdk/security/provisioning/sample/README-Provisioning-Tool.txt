LAST UPDATED 7/16/2015

To execute Provisioning Tool sample:

1) Build IoTivity with security enabled:
	$ cd <iotivity-base>
	$ scons resource SECURED=1

2) Verify Provisioning Tool functionality using secure sample apps:

    On Resource Server Device which needs to be 'provisioned':
	$ cd <iotivity-base>/out/<...>/release/resource/csdk/stack/samples/linux/secure
	$ export LD_LIBRARY_PATH=<iotivity-base>/out/<...>/release
	$ cp ../../../../security/provisioning/sample/oic_svr_db_unowned_server.json oic_svr_db_server.json
	$ ./ocserverbasicops


    On Provisioning Tool Device:
	$ cd <iotivity-base>/out/<...>/release/resource/csdk/security/provisioning/sample
	$ ./provisioningclient

    Follow the prompts on Provisioning Tool device and provisioning should be completed
    successfully. You should see a message 'Provisioning Success~!!'.
