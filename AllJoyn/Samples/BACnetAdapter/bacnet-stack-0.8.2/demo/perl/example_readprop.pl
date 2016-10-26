use warnings;
use strict;

my (
    $device,     # device instance number
    $objectName, # object type name
    $objectInst, # object instance number
    $propName,   # property name
    $index,      # property index
);

GetOptions(
    'device=i'   => \$device,
    'objName=s'  => \$objectName,
    'objInst=i'  => \$objectInst,
    'property=s' => \$propName,
    'index=i'    => \$index,
);

Help() unless ( defined($device)     &&
                defined($objectName) &&
                defined($objectInst) &&
                defined($propName)
);

my ($resp, $failed) = ReadProperty($device, $objectName, $objectInst, $propName, $index);
print "status was '$failed' and the response was '$resp'\n";

sub Help {
    print <<END;

This script demonstrates the ReadProperty service functionality using Perl
bindings. To run this script, you must specify the following arguments to it:
  * device   This is the device instance number (i.e. 1234)
  * objName  This is the object type name (i.e. OBJECT_ANALOG_VALUE). See
             include/bacenum.h for complete list
  * objInst  This is the object instance number you want to read (i.e. 1)
  * property This is the name of the property you want to read (i.e.
             PROP_PRESENT_VALUE). See include/bacenum.h for complete list
  * index    This is an optional parameter. If you want to read from a specific
             index, then specify here (i.e. 1). Otherwise, don't specify this
             option.

  As a complete example, to run this script using the main bacnet tool to read
  AnalogValue1.PresentValue from device instance 1234, use

perl bacnet.pl --script example_readprop.pl -- --device=1234 --objName=OBJECT_ANALOG_VALUE --objInst=1 --property=PROP_PRESENT_VALUE

END
    exit 1;
}

1;
