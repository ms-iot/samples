# Open three terminal windows in linux
# The first one use for start Light server
# Note: Here and below $PROJ_DIR is root directory of iotivity project (e.g /path/to/iotivity)
$ cd $PROJ_DIR/out/linux/x86_64/release/resource/csdk/security/provisioning/ck_manager/sample/Light_Resource
$ cp $PROJ_DIR/resource/csdk/security/provisioning/ck_manager/sample/Light_Resource/*.json ./
$ ./Light_server

# Second terminal window use for start Door server
$ cp $PROJ_DIR/resource/csdk/security/provisioning/ck_manager/sample/Door_Resource/*.json ./
$ ./Door_server

# And third terminal window use for start provisioning_client
# provisioning_client ask for input ACL data:
# 1. Controller device. Enter ID of the doorDeviceUUID00
# 2. Controlee device. Enter ID of the lightDeviceUUID0
# 3. Subject : doorDeviceUUID00
# 4. Num. of Resource : 1
# 5. [1]Resource : /a/light
# 6. permissions: CRUDN
# 7. Num. of Rowner : 1
# 8. [1]Rowner : lightDeviceUUID0
#
# After successfull sending acl provisioning_client will ask you for CRL data:
# 1. Enter number of revoced certificates(1..9): 1
# 2. Revoced certificate 0: Serial number (E. g.: 100): 3
# And then you should see message about successfull sending CRL
#
# Note: provisioning_client send ACL and CRL only to Light sever

$ cd $PROJ_DIR/out/linux/x86_64/release/resource/csdk/security/provisioning/ck_manager/sample
$ rm ckminfo.dat
$ cp $PROJ_DIR/resource/csdk/security/provisioning/ck_manager/sample/oic_svr_db_pt.json ./
$ ./provisioningclient
Provisioning device ID : doorDeviceUUID00
Provisioning Success~!!
Provisioning device ID : lightDeviceUUID0
Provisioning Success~!!
Sending credential is succeed~!!
******************************************************************************
-Set ACL policy for target device
******************************************************************************
-URN identifying the subject
ex) doorDeviceUUID00 (16 Numbers except to '-')
Subject : doorDeviceUUID00
Num. of Resource : 1
-URI of resource
ex) /a/light (Max_URI_Length: 64 Byte )
[1]Resource : /a/light
-Set the permission(C,R,U,D,N)
ex) CRUDN, CRU_N,..(5 Charaters)
Permission : CRUDN
Num. of Rowner : 1
-URN identifying the rowner
ex) lightDeviceUUID0 (16 Numbers except to '-')
[1]Rowner : lightDeviceUUID0
Sending ACL is succeed~!!
Enter number of revoced certificates (1..9)
1
Revoked certificate 0:
Serial number (E. g.: 100):
2
Sending CRL is succeed~!!

# Change window to terminal where Door server is running
# Enter 'd' for  discovery. You should see output like this:
21:56.283 INFO: DEMO: isUpdated is false...
21:56.495 INFO: DEMO: Callback Context for DISCOVER query recvd successfully
21:56.495 INFO: DEMO: StackResult: OC_STACK_OK
21:56.495 INFO: DEMO: Device =============> Discovered @ 10.0.2.15:37942
21:56.495 INFO: DEMO: Payload Type: Discovery
21:56.495 INFO: DEMO: 	Resource #1
21:56.495 INFO: DEMO: 	URI:/a/light
21:56.495 INFO: DEMO: 	SID:
21:56.495 INFO: DEMO: F0 5A 6C 8B 59 66 48 89 BE 1E 4E EF FA 23 4E FD
21:56.495 INFO: DEMO: 	Resource Types:
21:56.495 INFO: DEMO: 		core.light
21:56.495 INFO: DEMO: 	Interfaces:
21:56.495 INFO: DEMO: 		oic.if.baseline
21:56.495 INFO: DEMO: 	Bitmap: 3
21:56.495 INFO: DEMO: 	Secure?: true
21:56.495 INFO: DEMO: 	Port: 43910
21:56.495 INFO: DEMO:
21:56.495 INFO: DEMO: Uri -- /a/light
21:56.495 INFO: DEMO: Secure -- YES
21:56.591 INFO: DEMO: Callback Context for DISCOVER query recvd successfully
21:56.591 INFO: DEMO: StackResult: OC_STACK_OK
21:56.591 INFO: DEMO: Device =============> Discovered @ 10.0.2.15:55808
21:56.591 INFO: DEMO: Payload Type: Discovery
21:56.591 INFO: DEMO: 	Resource #1
21:56.591 INFO: DEMO: 	URI:/a/door
21:56.591 INFO: DEMO: 	SID:
21:56.591 INFO: DEMO: E9 68 45 ED 5D E1 4A F3 86 31 FD 0E 5E 25 EB B3
21:56.591 INFO: DEMO: 	Resource Types:
21:56.591 INFO: DEMO: 		core.door
21:56.591 INFO: DEMO: 	Interfaces:
21:56.591 INFO: DEMO: 		oic.if.baseline
21:56.591 INFO: DEMO: 	Bitmap: 3
21:56.591 INFO: DEMO: 	Secure?: true
21:56.591 INFO: DEMO: 	Port: 41403
21:56.591 INFO: DEMO:
21:56.591 INFO: DEMO: Uri -- /a/door
21:56.591 INFO: DEMO: Secure -- YES

# If you can see /a/light discowered then this is success.
# Next you should enter g to start get request
# Enter address : 10.0.2.15:43910
# Port you can find here
21:56.495 INFO: DEMO: 	URI:/a/light
21:56.495 INFO: DEMO: 	SID:
21:56.495 INFO: DEMO: F0 5A 6C 8B 59 66 48 89 BE 1E 4E EF FA 23 4E FD
21:56.495 INFO: DEMO: 	Resource Types:
21:56.495 INFO: DEMO: 		core.light
21:56.495 INFO: DEMO: 	Interfaces:
21:56.495 INFO: DEMO: 		oic.if.baseline
21:56.495 INFO: DEMO: 	Bitmap: 3
21:56.495 INFO: DEMO: 	Secure?: true
21:56.495 INFO: DEMO: 	Port: 43910

# If you see this lines in output:
22:31.647 INFO: DEMO: Callback Context for GET query recvd successfully
22:31.647 INFO: DEMO: StackResult: OC_STACK_OK
22:31.647 INFO: DEMO: SEQUENCE NUMBER: 2
22:31.647 INFO: DEMO: Payload Type: Representation
22:31.647 INFO: DEMO: 	Resource #1
22:31.647 INFO: DEMO: 	URI:/a/light
22:31.647 INFO: DEMO: 	Resource Types:
22:31.647 INFO: DEMO: 	Interfaces:
22:31.647 INFO: DEMO: 	Values:
22:31.647 INFO: DEMO: 		brightness(int):0
22:31.647 INFO: DEMO: =============> Get Response
# then certificate did not rejected with CRL
# if not then it did.
