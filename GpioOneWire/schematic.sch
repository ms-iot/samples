<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="7.6.0">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="RHT03_DHT-22_AM2302">
<description>&lt;h1&gt;RHT03 / DHT-22 / AM2302 library&lt;/h1&gt;
&lt;p&gt;
Library with devices (schematic symbol and packages) for the RHT03 / DHT-22 / AM2302 digital relative humidity and tempearture sensor.&lt;br /&gt;
Warning: Package does not include tKeepout areas so that the part may optionally be placed on a riser with components underneath.  Please keep the package outline and bent-pin space in mind.
&lt;/p&gt;</description>
<packages>
<package name="RHT03_DHT-22">
<description>&lt;h1&gt;RHT03 / DHT-22&lt;/h1&gt;
&lt;p&gt;
Package for the RHT03 / DHT-22 relative humidity and temperature sensor.
&lt;/p&gt;</description>
<pad name="VDD" x="-3.81" y="0" drill="0.8" shape="octagon"/>
<pad name="DATA" x="-1.27" y="0" drill="0.8"/>
<pad name="NULL" x="1.27" y="0" drill="0.8"/>
<pad name="GND" x="3.81" y="0" drill="0.8"/>
<wire x1="-7.55" y1="2.5" x2="7.55" y2="2.5" width="0.127" layer="21"/>
<wire x1="-7.55" y1="-5.2" x2="7.55" y2="-5.2" width="0.127" layer="21"/>
<wire x1="-7.55" y1="2.5" x2="-7.55" y2="0.8" width="0.127" layer="21"/>
<wire x1="-7.55" y1="0.8" x2="-7.55" y2="-5.2" width="0.127" layer="21"/>
<wire x1="7.55" y1="2.5" x2="7.55" y2="0.8" width="0.127" layer="21"/>
<wire x1="7.55" y1="0.8" x2="7.55" y2="-5.2" width="0.127" layer="21"/>
<wire x1="-7.55" y1="0.8" x2="7.55" y2="0.8" width="0.127" layer="21"/>
<text x="-6" y="-4.5" size="1.27" layer="21" font="vector">&gt;NAME</text>
<text x="-6" y="-2.5" size="1.27" layer="21" font="vector">&gt;VALUE</text>
</package>
<package name="AM2302">
<description>&lt;h1&gt;AM2302&lt;/h1&gt;
&lt;p&gt;
Package for the AM2302 wired variant of the RHT03/DHT-22 relative humidity and temperature sensor.
&lt;/p&gt;</description>
<pad name="VDD" x="-3.81" y="0" drill="0.8" shape="octagon"/>
<pad name="DATA" x="-1.27" y="0" drill="0.8"/>
<pad name="NULL" x="1.27" y="0" drill="0.8"/>
<pad name="GND" x="3.81" y="0" drill="0.8"/>
<text x="-5.08" y="-2.667" size="1.27" layer="21" font="vector">&gt;NAME</text>
<wire x1="-5.08" y1="-0.635" x2="-4.445" y2="-1.27" width="0.127" layer="21"/>
<wire x1="-4.445" y1="-1.27" x2="5.08" y2="-1.27" width="0.127" layer="21"/>
<wire x1="5.08" y1="-1.27" x2="5.08" y2="1.27" width="0.127" layer="21"/>
<wire x1="5.08" y1="1.27" x2="-5.08" y2="1.27" width="0.127" layer="21"/>
<wire x1="-5.08" y1="1.27" x2="-5.08" y2="-0.635" width="0.127" layer="21"/>
<text x="-5.08" y="1.397" size="1.27" layer="21">&gt;VALUE</text>
</package>
<package name="RHT03_DHT-22/RA">
<description>&lt;h1&gt;RHT03 / DHT-22 - RIGHT ANGLE&lt;/h1&gt;
&lt;p&gt;
Right angle package for the RHT03 / DHT-22 relative humidity and temperature sensor.
&lt;/p&gt;</description>
<pad name="VDD" x="-3.81" y="0" drill="0.8" shape="octagon"/>
<pad name="DATA" x="-1.27" y="0" drill="0.8"/>
<pad name="NULL" x="1.27" y="0" drill="0.8"/>
<pad name="GND" x="3.81" y="0" drill="0.8"/>
<wire x1="-7.55" y1="2.5" x2="7.55" y2="2.5" width="0.127" layer="21"/>
<text x="-5.08" y="5.08" size="1.27" layer="21" font="vector" rot="R90">&gt;NAME</text>
<wire x1="-7.55" y1="2.5" x2="-7.55" y2="22.5" width="0.127" layer="21"/>
<wire x1="7.55" y1="2.5" x2="7.55" y2="22.5" width="0.127" layer="21"/>
<wire x1="-7.55" y1="22.5" x2="7.55" y2="22.5" width="0.127" layer="21"/>
<wire x1="-5.25" y1="27.1" x2="5.25" y2="27.1" width="0.127" layer="21"/>
<wire x1="7.55" y1="22.5" x2="5.25" y2="27.1" width="0.127" layer="21"/>
<wire x1="-7.55" y1="22.5" x2="-5.25" y2="27.1" width="0.127" layer="21"/>
<hole x="0" y="24.5" drill="3.25"/>
<wire x1="-3.81" y1="1.27" x2="-3.81" y2="2.54" width="0.127" layer="21"/>
<wire x1="-1.27" y1="1.27" x2="-1.27" y2="2.54" width="0.127" layer="21"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="2.54" width="0.127" layer="21"/>
<wire x1="3.81" y1="1.27" x2="3.81" y2="2.54" width="0.127" layer="21"/>
<text x="0" y="12.5" size="1.27" layer="21" font="vector" rot="R90" align="center">&gt;VALUE</text>
</package>
</packages>
<symbols>
<symbol name="RHT03">
<description>&lt;h1&gt;RHT03 / DHT-22 / AM2302&lt;/h1&gt;
&lt;p&gt;
Symbol for the RHT03 / DHT-22 / AM2302 relative humidity and temperature sensor.
&lt;/p&gt;</description>
<wire x1="-5.08" y1="10.16" x2="-5.08" y2="-10.16" width="0.254" layer="94"/>
<wire x1="-5.08" y1="-10.16" x2="5.08" y2="-10.16" width="0.254" layer="94"/>
<wire x1="5.08" y1="-10.16" x2="5.08" y2="10.16" width="0.254" layer="94"/>
<wire x1="5.08" y1="10.16" x2="-5.08" y2="10.16" width="0.254" layer="94"/>
<pin name="VDD" x="-10.16" y="7.62" length="middle" direction="pwr"/>
<pin name="DATA" x="-10.16" y="2.54" length="middle"/>
<pin name="NULL" x="-10.16" y="-2.54" visible="pad" length="middle" direction="nc"/>
<pin name="GND" x="-10.16" y="-7.62" length="middle" direction="pwr"/>
<text x="-5.08" y="11.43" size="1.27" layer="94">&gt;NAME</text>
<text x="-5.08" y="-12.7" size="1.27" layer="94">&gt;VALUE</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="RHT03_DHT-22_AM2302" prefix="U" uservalue="yes">
<description>&lt;h1&gt;RHT03 / DHT-22 / AM2302&lt;/h1&gt;
&lt;p&gt;
Packages and symbol for the RHT03 / DHT-22 relative humidity and temperature sensor.&lt;br /&gt;
Note: The AM2302 is a wired version of the DHT-22 which typically requires less clearance and is thus provided as a separate package.
&lt;/p&gt;</description>
<gates>
<gate name="G$1" symbol="RHT03" x="0" y="0"/>
</gates>
<devices>
<device name="" package="RHT03_DHT-22">
<connects>
<connect gate="G$1" pin="DATA" pad="DATA"/>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="NULL" pad="NULL"/>
<connect gate="G$1" pin="VDD" pad="VDD"/>
</connects>
<technologies>
<technology name="">
<attribute name="PROD_ID" value="SEN-10167"/>
</technology>
</technologies>
</device>
<device name="WIRED" package="AM2302">
<connects>
<connect gate="G$1" pin="DATA" pad="DATA"/>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="NULL" pad="NULL"/>
<connect gate="G$1" pin="VDD" pad="VDD"/>
</connects>
<technologies>
<technology name="">
<attribute name="PROD_ID" value="SEN-10167"/>
</technology>
</technologies>
</device>
<device name="RA" package="RHT03_DHT-22/RA">
<connects>
<connect gate="G$1" pin="DATA" pad="DATA"/>
<connect gate="G$1" pin="GND" pad="GND"/>
<connect gate="G$1" pin="NULL" pad="NULL"/>
<connect gate="G$1" pin="VDD" pad="VDD"/>
</connects>
<technologies>
<technology name="">
<attribute name="PROD_ID" value="SEN-10167"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="transistor-fet">
<description>&lt;b&gt;Field Effect Transistors&lt;/b&gt;&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;&lt;p&gt;
&lt;p&gt;
Symbols changed according to IEC617&lt;p&gt; 
All types, packages and assignment to symbols and pins checked&lt;p&gt;
Package outlines partly checked&lt;p&gt;
&lt;p&gt;
JFET = junction FET&lt;p&gt;
IGBT-x = insulated gate bipolar transistor&lt;p&gt;
x=N: NPN; x=P: PNP&lt;p&gt;
IGFET-mc-nnn; (IGFET=insulated gate field effect transistor)&lt;P&gt;
m=D: depletion mode (Verdr&amp;auml;ngungstyp)&lt;p&gt;
m=E: enhancement mode (Anreicherungstyp)&lt;p&gt;
c: N=N-channel; P=P-Channel&lt;p&gt;
GDSB: gate, drain, source, bulk&lt;p&gt;
&lt;p&gt;
by R. Vogg  15.March.2002</description>
<packages>
<package name="TO92">
<description>&lt;b&gt;TO 92&lt;/b&gt;</description>
<wire x1="-2.0946" y1="-1.651" x2="-2.6549" y2="-0.254" width="0.127" layer="21" curve="-32.781"/>
<wire x1="-2.6549" y1="-0.254" x2="-0.7863" y2="2.5485" width="0.127" layer="21" curve="-78.3185"/>
<wire x1="0.7863" y1="2.5484" x2="2.0945" y2="-1.651" width="0.127" layer="21" curve="-111.1"/>
<wire x1="-2.0945" y1="-1.651" x2="2.0945" y2="-1.651" width="0.127" layer="21"/>
<wire x1="-2.2537" y1="-0.254" x2="-0.2863" y2="-0.254" width="0.127" layer="51"/>
<wire x1="-2.6549" y1="-0.254" x2="-2.2537" y2="-0.254" width="0.127" layer="21"/>
<wire x1="-0.2863" y1="-0.254" x2="0.2863" y2="-0.254" width="0.127" layer="21"/>
<wire x1="2.2537" y1="-0.254" x2="2.6549" y2="-0.254" width="0.127" layer="21"/>
<wire x1="0.2863" y1="-0.254" x2="2.2537" y2="-0.254" width="0.127" layer="51"/>
<wire x1="-0.7863" y1="2.5485" x2="0.7863" y2="2.5485" width="0.127" layer="51" curve="-34.2936"/>
<pad name="1" x="1.27" y="0" drill="0.8128" shape="octagon"/>
<pad name="2" x="0" y="1.905" drill="0.8128" shape="octagon"/>
<pad name="3" x="-1.27" y="0" drill="0.8128" shape="octagon"/>
<text x="3.175" y="0.635" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="3.175" y="-1.27" size="1.27" layer="27" ratio="10">&gt;VALUE</text>
<text x="-0.635" y="0.635" size="1.27" layer="51" ratio="10">2</text>
<text x="-2.159" y="0" size="1.27" layer="51" ratio="10">3</text>
<text x="1.143" y="0" size="1.27" layer="51" ratio="10">1</text>
</package>
</packages>
<symbols>
<symbol name="IGFET-EN-GDS">
<wire x1="-2.54" y1="-2.54" x2="-1.2192" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="0" y1="0.762" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="0" y2="-0.762" width="0.254" layer="94"/>
<wire x1="0" y1="3.683" x2="0" y2="1.397" width="0.254" layer="94"/>
<wire x1="1.905" y1="0.635" x2="0.635" y2="0" width="0.254" layer="94"/>
<wire x1="1.905" y1="-0.635" x2="0.635" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="0.635" y2="0" width="0.1524" layer="94"/>
<wire x1="0.635" y1="0" x2="2.54" y2="0" width="0.1524" layer="94"/>
<wire x1="2.54" y1="0" x2="2.54" y2="-2.54" width="0.1524" layer="94"/>
<wire x1="0" y1="-1.397" x2="0" y2="-3.683" width="0.254" layer="94"/>
<wire x1="-1.143" y1="2.54" x2="-1.143" y2="-2.54" width="0.254" layer="94"/>
<text x="-11.43" y="0" size="1.778" layer="96">&gt;VALUE</text>
<text x="-11.43" y="2.54" size="1.778" layer="95">&gt;NAME</text>
<pin name="D" x="5.08" y="2.54" visible="off" length="middle" direction="pas" rot="R180"/>
<pin name="S" x="5.08" y="-2.54" visible="off" length="middle" direction="pas" rot="R180"/>
<pin name="G" x="-5.08" y="-2.54" visible="off" length="short" direction="pas"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="BSS295" prefix="Q">
<description>&lt;b&gt;N-Channel Enhancement MOSFET&lt;/b&gt; not checked</description>
<gates>
<gate name="G$1" symbol="IGFET-EN-GDS" x="0" y="0"/>
</gates>
<devices>
<device name="" package="TO92">
<connects>
<connect gate="G$1" pin="D" pad="2"/>
<connect gate="G$1" pin="G" pad="1"/>
<connect gate="G$1" pin="S" pad="3"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="supply1">
<description>&lt;b&gt;Supply Symbols&lt;/b&gt;&lt;p&gt;
 GND, VCC, 0V, +5V, -5V, etc.&lt;p&gt;
 Please keep in mind, that these devices are necessary for the
 automatic wiring of the supply signals.&lt;p&gt;
 The pin name defined in the symbol is identical to the net which is to be wired automatically.&lt;p&gt;
 In this library the device names are the same as the pin names of the symbols, therefore the correct signal names appear next to the supply symbols in the schematic.&lt;p&gt;
 &lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
</packages>
<symbols>
<symbol name="+3V3">
<wire x1="1.27" y1="-1.905" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="-1.27" y2="-1.905" width="0.254" layer="94"/>
<text x="-2.54" y="-5.08" size="1.778" layer="96" rot="R90">&gt;VALUE</text>
<pin name="+3V3" x="0" y="-2.54" visible="off" length="short" direction="sup" rot="R90"/>
</symbol>
<symbol name="GND">
<wire x1="-1.905" y1="0" x2="1.905" y2="0" width="0.254" layer="94"/>
<text x="-2.54" y="-2.54" size="1.778" layer="96">&gt;VALUE</text>
<pin name="GND" x="0" y="2.54" visible="off" length="short" direction="sup" rot="R270"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="+3V3" prefix="+3V3">
<description>&lt;b&gt;SUPPLY SYMBOL&lt;/b&gt;</description>
<gates>
<gate name="G$1" symbol="+3V3" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="GND" prefix="GND">
<description>&lt;b&gt;SUPPLY SYMBOL&lt;/b&gt;</description>
<gates>
<gate name="1" symbol="GND" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="DHT22" library="RHT03_DHT-22_AM2302" deviceset="RHT03_DHT-22_AM2302" device=""/>
<part name="Q1" library="transistor-fet" deviceset="BSS295" device=""/>
<part name="+3V1" library="supply1" deviceset="+3V3" device=""/>
<part name="GND1" library="supply1" deviceset="GND" device=""/>
<part name="GND2" library="supply1" deviceset="GND" device=""/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="DHT22" gate="G$1" x="58.42" y="25.4"/>
<instance part="Q1" gate="G$1" x="30.48" y="17.78" smashed="yes"/>
<instance part="+3V1" gate="G$1" x="43.18" y="43.18"/>
<instance part="GND1" gate="1" x="43.18" y="10.16"/>
<instance part="GND2" gate="1" x="35.56" y="10.16"/>
</instances>
<busses>
</busses>
<nets>
<net name="+3V3" class="0">
<segment>
<pinref part="DHT22" gate="G$1" pin="VDD"/>
<pinref part="+3V1" gate="G$1" pin="+3V3"/>
<wire x1="48.26" y1="33.02" x2="43.18" y2="33.02" width="0.1524" layer="91"/>
<wire x1="43.18" y1="33.02" x2="43.18" y2="40.64" width="0.1524" layer="91"/>
</segment>
</net>
<net name="GND" class="0">
<segment>
<pinref part="DHT22" gate="G$1" pin="GND"/>
<pinref part="GND1" gate="1" pin="GND"/>
<wire x1="48.26" y1="17.78" x2="43.18" y2="17.78" width="0.1524" layer="91"/>
<wire x1="43.18" y1="17.78" x2="43.18" y2="12.7" width="0.1524" layer="91"/>
</segment>
<segment>
<pinref part="Q1" gate="G$1" pin="S"/>
<pinref part="GND2" gate="1" pin="GND"/>
<wire x1="35.56" y1="15.24" x2="35.56" y2="12.7" width="0.1524" layer="91"/>
</segment>
</net>
<net name="GPIO4" class="0">
<segment>
<pinref part="Q1" gate="G$1" pin="D"/>
<wire x1="35.56" y1="20.32" x2="35.56" y2="27.94" width="0.1524" layer="91"/>
<pinref part="DHT22" gate="G$1" pin="DATA"/>
<wire x1="35.56" y1="27.94" x2="48.26" y2="27.94" width="0.1524" layer="91"/>
<wire x1="35.56" y1="27.94" x2="15.24" y2="27.94" width="0.1524" layer="91"/>
<junction x="35.56" y="27.94"/>
<label x="15.24" y="27.94" size="1.778" layer="95"/>
</segment>
</net>
<net name="GPIO5" class="0">
<segment>
<pinref part="Q1" gate="G$1" pin="G"/>
<wire x1="25.4" y1="15.24" x2="15.24" y2="15.24" width="0.1524" layer="91"/>
<label x="15.24" y="15.24" size="1.778" layer="95"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
