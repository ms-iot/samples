Procedure to Execute Sample App:
================================
    1) Copy the generated rpm(com-oic-ca-sample-0.1-1.armv7l.rpm) to Tizen Device
       (say /opt/usr/media/xxx/)
    2) sdb shell
    3) su
    4) cd  /opt/usr/media/xxx/

    5) change-booting-mode.sh --update

    6) Install casample(console app). It contains Interface API libs and OIC-CORE Libs
        i) rpm -Uvh com-oic-ca-sample-0.1-1.armv7l.rpm --force --nodeps

    7) RPM will be installed in "/usr/apps/"
       Execute the sample app by below commands
        i) cd /usr/apps/com.oic.ca.sample/bin
        ii) ./ca_sample (for CA Sample)

Apply smack rule[ONLY for WIFI]
===============================

    1) After installing the RPMs, apply the following smack rule
        a) chsmack -a "net-config" /usr/apps/com.oic.ca.sample/bin/ca_sample
        b) chsmack -e "net-config" /usr/apps/com.oic.ca.sample/bin/ca_sample

    Note: EDR adapter will give permission issues if these rules are applied.
          So to test EDR adapter, delete the "/usr/apps/com.oic.ca.sample" folder
          and again install the RPMs.

Install Dependent RPMS(Only for EDR Adapter Testing)
====================================================

    1) In su(super user) mode, install Bluetooth related RPMs present in
    "connectivity/lib/tizen/edr/rpms" on Tizen v2.3 mobile device
    (Currently only SM Z910F Device is supported).
    2) Install the Sample as given above in "Procedure to Execute Sample App"

Install Dependent RPMS(Only for BLE Adapter Testing)
====================================================

    1) Flash the system image present in connectivity/lib/tizen/ble/image/ folder on SM Z910F Tizen v2.3
    2) In su(super user) mode, Install Bluetooth related rpms Present in connectivity/lib/tizen/ble/rpms on
    Tizen v2.3 mobile device (Currently only SM Z910F Device is supported)
    3) Install the Sample as given above in "Procedure to Execute Sample App" [ Till Step 6 ]
    4) Open another sdb shell, in su (super user) mode follow the below steps:
            (i)   cd /var/lib/bluetooth
            (ii)  Turn off Bluetooth in the device
            (iii) rm -rf *
            (iv)  Run the command ./usr/lib/bluetooth/bluetoothd -ndE &. for OIC Server device and
            run the command ./usr/lib/bluetooth/bluetoothd -nd &. for OIC Client device.
            (v)   Turn on the Bluetooth.
    5) Run the sample as given in Step 8 of "Procedure to Execute Sample App"

