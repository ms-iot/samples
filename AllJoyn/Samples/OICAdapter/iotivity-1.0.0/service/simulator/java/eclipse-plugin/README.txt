Eclipse plug-in

Pre-requisites
--------------
    1.Build the IoTivity project for linux:
        Run the "scons" command in iotivity home directory.
        It generates the libraries in ~/iotivity/out/linux/<arch>/release directory.
    2.Copy the libraries mentioned below into the libs folder of the plug-in project.
        Required libraries: libSimulatorManager.so, liboc.so, liboctbstack.so, and liboc_logger.so
    3.Import the Simulator Java SDK project from ~/iotivity/service/simulator/java/sdk/ into Eclipse IDE as given below.
        File -> Import -> Select 'Existing projects into Workspace' under General category -> click next -> Browse to the above mentioned location ->
        click Finish.
      Export the sdk project as JAR file.
        Right click the project -> Export -> select 'JAR file' option under Java -> Next -> Finish.
      Copy the JAR file into the libs folder of the plug-in project.

Steps to run the plug-in
------------------------
    1.Import the plug-in project from ~/iotivity/service/simulator/java/eclipse-plugin/ into Eclipse IDE as given below.
        File -> Import -> Select 'Existing projects into Workspace' under General category -> click next -> Browse to the above mentioned location ->
        click Finish.
    2.Set the LD_LIBRARY_PATH environment variable
        Right click the project -> Properties -> Run/Debug Settings -> Edit -> select 'Environment' tab -> click on 'Select' -> check LD_LIBRARY_PATH option -> Ok.
        Edit the LD_LIBRARY_PATH and add the complete path to the libs folder of the plug-in project -> Apply -> OK.
        Then Apply -> OK to close the properties window.
    3.Right click the project -> Run As Eclipse Application.