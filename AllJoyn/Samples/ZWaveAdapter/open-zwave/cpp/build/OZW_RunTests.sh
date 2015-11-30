#/bin/sh

#### Include the library
. `dirname $0`/sh2ju.sh

topsrcdir=$1
#### Clean old reports
juLogClean

#### A call to a customized method
CheckXML() {
   xmllint --noout --schema $*
   return $?
}

juLog  -name=CheckDeviceClassesXML		CheckXML "$topsrcdir/config/device_classes.xsd $topsrcdir/config/device_classes.xml"
juLog  -name=CheckOptionsXML			CheckXML "$topsrcdir/config/options.xsd $topsrcdir/config/options.xml"
juLog  -name=CheckManufactureSpecificXML	CheckXML "$topsrcdir/config/manufacturer_specific.xsd $topsrcdir/config/manufacturer_specific.xml"
for file in $(find $topsrcdir/config/ \( -name "*.xml" ! -name "device_classes.xml" ! -name "options.xml" ! -name "manufacturer_specific.xml" \) )
do
	juLog -name=$file CheckXML "$topsrcdir/config/device_configuration.xsd $file"
done		
