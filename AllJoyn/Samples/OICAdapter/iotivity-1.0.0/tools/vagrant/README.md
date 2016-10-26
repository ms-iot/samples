IoTivity Build VM from Vagrant
==============================

Vagrant can use the files in this directory to provision an Ubuntu 12.04
development environment for IoTivity.  To create a new VM type

    % vagrant up

in the current directory.  You can then connect to the VM with

    % vagrant ssh

On the newly provisioned VM, the IoTivity repo will be checked out in
the `iotivity` directory in the vagrant user's home directory and
configured to use the Arduino SDKs which are automatically downloaded
and installed in the home directory and patched when the VM is provisioned.

Using Vagrant to build a VM might be useful for:

* Testing the build process on a clean install to make sure all dependencies
  are captured

* Configuring a Linux build environment for Windows or Mac users

* Configuring build VMs for build automation

The bootstrap.sh and iotivity-setup.sh files can be used independent of
Vagrant to configure a existing Ubuntu environment.  How to do so is left
as an exercise for the reader.  (Hint: try `sudo bootstrap.sh &&
iotivity-setup.sh`.)

For more information about Vagrant, please see https://docs.vagrantup.com/v2/

Assumptions
===========

* The ${HOME}/.ssh/ directory contains config, id_rsa, id_rsa.pub, and
  known_hosts, that when copied to the VM will allow connection to the
  git repo.  (And that you don't mind those files being copied to the VM.)

* If a USER environment variable, it has the user name to use for git
  otherwise, USERNAME has the user ID for git.  USER should work for
  Linux and USERNAME works inside Git bash on Windows.

To Do
=====

* Everything is dumped into the top-level directory.  A cleaner layout
  would be better (e.g., move the Arduino libraries to iotivity/extlibs).

* Verify on more configurations (tested on Git bash under Windows)

* Try with Ubuntu 14.04

* Have a more flexible way to determine the git user name

* Install Android NDK
