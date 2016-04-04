use warnings;
use strict;
use Getopt::Long;
use Convert::Binary::C;
use Hash::Util qw/lock_hash/;
use English;
use Scalar::Util qw/looks_like_number/;
use File::Basename;
use File::Spec;
use Pod::Usage;
use Carp;

=head1 NAME

bacnet.pl - Scriptable BACnet communications 

=head1 DESCRIPTION

This is a tool for scriptable BACnet communication. Users can write their own
scripts using standard Perl syntax and API defined in this tool to perform desired
execution sequences. For details on this tool's API, see Documentation.html. For other
Perl documentation, see http://perldoc.perl.org

=begin html
<link href="syntax.css" rel="stylesheet" type="text/css">
<script src="jquery.js"></script>
<script src="syntax.js"></script>

=end html

=head1 OPTIONS

Usage: bacnet.pl [program_options] [-- script_args]

This program executes a script in perl syntax to perform BACnet/IP operations.
 
 Possible program options:
   --script=s    The script to execute.
   --log=s       The file to log all output.
   --help        This help message.

 Possible environment variables are:
    BACNET_IFACE - set this value to dotted IP address of the interface (see
         ipconfig) for which you want to bind.  Default is the interface which
         Windows considers to be the default (how???).  Hence, if there is only a
         single network interface on Windows, the applications will choose it, and
         this setting will not be needed.
    BACNET_IP_PORT - UDP/IP port number (0..65534) used for BACnet/IP
         communications.  Default is 47808 (0xBAC0).
    BACNET_APDU_TIMEOUT - set this value in milliseconds to change the APDU
         timeout.  APDU Timeout is how much time a client waits for a response from
         a BACnet device.
    BACNET_BBMD_PORT - UDP/IP port number (0..65534) used for Foreign Device
         Registration.  Defaults to 47808 (0xBAC0).
    BACNET_BBMD_TIMETOLIVE - number of seconds used in Foreign Device
         Registration (0..65535). Defaults to 60000 seconds.
    BACNET_BBMD_ADDRESS - dotted IPv4 address of the BBMD or Foreign Device
         Registrar.

=cut

############################################
#   Steps to prepare for execution
############################################

# This is the relative path to get to the base directory cotaining the BACnet
# Stack sources from the directory containing this file and the directory
# within which InlineC code is built. The reason for delaring it here and
# setting the value in a BEGIN block is so that the variable gets its value at
# compile time before Inline::C tries to use that variable.
my $relSourcePath;
my $inlineCFile;
my $inlineBuildDir;
my $libDir;
my $incDir1;
my $incDir2;
my $incDir3;
BEGIN {
    # the Perl source file is in the same directory as in the InlineC file
    # this path should not contain any spaces
    $relSourcePath = File::Spec->rel2abs(dirname($0));
    die "Install path must not have spaces.\n" if $relSourcePath =~ /\s/;
    my @dirs = ();
    push @dirs, $relSourcePath;
    $inlineCFile = File::Spec->catfile(@dirs, "perl_bindings.c");

    # all Inline C sources shall be contained in ./.Inline
    push @dirs, ".Inline";
    $inlineBuildDir = File::Spec->catdir(@dirs);
    pop @dirs;

    # to properly link, need to reference ./../../lib
    push @dirs, "..";
    push @dirs, "..";
    push @dirs, "lib";
    $libDir = File::Spec->catdir(@dirs);
    pop @dirs;

    # to properly build, need to reference ./../../include
    push @dirs, "include";
    $incDir1 = File::Spec->catdir(@dirs);
    pop @dirs;

    # we will use the demo handlers, need to reference ./../../demo/object
    push @dirs, "demo";
    push @dirs, "object";
    $incDir2 = File::Spec->catdir(@dirs);
    pop @dirs;
    pop @dirs;

    # TODO: This should be done in a more universal way
    # to properly build Win32 ports, need to refrence ./../../ports/win32
    push @dirs, "ports";
    push @dirs, "win32";
    $incDir3 = File::Spec->catdir(@dirs);
}

use Inline (
    C => Config => 
        LIBS      => "-L$libDir -lbacnet -liphlpapi",
        INC       => ["-I$incDir1", "-I$incDir2", "-I$incDir3"],
        DIRECTORY => $inlineBuildDir,
);

# this is the C source file for interfacing to the library. Yes, this could be
# done natively in Perl, but this is just as easy (and probably faster to
# execute).
use Inline C => "$inlineCFile";


my $ask_help = 0;
my $script;
my $log;
my $logTo = \*STDOUT;
my $logIndent = 0;
my $logIsQuiet = 0;
my $errorMsg;
my $answer = '';

($ask_help = 1) unless GetOptions(
    'help|?'   => \$ask_help,
    'script=s' => \$script,
    'log=s'    => \$log,
); 

if (!defined($script) || !(-f $script))
{
    print "Bad or no script file scpecified.\n";
    $ask_help = 1;
}
else
{
    # Add the script's location to @INC so that they can include other scripts 
    # using relative paths
    my $scriptdir = File::Spec->rel2abs(dirname($script));
    push @INC,$scriptdir;
}

if ($ask_help) {
    print "============================\n\n";
    pod2usage(
         -exitval  => 0,
         -verbose  => 99,
         -sections => "NAME|DESCRIPTION|OPTIONS"
    );
}

if (defined($log))
{
    open(LOG, ">$log") || croak "Cannot open $log for writing: $!\n";
    $logTo = \*LOG;
}

# Pull in the BACnet enumerations from the C header file
my %C_ENUMS;
eval {
    my $pwd = File::Spec->rel2abs(File::Spec->curdir());

    # let's get into the directory so that we can pull in the bacnet enumerations
    my @dirs = ();
    push @dirs, dirname($0);
    push @dirs, "../../include";
    chdir(File::Spec->catdir(@dirs));
    my $c = Convert::Binary::C->new->parse_file('bacenum.h');
    foreach my $typedef ($c->typedef)
    {
        if (ref($$typedef{type}) eq "HASH")
        {
            my $enumeration = \%{$C_ENUMS{$$typedef{declarator}}};
            foreach my $enum_name (keys %{$$typedef{type}{enumerators}})
            {
                ${$C_ENUMS{$$typedef{declarator}}}{$enum_name} = ${$$typedef{type}{enumerators}}{$enum_name};
            }
        }
    }
    lock_hash(%C_ENUMS);
    chdir($pwd);
};
if ($EVAL_ERROR)
{
    croak "Error pulling in the enumerations. $@\n";
}

# Prepare things for communication
BacnetPrepareComm();

# Execute the user specified script
Log("Executing $script - start time " . scalar(localtime(time())) );
unless (my $return = do $script)
{
    croak "could not parse $script: $@"   if $@;
    croak "could not pull in $script: $!" unless defined $return;
    croak "could not execute $script"     unless $return;
}
Log("Finished executing $script - end time " . scalar(localtime(time())) );

=head1 This tool's API

In addition to having all standard Perl flow control, functions, and modules,
the this tool provides an API for performing BACnet communication functions.

=cut

##########################################
#  This block is the external API
##########################################

=head2 ReadProperty

This function implements the ReadProperty service. There are no built in retry
mechanisms. NOTE: all enumerations are defined in F<bacenum.h>

=head3 Inputs to ReadProperty

=begin html
<ul>
  <li><b>devideInstance</b> - the instance number of the device we are reading</li>
  <li><b>objectName</b>     - the enumeration for the object name we are reading</li>
  <li><b>objectInstance</b> - the instance number of the object we are reading</li>
  <li><b>propertyName</b>   - the enumeration  for the property name we are reading</li>
  <li><b>index</b>          - Optional (default -1): the index number we are reading from. -1 if not applicable</li>
</ul>

=end html

=head3 Outputs from ReadProperty

=begin html
<ul>
  <li><b>result</b>    - the sting result (value or error) for ReadProperty</li>
  <li><b>isFailure</b> - zero means no failure, non-zero means failure</li>
</ul>

=end html

=head3 Example of ReadProperty

The following example will read AV0.PresentValue from device 1234

    my ($res, $failed) = ReadProperty(1234, 'OBJECT_ANALOG_VALUE', 0, 'PROP_PRESENT_VALUE');

=cut

sub ReadProperty {
    my $deviceInstance = shift;
    my $objectName = shift;
    my $objectInstance = shift;
    my $propertyName = shift;
    my $index = shift;
    my $isFailure = BindToDevice($deviceInstance);

    # Loop for early exit
    while(1)
    {
        last if $isFailure;

        my ($objectPrintName, $objectValue) = LookupEnumValue('BACNET_OBJECT_TYPE', $objectName);
        my ($propertyPrintName, $propertyValue) = LookupEnumValue('BACNET_PROPERTY_ID', $propertyName);

        my $msg = "ReadProperty $objectPrintName" . '[' . $objectInstance . "].$propertyPrintName";
        if (defined($index))
        {
            $msg .= ".$index";
        } else {
            $index = -1;
        }
        $msg .= " from Device" . '[' . $deviceInstance . "] ==> ";

        LogAnswer('', 0);
        if ( BacnetReadProperty($deviceInstance, $objectValue, $objectInstance, $propertyValue, $index) )
        {
            BacnetGetError($errorMsg);
            $msg .= "Problem: $errorMsg";
            $isFailure = 1;
        }
        else
        {
            $msg .= $answer;
            $isFailure = 0;
        }
        Log($msg);
        last;
    }

    return ($answer, $isFailure);
}

=head2 ReadPropertyMultiple

This function implements the ReadPropertyMultiple service. There are no built in retry
mechanisms. NOTE: all enumerations are defined in F<bacenum.h> 

=head3 Inputs to ReadPropertyMultiple

=begin html
<ul>
  <li><b>devideInstance</b> - the instance number of the device we are reading</li>
  <li><b>r_answerList</b>   - reference to a list where to store the answers</li>
  <li><b>list</b>           - a list of ReadAccessSpecifications</li>
  <ul>
    <li><b>objectType</b>     - the enumeration for the object name to read from</li>
    <li><b>objectInstance</b> - the instance number of the object we are reading</li>
    <li><b>propertyName</b>   - the enumeration  for the property name we are reading</li>
    <li><b>index</b>          - the index number we are reading from. Use -1 if not applicable</li>
  </ul>
</ul>

=end html

=head3 Outputs from ReadPropertyMultiple

=begin html
<ul>
  <li><b>result</b>    - the 'QQQ' delimited concatenated sting result (value or error) for ReadPropertyMultiple. The parsed out result is returned in r_answerList</li>
  <li><b>isFailure</b> - zero means no failure, non-zero means failure</li>
</ul>

=end html

=head3 Example of ReadPropertyMultiple

The following example will read AV0.PresentValue and AV1.PresentValue from device 1234

    my @RPM_request = ();
    my @RPM_answer = ();
    my $failed;
    push @RPM_request, ['OBJECT_ANALOG_VALUE', 0, 'PROP_PRESENT_VALUE', -1];
    push @RPM_request, ['OBJECT_ANALOG_VALUE', 1, 'PROP_PRESENT_VALUE', -1];
    (undef, $failed) = ReadPropertyMultiple(1234, \@RPM_answer, @RPM_request);

=cut

sub ReadPropertyMultiple
{
    my $deviceInstanceNumber = shift;
    my $r_answerList = shift;
    my @list = @ARG;
    my @modifiedList = ();
    my $msg = '';
    my $isFailure = BindToDevice($deviceInstanceNumber);

    # loop for early exit
    while(1)
    {
        last if $isFailure;

        Log("ReadPropertyMultiple:");
        $logIndent += 4;

        foreach my $r_prop (@list)
        {
            my @tmpList = ();
            push @tmpList, $$r_prop[$_] for (0 .. 3);
            (undef, $tmpList[0]) = LookupEnumValue('BACNET_OBJECT_TYPE', $$r_prop[0]);
            (undef, $tmpList[2]) = LookupEnumValue('BACNET_PROPERTY_ID', $$r_prop[2]);
            push @modifiedList, \@tmpList;
        }

        LogAnswer('', 0);
        @{$r_answerList} = ();
        if (BacnetReadPropertyMultiple($deviceInstanceNumber, @modifiedList))
        {
            BacnetGetError($errorMsg);
            Log("Problem: $errorMsg");
            $isFailure = 1;
        }
        else
        {
            my $i = 0;
            foreach (split('QQQ', $answer))
            {
                my ($objectPrintName, undef) =  LookupEnumValue('BACNET_OBJECT_TYPE', $list[$i][0]);
                my ($propertyPrintName, undef) = LookupEnumValue('BACNET_PROPERTY_ID', $list[$i][2]);
                my $msg = $objectPrintName . '.[' . $list[$i][1] . '].' . $propertyPrintName;
                if ($list[$i][3] != -1)
                {
                    $msg .= '.[' . $list[$i][3] . ']';
                }
                $msg .=  " ==> $_";
                Log($msg);
                push @{$r_answerList}, $_;
                $i++;
            }
            $isFailure = 0;
        }

        $logIndent -= 4;
        last;
    }

    return ($answer, $isFailure);
}

=head2 WriteProperty

This function implements the WriteProperty service. There are no built in retry
mechanisms. NOTE: all enumerations are defined in F<bacenum.h> 

=head3 Inputs to WriteProperty

=begin html
<ul>
  <li><b>devideInstance</b> - the instance number of the device we are writing</li>
  <li><b>objectName</b>     - the enumeration for the object name we are writing</li>
  <li><b>objectInstance</b> - the instance number of the object we are writing</li>
  <li><b>propertyName</b>   - the enumeration for the property name we are writing</li>
  <li><b>tagName</b>        - the enumeration for the type of value we are writing. To specify context tags, prepend the tag name with "Cn:" where 'n' is the context number.</li>
  <li><b>value</b>          - the value we are writing</li>
  <li><b>priority</b>       - Optional (default 0): the priority within Priority Array to write at. Use 1-16 when specify priority, 0 to not specify priority.</li>
  <li><b>index</b>          - Optional (default -1): the index within an array we are writing to. Use positive number to indicate index, -1 to not specify index.</li>
</ul>

=end html

=head3 Outputs from WriteProperty

=begin html
<ul>
  <li><b>result</b>    - the sting result (value or error) for WriteProperty</li>
  <li><b>isFailure</b> - zero means no failure, non-zero means failure</li>
</ul>

=end html

=head3 Example of WriteProperty

The following example will write 1.0 to AV0.PresentValue in device 1234

    my ($res, $failed) = WriteProperty(1234, 'OBJECT_ANALOG_VALUE', 0, 'PROP_PRESENT_VALUE', 'BACNET_APPLICATION_TAG_REAL', 1.0);

=cut

sub WriteProperty {
    my $deviceInstance = shift;
    my $objectName = shift;
    my $objectInstance = shift;
    my $propertyName = shift;
    my $tagName = shift;
    my $value = shift;
    my $priority = shift;
    my $index = shift;
    my $isFailure = BindToDevice($deviceInstance);

    # loop for early exit
    while(1)
    {
        last if $isFailure;

        my ($objectPrintName, $objectValue) = LookupEnumValue('BACNET_OBJECT_TYPE', $objectName);
        my ($propertyPrintName, $propertyValue) = LookupEnumValue('BACNET_PROPERTY_ID', $propertyName);

        my $tagValue = '';
        if ($tagName =~ /^(C\d+):(.*)$/)
        {
            $tagName = $2;
            $tagValue = "$1 ";
        }
        my ($tagPrintName, $tagNewValue) = LookupEnumValue('BACNET_APPLICATION_TAG', $tagName);
        $tagValue .= $tagNewValue;

        my $msg = "WriteProperty $tagPrintName:$value to $objectPrintName" . '[' . $objectInstance . "].$propertyPrintName";
        if (defined($index))
        {
            $msg .= '[' . $index . ']';
        }
        else
        {
            # an index of -1 means that we are not writing to an array
            $index = -1;
        }
        if (defined($priority))
        {
            $msg .=  '@' . $priority
        }
        else
        {
           # a priority of 0 means we are not writing to a priority array 
           $priority = 0;
        }
        $msg .= " in Device" . '[' . $deviceInstance . "] ==> ";

        LogAnswer('', 0);
        if ( BacnetWriteProperty($deviceInstance, $objectValue, $objectInstance, $propertyValue, $priority, $index, $tagValue, $value) )
        {
            BacnetGetError($errorMsg);
            $msg .= "Problem: $errorMsg\n";
            $isFailure = 1;
        }
        else
        {
            $msg .= $answer;
            $isFailure = 0;
        }
        Log($msg);
        last;
    }

    return ($answer, $isFailure);
}

=head2 TimeSync

This function implements the TimeSync and UTCTimeSync services

=head3 Inputs to TimeSync

=begin html
<ul>
    <li><b>deviceInstanceNumber</b> - the instance number of the device we are  reading</li>
    <li><b>year</b>                 - Year (i.e. 2011)</li>
    <li><b>month</b>                - Month (i.e. 11 for November)</li>
    <li><b>day</b>                  - Day (i.e. 1 for first of month)</li>
    <li><b>hour</b>                 - Hour (i.e. 23 for 11pm)</li>
    <li><b>minute</b>               - Minute (i.e. 0-59)</li>
    <li><b>second</b>               - Second (i,e. 0-59)</li>
    <li><b>utcOffset</b>            - Optional: if specified defines the UTC offset and forces UTCTimeSync</li>
</ul>

=end html

=head3 Outputs from TimeSync

=begin html
<ul>
  <li><b>isFailure</b> - zero means no failure, non-zero means failure</li>
</ul>

=end html

=head3 Example of TimeSync

    $isFailure = TimeSync($deviceInstance, $1, $2, $3, $4, $5, $6) unless $isFailure;

=cut

sub TimeSync 
{
    my $deviceInstanceNumber = shift;
    my $year = shift;
    my $month = shift;
    my $day = shift;
    my $hour = shift;
    my $minute = shift;
    my $second = shift;
    my $utcOffset = shift;
    my $isUTC;

    my $isFailure = BindToDevice($deviceInstanceNumber);

    # loop for early exit
    while(1)
    {
        last if $isFailure;

        # be a pessimist. Assume things will fail
        $isFailure = 1;
        
        if (defined($utcOffset))
        {
            $isUTC = 1;
            Log("UTC Time Sync not yet supported.");
            last;
        }
        else
        {
            $utcOffset = 0;
            $isUTC = 0;
        }
   
        if ($year < 1900 || $year > 2099)
        {
            Log("Year '$year' is invalid.");
            last;
        }

        if ($month <= 0 || $month > 12)
        {
            Log("Month '$month' is invalid.");
            last;
        }

        if ($day <= 0 || $day > 31)
        {
            Log("Day '$day' is invalid.");
            last;
        }

        if ($hour < 0 || $hour > 23)
        {
            Log("Hour '$hour' is invalid.");
            last;
        }
        
        if ($minute < 0 || $minute > 59)
        {
            Log("Minute '$minute' is invalid.");
            last;
        }

        if ($second < 0 || $second > 59)
        {
            Log("Second '$second' is invalid.");
            last;
        }

        Log("TimeSync: Device[$deviceInstanceNumber] $year/$month/$day $hour:$minute:$second");

        $isFailure = BacnetTimeSync($deviceInstanceNumber, $year, $month, $day, $hour, $minute, $second, $isUTC, $utcOffset);
        last;
    }

    return $isFailure; 
}

=head2 Log

This function prints out to the desired method of logging (STDOUT or file).
NewLine characters are not required when making calls to this function. If any
NewLine characters are specified, they will be stripped out. To print an empty
line, pass in a space as the message. NOTE: This function will honor previous 
requests to silence the log (see SilcenseLog for details)

=head3 Inputs to Log

=begin html
<ul>
  <li><b>msg</b> - the message to output
</ul>

=end html

=head3 Example of Log

The following example will print out "hello world"

    Log("Hello World");

=cut

###############################################################################
# Global Variables affecting this function
#    logIsQuiet  do not print anytihng if the log was qieted
#    logIndent   how many spaces to put in front of every logged line
###############################################################################
sub Log {
    my $msg = shift;

    if (defined($msg) && !$logIsQuiet)
    {
        my @last = split('', substr($msg, -2));

        # if there is nothing to print, then don't do it
        return if (scalar(@last) == 0);
        
        # if there are newline-like characters, get rid of them.
        while ($msg =~/^(.*)[\r\n]+(.*)$/)
        {
            $msg = $1 . $2;
        }

        local $OUTPUT_RECORD_SEPARATOR = "\n";
        print $logTo ' ' x $logIndent . $msg;
    }
}

=head2 SilenceLog

This function requests that all future log messages be either suppressed or
enabled.

=head3 Inputs to SilenceLog

=begin html
<ul>
  <li><b>logIsQuiet</b> - zero means print to log, non-zero means supress log
</ul>

=end html

=head3 Outputs from SilenceLog

The previous value of whether or not the log was silenced before caling this
function.

=head3 Example of SilenceLog

The following example will print out "hello", but not "world"

    Log("Hello");
    SilenceLog(1);
    Log("World");

=cut

sub SilenceLog {
    my $prevValue = $logIsQuiet;
    $logIsQuiet = shift;
    return $prevValue;
}

=head2 Retry

This function will try to execute the requested command up to specified number
of times, awaiting the requested answer, with a specified pause between
retries.  NOTE: the only functions which can be executed by this function are
ones which return two parameres in the form of ($response, $isFailure)

=head3 Inputs to Retry

=begin html
<ul>
  <li><b>r_func</b>        - The reference to the function which is to be retried</li>
  <li><b>r_funcArgs</b>    - A reference to an array of arguments for the function to be executed</li>
  <li><b>desiredOutput</b> - The condition which will terminate the retrying. Can be either a number or a regexp to patch against the $response return of the function</li>
  <li><b>maxTries</b>      - The maximum number of retry attempts before calling it quits</li>
  <li><b>sleepSeconds</b>  - The number of seconds (could be fractional) to wait between retries</li>
</ul>

=end html

=head3 Outputs from Retry

=begin html
<ul>
  <li><b>$resp</b>     - The response from the last execution of requested function</li>
  <li><b>isFailure</b> - zero means no failure, non-zero means failure</li>
</ul>

=end html

=head3 Example of Retry

The following example will execute the ReadProperty function to read a property
from an object (see ReadProperty for details on those arguments) with up to
$maxRetries retries (with $retryDelay delay between retries) or unitl the
desired answer of 42 is received.

    my ($resp, $isFailure) = Retry(
                \&ReadProperty, [$deviceInstance, 'OBJECT_ANALOG_VALUE', 0, 'PROP_PRESENT_VALUE'],
                42, $maxRetries, $retryDelay
              );
    if ($isFailure)
    {
        die "Value was not 42. Last response was '$resp'";
    }

The following example will try to execute a WriteProperty (see that function for
details on its arguments) until the write succeeds.

    my ($resp, $isFailure) = Retry(
                \&WriteProperty, [$deviceInstance, 'OBJECT_ANALOG_VALUE', 0, 'PROP_PRESENT_VALUE', 'BACNET_APPLICATION_TAG_REAL', 42.0],
                "Acknowledged", $maxRetries, $retryDelay
              );
    if ($isFailure)
    {
        die "Could not write 42. Last response was '$resp'";
    }

=cut
sub Retry {
    my $r_func = shift;
    my $r_funcArgs = shift;
    my $desiredOutput = shift;
    my $maxTries = shift;
    my $sleepSeconds = shift;

    my ($resp, $failed);

    my $i;
    for ($i=0; $i<$maxTries; $i++)
    {
        ($resp, $failed) = &{$r_func}(@{$r_funcArgs});
        unless ($failed)
        {
            if (looks_like_number($desiredOutput))
            {
                last if (looks_like_number($resp) && ($resp == $desiredOutput));
            }
            else
            {
                last if ($resp =~ /$desiredOutput/);
            }
        }
        select(undef, undef, undef, $sleepSeconds);
    }

    return ($resp, ($i == $maxTries));
}


##########################################
#  These are the supporting functions
##########################################

sub LookupEnumValue {
    my $enumType = shift;
    my $enumName = shift;
    my $printName;

    if (!exists($C_ENUMS{$enumType}{$enumName}))
    {
        print "Requested enumeration '$enumName' does not exist within '$enumType'.\n";
        exit -1;
    }

    # lookup the  value
    my $value = $C_ENUMS{$enumType}{$enumName};

    # reformat the OBJECT name style
    my %reformat = (
        'BACNET_PROPERTY_ID'     => 'PROP',
        'BACNET_OBJECT_TYPE'     => 'OBJECT',
        'BACNET_APPLICATION_TAG' => 'BACNET_APPLICATION_TAG',
    );

    if (exists($reformat{$enumType}))
    {
        if ($enumName =~ /$reformat{$enumType}_(.*)/)
        {
            $printName = '';
            $printName .= ucfirst lc $_ foreach (split('_', $1));
        }
    }

    return ($printName, $value);
}

sub BindToDevice {
    my $deviceInstance = shift;
    my $isFailure = 0;

    if ( BacnetBindToDevice($deviceInstance) )
    {
        BacnetGetError($errorMsg);
        Log("Problem binding to deivce $deviceInstance: $errorMsg\n");
        $isFailure = 1;
    }

    return $isFailure;
}

sub LogAnswer {
    my $newAnswer = shift;
    my $append = shift;

    $answer = '' unless $append;
    $answer .= $newAnswer;
}

