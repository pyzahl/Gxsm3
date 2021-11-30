<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.6.2">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="2" name="Route2" color="1" fill="3" visible="no" active="no"/>
<layer number="3" name="Route3" color="4" fill="3" visible="no" active="no"/>
<layer number="4" name="Route4" color="1" fill="4" visible="no" active="no"/>
<layer number="5" name="Route5" color="4" fill="4" visible="no" active="no"/>
<layer number="6" name="Route6" color="1" fill="8" visible="no" active="no"/>
<layer number="7" name="Route7" color="4" fill="8" visible="no" active="no"/>
<layer number="8" name="Route8" color="1" fill="2" visible="no" active="no"/>
<layer number="9" name="Route9" color="4" fill="2" visible="no" active="no"/>
<layer number="10" name="Route10" color="1" fill="7" visible="no" active="no"/>
<layer number="11" name="Route11" color="4" fill="7" visible="no" active="no"/>
<layer number="12" name="Route12" color="1" fill="5" visible="no" active="no"/>
<layer number="13" name="Route13" color="4" fill="5" visible="no" active="no"/>
<layer number="14" name="Route14" color="1" fill="6" visible="no" active="no"/>
<layer number="15" name="Route15" color="4" fill="6" visible="no" active="no"/>
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
<layer number="88" name="SimResults" color="9" fill="1" visible="yes" active="yes"/>
<layer number="89" name="SimProbes" color="9" fill="1" visible="yes" active="yes"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
<layer number="99" name="SpiceOrder" color="5" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="con-lsta" urn="urn:adsk.eagle:library:161">
<description>&lt;b&gt;Female Headers etc.&lt;/b&gt;&lt;p&gt;
Naming:&lt;p&gt;
FE = female&lt;p&gt;
# contacts - # rows&lt;p&gt;
W = angled&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="FE13-2" urn="urn:adsk.eagle:footprint:8105/1" library_version="1">
<description>&lt;b&gt;FEMALE HEADER&lt;/b&gt;</description>
<wire x1="-16.51" y1="-2.413" x2="-16.51" y2="2.413" width="0.1524" layer="21"/>
<wire x1="-16.51" y1="2.413" x2="-15.875" y2="3.048" width="0.1524" layer="21" curve="-90"/>
<wire x1="-16.51" y1="-2.413" x2="-15.875" y2="-3.048" width="0.1524" layer="21" curve="90"/>
<wire x1="-15.875" y1="3.048" x2="15.875" y2="3.048" width="0.1524" layer="21"/>
<wire x1="16.51" y1="-2.413" x2="16.51" y2="2.413" width="0.1524" layer="21"/>
<wire x1="-15.875" y1="-3.048" x2="15.875" y2="-3.048" width="0.1524" layer="21"/>
<wire x1="15.875" y1="3.048" x2="16.51" y2="2.413" width="0.1524" layer="21" curve="-90"/>
<wire x1="15.875" y1="-3.048" x2="16.51" y2="-2.413" width="0.1524" layer="21" curve="90"/>
<circle x="-15.24" y="-1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="-15.24" y="1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="-12.7" y="-1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="-12.7" y="1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="-10.16" y="-1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="-10.16" y="1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="-7.62" y="-1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="-7.62" y="1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="-5.08" y="-1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="-5.08" y="1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="-15.24" y="-1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="-15.24" y="1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="-12.7" y="1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="-10.16" y="1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="-7.62" y="1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="-5.08" y="1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="-12.7" y="-1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="-10.16" y="-1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="-7.62" y="-1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="-5.08" y="-1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="-2.54" y="-1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="-2.54" y="1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="0" y="-1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="0" y="1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="2.54" y="-1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="2.54" y="1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="5.08" y="-1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="5.08" y="1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="7.62" y="-1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="7.62" y="1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="-2.54" y="-1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="-2.54" y="1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="0" y="1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="2.54" y="1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="5.08" y="1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="7.62" y="1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="0" y="-1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="2.54" y="-1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="5.08" y="-1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="7.62" y="-1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="10.16" y="-1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="10.16" y="1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="12.7" y="-1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="12.7" y="1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="15.24" y="-1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="15.24" y="1.27" radius="0.127" width="0.4064" layer="51"/>
<circle x="10.16" y="-1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="10.16" y="1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="12.7" y="1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="15.24" y="1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="12.7" y="-1.27" radius="0.889" width="0.1524" layer="51"/>
<circle x="15.24" y="-1.27" radius="0.889" width="0.1524" layer="51"/>
<pad name="1" x="-15.24" y="1.27" drill="0.9144"/>
<pad name="2" x="-15.24" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="3" x="-12.7" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="4" x="-12.7" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="5" x="-10.16" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="6" x="-10.16" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="7" x="-7.62" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="8" x="-7.62" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="9" x="-5.08" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="10" x="-5.08" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="11" x="-2.54" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="12" x="-2.54" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="13" x="0" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="14" x="0" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="15" x="2.54" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="16" x="2.54" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="17" x="5.08" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="18" x="5.08" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="19" x="7.62" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="20" x="7.62" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="21" x="10.16" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="22" x="10.16" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="23" x="12.7" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="24" x="12.7" y="-1.27" drill="0.9144" shape="octagon"/>
<pad name="25" x="15.24" y="1.27" drill="0.9144" shape="octagon"/>
<pad name="26" x="15.24" y="-1.27" drill="0.9144" shape="octagon"/>
<text x="-10.16" y="3.429" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="-15.621" y="3.429" size="1.27" layer="21" ratio="10">1</text>
<text x="14.351" y="-4.699" size="1.27" layer="21" ratio="10">26</text>
<text x="-16.51" y="-4.699" size="1.27" layer="27" ratio="10">&gt;VALUE</text>
</package>
</packages>
<packages3d>
<package3d name="FE13-2" urn="urn:adsk.eagle:package:8172/1" type="box" library_version="1">
<description>FEMALE HEADER</description>
<packageinstances>
<packageinstance name="FE13-2"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="FE13-2" urn="urn:adsk.eagle:symbol:8104/1" library_version="1">
<wire x1="3.81" y1="-17.78" x2="-3.81" y2="-17.78" width="0.4064" layer="94"/>
<wire x1="-1.905" y1="-10.795" x2="-1.905" y2="-9.525" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="-1.905" y1="-13.335" x2="-1.905" y2="-12.065" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="-1.905" y1="-15.875" x2="-1.905" y2="-14.605" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="1.905" y1="-9.525" x2="1.905" y2="-10.795" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="1.905" y1="-12.065" x2="1.905" y2="-13.335" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="1.905" y1="-14.605" x2="1.905" y2="-15.875" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="-1.905" y1="-5.715" x2="-1.905" y2="-4.445" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="-1.905" y1="-8.255" x2="-1.905" y2="-6.985" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="1.905" y1="-4.445" x2="1.905" y2="-5.715" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="1.905" y1="-6.985" x2="1.905" y2="-8.255" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="-1.905" y1="1.905" x2="-1.905" y2="3.175" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="-1.905" y1="-0.635" x2="-1.905" y2="0.635" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="-1.905" y1="-3.175" x2="-1.905" y2="-1.905" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="1.905" y1="3.175" x2="1.905" y2="1.905" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="1.905" y1="0.635" x2="1.905" y2="-0.635" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="1.905" y1="-1.905" x2="1.905" y2="-3.175" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="-1.905" y1="6.985" x2="-1.905" y2="8.255" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="-1.905" y1="4.445" x2="-1.905" y2="5.715" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="1.905" y1="8.255" x2="1.905" y2="6.985" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="1.905" y1="5.715" x2="1.905" y2="4.445" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="-1.905" y1="14.605" x2="-1.905" y2="15.875" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="-1.905" y1="12.065" x2="-1.905" y2="13.335" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="-1.905" y1="9.525" x2="-1.905" y2="10.795" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="1.905" y1="15.875" x2="1.905" y2="14.605" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="1.905" y1="13.335" x2="1.905" y2="12.065" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="1.905" y1="10.795" x2="1.905" y2="9.525" width="0.254" layer="94" curve="-180" cap="flat"/>
<wire x1="-3.81" y1="17.78" x2="-3.81" y2="-17.78" width="0.4064" layer="94"/>
<wire x1="3.81" y1="-17.78" x2="3.81" y2="17.78" width="0.4064" layer="94"/>
<wire x1="-3.81" y1="17.78" x2="3.81" y2="17.78" width="0.4064" layer="94"/>
<text x="-3.81" y="-20.32" size="1.778" layer="96">&gt;VALUE</text>
<text x="-3.81" y="18.542" size="1.778" layer="95">&gt;NAME</text>
<pin name="1" x="-7.62" y="-15.24" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="3" x="-7.62" y="-12.7" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="5" x="-7.62" y="-10.16" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="2" x="7.62" y="-15.24" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="4" x="7.62" y="-12.7" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="6" x="7.62" y="-10.16" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="7" x="-7.62" y="-7.62" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="9" x="-7.62" y="-5.08" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="8" x="7.62" y="-7.62" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="10" x="7.62" y="-5.08" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="11" x="-7.62" y="-2.54" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="13" x="-7.62" y="0" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="15" x="-7.62" y="2.54" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="12" x="7.62" y="-2.54" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="14" x="7.62" y="0" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="16" x="7.62" y="2.54" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="17" x="-7.62" y="5.08" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="19" x="-7.62" y="7.62" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="18" x="7.62" y="5.08" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="20" x="7.62" y="7.62" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="21" x="-7.62" y="10.16" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="23" x="-7.62" y="12.7" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="25" x="-7.62" y="15.24" visible="pad" length="middle" direction="pas" swaplevel="1"/>
<pin name="22" x="7.62" y="10.16" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="24" x="7.62" y="12.7" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
<pin name="26" x="7.62" y="15.24" visible="pad" length="middle" direction="pas" swaplevel="1" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="FE13-2" urn="urn:adsk.eagle:component:8212/1" prefix="SV" uservalue="yes" library_version="1">
<description>&lt;b&gt;FEMALE HEADER&lt;/b&gt;</description>
<gates>
<gate name="G$1" symbol="FE13-2" x="0" y="0"/>
</gates>
<devices>
<device name="" package="FE13-2">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="10" pad="10"/>
<connect gate="G$1" pin="11" pad="11"/>
<connect gate="G$1" pin="12" pad="12"/>
<connect gate="G$1" pin="13" pad="13"/>
<connect gate="G$1" pin="14" pad="14"/>
<connect gate="G$1" pin="15" pad="15"/>
<connect gate="G$1" pin="16" pad="16"/>
<connect gate="G$1" pin="17" pad="17"/>
<connect gate="G$1" pin="18" pad="18"/>
<connect gate="G$1" pin="19" pad="19"/>
<connect gate="G$1" pin="2" pad="2"/>
<connect gate="G$1" pin="20" pad="20"/>
<connect gate="G$1" pin="21" pad="21"/>
<connect gate="G$1" pin="22" pad="22"/>
<connect gate="G$1" pin="23" pad="23"/>
<connect gate="G$1" pin="24" pad="24"/>
<connect gate="G$1" pin="25" pad="25"/>
<connect gate="G$1" pin="26" pad="26"/>
<connect gate="G$1" pin="3" pad="3"/>
<connect gate="G$1" pin="4" pad="4"/>
<connect gate="G$1" pin="5" pad="5"/>
<connect gate="G$1" pin="6" pad="6"/>
<connect gate="G$1" pin="7" pad="7"/>
<connect gate="G$1" pin="8" pad="8"/>
<connect gate="G$1" pin="9" pad="9"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:8172/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="" constant="no"/>
<attribute name="OC_FARNELL" value="unknown" constant="no"/>
<attribute name="OC_NEWARK" value="unknown" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="AD5791ARUZ">
<description>&lt;Analog Devices AD5791ARUZ, 20 bit Serial DAC, 20-Pin TSSOP&lt;/b&gt;&lt;p&gt;
&lt;author&gt;Created by SamacSys&lt;/author&gt;</description>
<packages>
<package name="SOP65P640X120-20N">
<description>&lt;b&gt;SOP65P640X120-20N&lt;/b&gt;&lt;br&gt;
</description>
<smd name="1" x="-2.8194" y="2.921" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="2" x="-2.8194" y="2.286" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="3" x="-2.8194" y="1.6256" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="4" x="-2.8194" y="0.9652" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="5" x="-2.8194" y="0.3302" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="6" x="-2.8194" y="-0.3302" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="7" x="-2.8194" y="-0.9652" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="8" x="-2.8194" y="-1.6256" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="9" x="-2.8194" y="-2.286" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="10" x="-2.8194" y="-2.921" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="11" x="2.8194" y="-2.921" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="12" x="2.8194" y="-2.286" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="13" x="2.8194" y="-1.6256" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="14" x="2.8194" y="-0.9652" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="15" x="2.8194" y="-0.3302" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="16" x="2.8194" y="0.3302" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="17" x="2.8194" y="0.9652" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="18" x="2.8194" y="1.6256" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="19" x="2.8194" y="2.286" dx="1.4732" dy="0.3556" layer="1"/>
<smd name="20" x="2.8194" y="2.921" dx="1.4732" dy="0.3556" layer="1"/>
<text x="-3.4544" y="5.715" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="-3.4544" y="-7.62" size="1.27" layer="27" align="center">&gt;VALUE</text>
<wire x1="-1.8288" y1="-3.302" x2="1.8288" y2="-3.302" width="0.1524" layer="21"/>
<wire x1="1.8288" y1="3.302" x2="0.3048" y2="3.302" width="0.1524" layer="21"/>
<wire x1="0.3048" y1="3.302" x2="-0.3048" y2="3.302" width="0.1524" layer="21"/>
<wire x1="-0.3048" y1="3.302" x2="-1.8288" y2="3.302" width="0.1524" layer="21"/>
<wire x1="0.3048" y1="3.302" x2="-0.3048" y2="3.302" width="0.1524" layer="21" curve="-180"/>
<wire x1="-2.2352" y1="2.7686" x2="-2.2352" y2="3.0734" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="3.0734" x2="-3.2004" y2="3.0734" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="3.0734" x2="-3.2004" y2="2.7686" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="2.7686" x2="-2.2352" y2="2.7686" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="2.1336" x2="-2.2352" y2="2.4384" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="2.4384" x2="-3.2004" y2="2.4384" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="2.4384" x2="-3.2004" y2="2.1336" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="2.1336" x2="-2.2352" y2="2.1336" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="1.4732" x2="-2.2352" y2="1.778" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="1.778" x2="-3.2004" y2="1.778" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="1.778" x2="-3.2004" y2="1.4732" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="1.4732" x2="-2.2352" y2="1.4732" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="0.8128" x2="-2.2352" y2="1.1176" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="1.1176" x2="-3.2004" y2="1.1176" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="1.1176" x2="-3.2004" y2="0.8128" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="0.8128" x2="-2.2352" y2="0.8128" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="0.1778" x2="-2.2352" y2="0.4826" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="0.4826" x2="-3.2004" y2="0.4826" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="0.4826" x2="-3.2004" y2="0.1778" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="0.1778" x2="-2.2352" y2="0.1778" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="-0.4826" x2="-2.2352" y2="-0.1778" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="-0.1778" x2="-3.2004" y2="-0.1778" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="-0.1778" x2="-3.2004" y2="-0.4826" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="-0.4826" x2="-2.2352" y2="-0.4826" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="-1.1176" x2="-2.2352" y2="-0.8128" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="-0.8128" x2="-3.2004" y2="-0.8128" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="-0.8128" x2="-3.2004" y2="-1.1176" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="-1.1176" x2="-2.2352" y2="-1.1176" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="-1.778" x2="-2.2352" y2="-1.4732" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="-1.4732" x2="-3.2004" y2="-1.4732" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="-1.4732" x2="-3.2004" y2="-1.778" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="-1.778" x2="-2.2352" y2="-1.778" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="-2.4384" x2="-2.2352" y2="-2.1336" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="-2.1336" x2="-3.2004" y2="-2.1336" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="-2.1336" x2="-3.2004" y2="-2.4384" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="-2.4384" x2="-2.2352" y2="-2.4384" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="-3.0734" x2="-2.2352" y2="-2.7686" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="-2.7686" x2="-3.2004" y2="-2.7686" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="-2.7686" x2="-3.2004" y2="-3.0734" width="0.1524" layer="51"/>
<wire x1="-3.2004" y1="-3.0734" x2="-2.2352" y2="-3.0734" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="-2.7686" x2="2.2352" y2="-3.0734" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="-3.0734" x2="3.2004" y2="-3.0734" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="-3.0734" x2="3.2004" y2="-2.7686" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="-2.7686" x2="2.2352" y2="-2.7686" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="-2.1336" x2="2.2352" y2="-2.4384" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="-2.4384" x2="3.2004" y2="-2.4384" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="-2.4384" x2="3.2004" y2="-2.1336" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="-2.1336" x2="2.2352" y2="-2.1336" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="-1.4732" x2="2.2352" y2="-1.778" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="-1.778" x2="3.2004" y2="-1.778" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="-1.778" x2="3.2004" y2="-1.4732" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="-1.4732" x2="2.2352" y2="-1.4732" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="-0.8128" x2="2.2352" y2="-1.1176" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="-1.1176" x2="3.2004" y2="-1.1176" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="-1.1176" x2="3.2004" y2="-0.8128" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="-0.8128" x2="2.2352" y2="-0.8128" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="-0.1778" x2="2.2352" y2="-0.4826" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="-0.4826" x2="3.2004" y2="-0.4826" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="-0.4826" x2="3.2004" y2="-0.1778" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="-0.1778" x2="2.2352" y2="-0.1778" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="0.4826" x2="2.2352" y2="0.1778" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="0.1778" x2="3.2004" y2="0.1778" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="0.1778" x2="3.2004" y2="0.4826" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="0.4826" x2="2.2352" y2="0.4826" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="1.1176" x2="2.2352" y2="0.8128" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="0.8128" x2="3.2004" y2="0.8128" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="0.8128" x2="3.2004" y2="1.1176" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="1.1176" x2="2.2352" y2="1.1176" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="1.778" x2="2.2352" y2="1.4732" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="1.4732" x2="3.2004" y2="1.4732" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="1.4732" x2="3.2004" y2="1.778" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="1.778" x2="2.2352" y2="1.778" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="2.4384" x2="2.2352" y2="2.1336" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="2.1336" x2="3.2004" y2="2.1336" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="2.1336" x2="3.2004" y2="2.4384" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="2.4384" x2="2.2352" y2="2.4384" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="3.0734" x2="2.2352" y2="2.7686" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="2.7686" x2="3.2004" y2="2.7686" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="2.7686" x2="3.2004" y2="3.0734" width="0.1524" layer="51"/>
<wire x1="3.2004" y1="3.0734" x2="2.2352" y2="3.0734" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="-3.302" x2="2.2352" y2="-3.302" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="-3.302" x2="2.2352" y2="3.302" width="0.1524" layer="51"/>
<wire x1="2.2352" y1="3.302" x2="0.3048" y2="3.302" width="0.1524" layer="51"/>
<wire x1="0.3048" y1="3.302" x2="-0.3048" y2="3.302" width="0.1524" layer="51"/>
<wire x1="-0.3048" y1="3.302" x2="-2.2352" y2="3.302" width="0.1524" layer="51"/>
<wire x1="-2.2352" y1="3.302" x2="-2.2352" y2="-3.302" width="0.1524" layer="51"/>
<wire x1="0.3048" y1="3.302" x2="-0.3048" y2="3.302" width="0.1524" layer="51" curve="-180"/>
</package>
</packages>
<symbols>
<symbol name="AD5791ARUZ">
<wire x1="5.08" y1="2.54" x2="27.94" y2="2.54" width="0.254" layer="94"/>
<wire x1="27.94" y1="-25.4" x2="27.94" y2="2.54" width="0.254" layer="94"/>
<wire x1="27.94" y1="-25.4" x2="5.08" y2="-25.4" width="0.254" layer="94"/>
<wire x1="5.08" y1="2.54" x2="5.08" y2="-25.4" width="0.254" layer="94"/>
<text x="29.21" y="7.62" size="1.778" layer="95" align="center-left">&gt;NAME</text>
<text x="29.21" y="5.08" size="1.778" layer="96" align="center-left">&gt;VALUE</text>
<pin name="INV" x="0" y="0" length="middle"/>
<pin name="VOUT" x="0" y="-2.54" length="middle"/>
<pin name="VREFPS" x="0" y="-5.08" length="middle"/>
<pin name="VREFPF" x="0" y="-7.62" length="middle"/>
<pin name="VDD" x="0" y="-10.16" length="middle"/>
<pin name="RESET!" x="0" y="-12.7" length="middle"/>
<pin name="CLR!" x="0" y="-15.24" length="middle"/>
<pin name="LDAC!" x="0" y="-17.78" length="middle"/>
<pin name="VCC" x="0" y="-20.32" length="middle"/>
<pin name="IOVCC" x="0" y="-22.86" length="middle"/>
<pin name="SDO" x="33.02" y="-22.86" length="middle" rot="R180"/>
<pin name="SDIN" x="33.02" y="-20.32" length="middle" rot="R180"/>
<pin name="SCLK" x="33.02" y="-17.78" length="middle" rot="R180"/>
<pin name="SYNC!" x="33.02" y="-15.24" length="middle" rot="R180"/>
<pin name="DGND" x="33.02" y="-12.7" length="middle" rot="R180"/>
<pin name="VREFNF" x="33.02" y="-10.16" length="middle" rot="R180"/>
<pin name="VREFNS" x="33.02" y="-7.62" length="middle" rot="R180"/>
<pin name="VSS" x="33.02" y="-5.08" length="middle" rot="R180"/>
<pin name="AGND" x="33.02" y="-2.54" length="middle" rot="R180"/>
<pin name="RFB" x="33.02" y="0" length="middle" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="AD5791ARUZ" prefix="IC">
<description>&lt;b&gt;Analog Devices AD5791ARUZ, 20 bit Serial DAC, 20-Pin TSSOP&lt;/b&gt;&lt;p&gt;
Source: &lt;a href=""&gt; Datasheet &lt;/a&gt;</description>
<gates>
<gate name="G$1" symbol="AD5791ARUZ" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOP65P640X120-20N">
<connects>
<connect gate="G$1" pin="AGND" pad="19"/>
<connect gate="G$1" pin="CLR!" pad="7"/>
<connect gate="G$1" pin="DGND" pad="15"/>
<connect gate="G$1" pin="INV" pad="1"/>
<connect gate="G$1" pin="IOVCC" pad="10"/>
<connect gate="G$1" pin="LDAC!" pad="8"/>
<connect gate="G$1" pin="RESET!" pad="6"/>
<connect gate="G$1" pin="RFB" pad="20"/>
<connect gate="G$1" pin="SCLK" pad="13"/>
<connect gate="G$1" pin="SDIN" pad="12"/>
<connect gate="G$1" pin="SDO" pad="11"/>
<connect gate="G$1" pin="SYNC!" pad="14"/>
<connect gate="G$1" pin="VCC" pad="9"/>
<connect gate="G$1" pin="VDD" pad="5"/>
<connect gate="G$1" pin="VOUT" pad="2"/>
<connect gate="G$1" pin="VREFNF" pad="16"/>
<connect gate="G$1" pin="VREFNS" pad="17"/>
<connect gate="G$1" pin="VREFPF" pad="4"/>
<connect gate="G$1" pin="VREFPS" pad="3"/>
<connect gate="G$1" pin="VSS" pad="18"/>
</connects>
<technologies>
<technology name="">
<attribute name="ARROW_PART_NUMBER" value="AD5791ARUZ" constant="no"/>
<attribute name="ARROW_PRICE-STOCK" value="https://www.arrow.com/en/products/ad5791aruz/maxim-integrated?region=nac" constant="no"/>
<attribute name="DESCRIPTION" value="Analog Devices AD5791ARUZ, 20 bit Serial DAC, 20-Pin TSSOP" constant="no"/>
<attribute name="HEIGHT" value="mm" constant="no"/>
<attribute name="MANUFACTURER_NAME" value="Analog Devices" constant="no"/>
<attribute name="MANUFACTURER_PART_NUMBER" value="AD5791ARUZ" constant="no"/>
<attribute name="MOUSER_PART_NUMBER" value="584-AD5791ARUZ" constant="no"/>
<attribute name="MOUSER_PRICE-STOCK" value="https://www.mouser.co.uk/ProductDetail/Analog-Devices/AD5791ARUZ?qs=NmRFExCfTkFFzkQEIb8Zgg%3D%3D" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="AD8676BRZ">
<description>&lt;AD8676BRZ, Dual Operational Amplifier 10MHz Rail-Rail, 8-Pin SOIC&lt;/b&gt;&lt;p&gt;
&lt;author&gt;Created by SamacSys&lt;/author&gt;</description>
<packages>
<package name="SOIC127P600X175-8N">
<description>&lt;b&gt;R-08&lt;/b&gt;&lt;br&gt;
</description>
<smd name="1" x="-2.712" y="1.905" dx="1.525" dy="0.65" layer="1"/>
<smd name="2" x="-2.712" y="0.635" dx="1.525" dy="0.65" layer="1"/>
<smd name="3" x="-2.712" y="-0.635" dx="1.525" dy="0.65" layer="1"/>
<smd name="4" x="-2.712" y="-1.905" dx="1.525" dy="0.65" layer="1"/>
<smd name="5" x="2.712" y="-1.905" dx="1.525" dy="0.65" layer="1"/>
<smd name="6" x="2.712" y="-0.635" dx="1.525" dy="0.65" layer="1"/>
<smd name="7" x="2.712" y="0.635" dx="1.525" dy="0.65" layer="1"/>
<smd name="8" x="2.712" y="1.905" dx="1.525" dy="0.65" layer="1"/>
<text x="0" y="0" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="0" y="0" size="1.27" layer="27" align="center">&gt;VALUE</text>
<wire x1="-3.725" y1="2.75" x2="3.725" y2="2.75" width="0.05" layer="51"/>
<wire x1="3.725" y1="2.75" x2="3.725" y2="-2.75" width="0.05" layer="51"/>
<wire x1="3.725" y1="-2.75" x2="-3.725" y2="-2.75" width="0.05" layer="51"/>
<wire x1="-3.725" y1="-2.75" x2="-3.725" y2="2.75" width="0.05" layer="51"/>
<wire x1="-1.95" y1="2.45" x2="1.95" y2="2.45" width="0.1" layer="51"/>
<wire x1="1.95" y1="2.45" x2="1.95" y2="-2.45" width="0.1" layer="51"/>
<wire x1="1.95" y1="-2.45" x2="-1.95" y2="-2.45" width="0.1" layer="51"/>
<wire x1="-1.95" y1="-2.45" x2="-1.95" y2="2.45" width="0.1" layer="51"/>
<wire x1="-1.95" y1="1.18" x2="-0.68" y2="2.45" width="0.1" layer="51"/>
<wire x1="-1.6" y1="2.45" x2="1.6" y2="2.45" width="0.2" layer="21"/>
<wire x1="1.6" y1="2.45" x2="1.6" y2="-2.45" width="0.2" layer="21"/>
<wire x1="1.6" y1="-2.45" x2="-1.6" y2="-2.45" width="0.2" layer="21"/>
<wire x1="-1.6" y1="-2.45" x2="-1.6" y2="2.45" width="0.2" layer="21"/>
<wire x1="-3.475" y1="2.58" x2="-1.95" y2="2.58" width="0.2" layer="21"/>
</package>
</packages>
<symbols>
<symbol name="AD8676BRZ">
<wire x1="5.08" y1="2.54" x2="27.94" y2="2.54" width="0.254" layer="94"/>
<wire x1="27.94" y1="-10.16" x2="27.94" y2="2.54" width="0.254" layer="94"/>
<wire x1="27.94" y1="-10.16" x2="5.08" y2="-10.16" width="0.254" layer="94"/>
<wire x1="5.08" y1="2.54" x2="5.08" y2="-10.16" width="0.254" layer="94"/>
<text x="29.21" y="7.62" size="1.778" layer="95" align="center-left">&gt;NAME</text>
<text x="29.21" y="5.08" size="1.778" layer="96" align="center-left">&gt;VALUE</text>
<pin name="OUT_A" x="0" y="0" length="middle"/>
<pin name="-IN_A" x="0" y="-2.54" length="middle"/>
<pin name="+_IN_A" x="0" y="-5.08" length="middle"/>
<pin name="V-" x="0" y="-7.62" length="middle"/>
<pin name="V+" x="33.02" y="0" length="middle" rot="R180"/>
<pin name="OUT_B" x="33.02" y="-2.54" length="middle" rot="R180"/>
<pin name="-_IN_B" x="33.02" y="-5.08" length="middle" rot="R180"/>
<pin name="+_IN_B" x="33.02" y="-7.62" length="middle" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="AD8676BRZ" prefix="IC">
<description>&lt;b&gt;AD8676BRZ, Dual Operational Amplifier 10MHz Rail-Rail, 8-Pin SOIC&lt;/b&gt;&lt;p&gt;
Source: &lt;a href="http://www.analog.com/static/imported-files/data_sheets/AD8676.pdf"&gt; Datasheet &lt;/a&gt;</description>
<gates>
<gate name="G$1" symbol="AD8676BRZ" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOIC127P600X175-8N">
<connects>
<connect gate="G$1" pin="+_IN_A" pad="3"/>
<connect gate="G$1" pin="+_IN_B" pad="5"/>
<connect gate="G$1" pin="-IN_A" pad="2"/>
<connect gate="G$1" pin="-_IN_B" pad="6"/>
<connect gate="G$1" pin="OUT_A" pad="1"/>
<connect gate="G$1" pin="OUT_B" pad="7"/>
<connect gate="G$1" pin="V+" pad="8"/>
<connect gate="G$1" pin="V-" pad="4"/>
</connects>
<technologies>
<technology name="">
<attribute name="ARROW_PART_NUMBER" value="AD8676BRZ" constant="no"/>
<attribute name="ARROW_PRICE-STOCK" value="https://www.arrow.com/en/products/ad8676brz/analog-devices" constant="no"/>
<attribute name="DESCRIPTION" value="AD8676BRZ, Dual Operational Amplifier 10MHz Rail-Rail, 8-Pin SOIC" constant="no"/>
<attribute name="HEIGHT" value="1.75mm" constant="no"/>
<attribute name="MANUFACTURER_NAME" value="Analog Devices" constant="no"/>
<attribute name="MANUFACTURER_PART_NUMBER" value="AD8676BRZ" constant="no"/>
<attribute name="MOUSER_PART_NUMBER" value="584-AD8676BRZ" constant="no"/>
<attribute name="MOUSER_PRICE-STOCK" value="https://www.mouser.co.uk/ProductDetail/Analog-Devices/AD8676BRZ?qs=%2FtpEQrCGXCywkTTL2opozg%3D%3D" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="AD8675ARZ">
<description>&lt;Precision Amplifiers 2.5nV R-R out Precision BP AMP&lt;/b&gt;&lt;p&gt;
&lt;author&gt;Created by SamacSys&lt;/author&gt;</description>
<packages>
<package name="SOIC127P600X175-8N">
<description>&lt;b&gt;SOIC N&lt;/b&gt;&lt;br&gt;
</description>
<smd name="1" x="-2.712" y="1.905" dx="1.525" dy="0.65" layer="1"/>
<smd name="2" x="-2.712" y="0.635" dx="1.525" dy="0.65" layer="1"/>
<smd name="3" x="-2.712" y="-0.635" dx="1.525" dy="0.65" layer="1"/>
<smd name="4" x="-2.712" y="-1.905" dx="1.525" dy="0.65" layer="1"/>
<smd name="5" x="2.712" y="-1.905" dx="1.525" dy="0.65" layer="1"/>
<smd name="6" x="2.712" y="-0.635" dx="1.525" dy="0.65" layer="1"/>
<smd name="7" x="2.712" y="0.635" dx="1.525" dy="0.65" layer="1"/>
<smd name="8" x="2.712" y="1.905" dx="1.525" dy="0.65" layer="1"/>
<text x="0" y="0" size="1.27" layer="25" align="center">&gt;NAME</text>
<text x="0" y="0" size="1.27" layer="27" align="center">&gt;VALUE</text>
<wire x1="-3.725" y1="2.75" x2="3.725" y2="2.75" width="0.05" layer="51"/>
<wire x1="3.725" y1="2.75" x2="3.725" y2="-2.75" width="0.05" layer="51"/>
<wire x1="3.725" y1="-2.75" x2="-3.725" y2="-2.75" width="0.05" layer="51"/>
<wire x1="-3.725" y1="-2.75" x2="-3.725" y2="2.75" width="0.05" layer="51"/>
<wire x1="-1.95" y1="2.45" x2="1.95" y2="2.45" width="0.1" layer="51"/>
<wire x1="1.95" y1="2.45" x2="1.95" y2="-2.45" width="0.1" layer="51"/>
<wire x1="1.95" y1="-2.45" x2="-1.95" y2="-2.45" width="0.1" layer="51"/>
<wire x1="-1.95" y1="-2.45" x2="-1.95" y2="2.45" width="0.1" layer="51"/>
<wire x1="-1.95" y1="1.18" x2="-0.68" y2="2.45" width="0.1" layer="51"/>
<wire x1="-1.6" y1="2.45" x2="1.6" y2="2.45" width="0.2" layer="21"/>
<wire x1="1.6" y1="2.45" x2="1.6" y2="-2.45" width="0.2" layer="21"/>
<wire x1="1.6" y1="-2.45" x2="-1.6" y2="-2.45" width="0.2" layer="21"/>
<wire x1="-1.6" y1="-2.45" x2="-1.6" y2="2.45" width="0.2" layer="21"/>
<wire x1="-3.475" y1="2.58" x2="-1.95" y2="2.58" width="0.2" layer="21"/>
</package>
</packages>
<symbols>
<symbol name="AD8675ARZ">
<wire x1="5.08" y1="2.54" x2="27.94" y2="2.54" width="0.254" layer="94"/>
<wire x1="27.94" y1="-10.16" x2="27.94" y2="2.54" width="0.254" layer="94"/>
<wire x1="27.94" y1="-10.16" x2="5.08" y2="-10.16" width="0.254" layer="94"/>
<wire x1="5.08" y1="2.54" x2="5.08" y2="-10.16" width="0.254" layer="94"/>
<text x="29.21" y="7.62" size="1.778" layer="95" align="center-left">&gt;NAME</text>
<text x="29.21" y="5.08" size="1.778" layer="96" align="center-left">&gt;VALUE</text>
<pin name="NULL_1" x="0" y="0" length="middle"/>
<pin name="-IN" x="0" y="-2.54" length="middle"/>
<pin name="+IN" x="0" y="-5.08" length="middle"/>
<pin name="V-" x="0" y="-7.62" length="middle"/>
<pin name="NULL_2" x="33.02" y="0" length="middle" rot="R180"/>
<pin name="V+" x="33.02" y="-2.54" length="middle" rot="R180"/>
<pin name="OUT" x="33.02" y="-5.08" length="middle" rot="R180"/>
<pin name="NC" x="33.02" y="-7.62" length="middle" direction="nc" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="AD8675ARZ" prefix="IC">
<description>&lt;b&gt;Precision Amplifiers 2.5nV R-R out Precision BP AMP&lt;/b&gt;&lt;p&gt;
Source: &lt;a href="https://www.analog.com/media/en/technical-documentation/data-sheets/AD8675.pdf"&gt; Datasheet &lt;/a&gt;</description>
<gates>
<gate name="G$1" symbol="AD8675ARZ" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOIC127P600X175-8N">
<connects>
<connect gate="G$1" pin="+IN" pad="3"/>
<connect gate="G$1" pin="-IN" pad="2"/>
<connect gate="G$1" pin="NC" pad="5"/>
<connect gate="G$1" pin="NULL_1" pad="1"/>
<connect gate="G$1" pin="NULL_2" pad="8"/>
<connect gate="G$1" pin="OUT" pad="6"/>
<connect gate="G$1" pin="V+" pad="7"/>
<connect gate="G$1" pin="V-" pad="4"/>
</connects>
<technologies>
<technology name="">
<attribute name="ARROW_PART_NUMBER" value="AD8675ARZ" constant="no"/>
<attribute name="ARROW_PRICE-STOCK" value="https://www.arrow.com/en/products/ad8675arz/analog-devices" constant="no"/>
<attribute name="DESCRIPTION" value="Precision Amplifiers 2.5nV R-R out Precision BP AMP" constant="no"/>
<attribute name="HEIGHT" value="1.75mm" constant="no"/>
<attribute name="MANUFACTURER_NAME" value="Analog Devices" constant="no"/>
<attribute name="MANUFACTURER_PART_NUMBER" value="AD8675ARZ" constant="no"/>
<attribute name="MOUSER_PART_NUMBER" value="584-AD8675ARZ" constant="no"/>
<attribute name="MOUSER_PRICE-STOCK" value="https://www.mouser.co.uk/ProductDetail/Analog-Devices/AD8675ARZ?qs=%2FtpEQrCGXCyRAPqNwuQ%2Fog%3D%3D" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="con-coax" urn="urn:adsk.eagle:library:133">
<description>&lt;b&gt;Coax Connectors&lt;/b&gt;&lt;p&gt;
Radiall  and M/A COM.&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="BU-SMA-H" urn="urn:adsk.eagle:footprint:6179/1" library_version="2">
<description>FEMALE &lt;b&gt;SMA CONNECTOR&lt;/b&gt;&lt;p&gt;
Radiall&lt;p&gt;</description>
<wire x1="-1.1" y1="3.2" x2="1.1" y2="3.2" width="0.2032" layer="21"/>
<wire x1="3.2" y1="1.1" x2="3.2" y2="-1.1" width="0.2032" layer="21"/>
<wire x1="1.1" y1="-3.2" x2="-1.1" y2="-3.2" width="0.2032" layer="21"/>
<wire x1="-3.2" y1="-1.1" x2="-3.2" y2="1.1" width="0.2032" layer="21"/>
<wire x1="-3.1999" y1="3.1999" x2="3.2" y2="3.2" width="0.2032" layer="51"/>
<wire x1="3.2" y1="3.2" x2="3.2" y2="2.9" width="0.2032" layer="51"/>
<wire x1="3.2" y1="2.9" x2="3.2" y2="-2.9" width="0.2032" layer="51"/>
<wire x1="3.2" y1="-2.9" x2="3.1999" y2="-3.1999" width="0.2032" layer="51"/>
<wire x1="3.1999" y1="-3.1999" x2="-3.2" y2="-3.2" width="0.2032" layer="51"/>
<wire x1="-3.2" y1="-3.2" x2="-3.1999" y2="3.1999" width="0.2032" layer="51"/>
<wire x1="3.2" y1="2.9" x2="4.3" y2="2.9" width="0.2032" layer="51"/>
<wire x1="4.3" y1="2.9" x2="8.3" y2="2.9" width="0.2032" layer="21"/>
<wire x1="8.3" y1="2.9" x2="8.6" y2="3.4" width="0.2032" layer="21"/>
<wire x1="10.7" y1="3.4" x2="11" y2="2.9" width="0.2032" layer="21"/>
<wire x1="11" y1="2.9" x2="11.6" y2="2.9" width="0.2032" layer="21"/>
<wire x1="11.6" y1="-2.9" x2="11.6" y2="2.9" width="0.2032" layer="21"/>
<wire x1="8.6" y1="3.4" x2="10.7" y2="3.4" width="0.2032" layer="21"/>
<wire x1="3.2" y1="-2.9" x2="4.3" y2="-2.9" width="0.2032" layer="51"/>
<wire x1="4.3" y1="-2.9" x2="8.3" y2="-2.9" width="0.2032" layer="21"/>
<wire x1="8.3" y1="-2.9" x2="8.6" y2="-3.4" width="0.2032" layer="21"/>
<wire x1="10.7" y1="-3.4" x2="11" y2="-2.9" width="0.2032" layer="21"/>
<wire x1="11" y1="-2.9" x2="11.6" y2="-2.9" width="0.2032" layer="21"/>
<wire x1="8.6" y1="-3.4" x2="10.7" y2="-3.4" width="0.2032" layer="21"/>
<wire x1="11" y1="-2.9" x2="11" y2="2.9" width="0.2032" layer="21"/>
<wire x1="8.3" y1="-2.9" x2="8.3" y2="2.9" width="0.2032" layer="21"/>
<circle x="0" y="0" radius="3.1999" width="0.2032" layer="51"/>
<circle x="0" y="0" radius="1.7" width="0.2032" layer="21"/>
<pad name="1" x="0" y="0" drill="1.27"/>
<pad name="2" x="-2.5499" y="2.5499" drill="1.778"/>
<pad name="3" x="2.5499" y="2.5499" drill="1.778"/>
<pad name="4" x="2.5499" y="-2.5499" drill="1.778"/>
<pad name="5" x="-2.5499" y="-2.5499" drill="1.778"/>
<text x="-2.54" y="4.445" size="1.27" layer="25">&gt;NAME</text>
<text x="-3.175" y="-5.715" size="1.27" layer="27">&gt;VALUE</text>
</package>
</packages>
<packages3d>
<package3d name="BU-SMA-H" urn="urn:adsk.eagle:package:6206/1" type="box" library_version="2">
<description>FEMALE SMA CONNECTOR
Radiall</description>
<packageinstances>
<packageinstance name="BU-SMA-H"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="BNC-FGND" urn="urn:adsk.eagle:symbol:6174/1" library_version="2">
<wire x1="0" y1="-2.54" x2="-0.762" y2="-1.778" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="-0.508" y2="0" width="0.1524" layer="94"/>
<wire x1="-2.54" y1="0.508" x2="-0.762" y2="0.508" width="0.254" layer="94"/>
<wire x1="-0.762" y1="0.508" x2="-0.508" y2="0" width="0.254" layer="94"/>
<wire x1="-0.508" y1="0" x2="-0.762" y2="-0.508" width="0.254" layer="94"/>
<wire x1="-0.762" y1="-0.508" x2="-2.54" y2="-0.508" width="0.254" layer="94"/>
<wire x1="-2.54" y1="2.54" x2="0" y2="0.508" width="0.3048" layer="94" curve="-79.611142" cap="flat"/>
<wire x1="-2.54" y1="-2.54" x2="0" y2="-0.508" width="0.3048" layer="94" curve="79.611142" cap="flat"/>
<text x="-2.54" y="-5.08" size="1.778" layer="96">&gt;VALUE</text>
<text x="-2.54" y="3.302" size="1.778" layer="95">&gt;NAME</text>
<pin name="1" x="2.54" y="0" visible="off" length="short" direction="pas" rot="R180"/>
<pin name="2" x="2.54" y="-2.54" visible="off" length="short" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="BU-SMA-H" urn="urn:adsk.eagle:component:6234/2" prefix="X" library_version="2">
<description>FEMALE &lt;b&gt;SMA CONNECTOR&lt;/b&gt;&lt;p&gt;
Radiall&lt;p&gt;</description>
<gates>
<gate name="G$1" symbol="BNC-FGND" x="0" y="0"/>
</gates>
<devices>
<device name="" package="BU-SMA-H">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="2 3 4 5"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:6206/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="MF" value="" constant="no"/>
<attribute name="MPN" value="" constant="no"/>
<attribute name="OC_FARNELL" value="unknown" constant="no"/>
<attribute name="OC_NEWARK" value="unknown" constant="no"/>
<attribute name="POPULARITY" value="4" constant="no"/>
</technology>
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
<part name="RP-E2" library="con-lsta" library_urn="urn:adsk.eagle:library:161" deviceset="FE13-2" device="" package3d_urn="urn:adsk.eagle:package:8172/1"/>
<part name="RP-E1" library="con-lsta" library_urn="urn:adsk.eagle:library:161" deviceset="FE13-2" device="" package3d_urn="urn:adsk.eagle:package:8172/1"/>
<part name="IC1" library="AD5791ARUZ" deviceset="AD5791ARUZ" device=""/>
<part name="IC2" library="AD8676BRZ" deviceset="AD8676BRZ" device=""/>
<part name="IC3" library="AD8675ARZ" deviceset="AD8675ARZ" device=""/>
<part name="X1" library="con-coax" library_urn="urn:adsk.eagle:library:133" deviceset="BU-SMA-H" device="" package3d_urn="urn:adsk.eagle:package:6206/1"/>
</parts>
<sheets>
<sheet>
<plain>
<text x="-45.72" y="86.36" size="1.778" layer="91">VCC 3.3V</text>
</plain>
<instances>
<instance part="RP-E2" gate="G$1" x="-53.34" y="38.1" smashed="yes">
<attribute name="VALUE" x="-57.15" y="17.78" size="1.778" layer="96"/>
<attribute name="NAME" x="-57.15" y="56.642" size="1.778" layer="95"/>
</instance>
<instance part="RP-E1" gate="G$1" x="-53.34" y="106.68" smashed="yes">
<attribute name="VALUE" x="-57.15" y="86.36" size="1.778" layer="96"/>
<attribute name="NAME" x="-57.15" y="125.222" size="1.778" layer="95"/>
</instance>
<instance part="IC1" gate="G$1" x="5.08" y="106.68" smashed="yes">
<attribute name="NAME" x="34.29" y="114.3" size="1.778" layer="95" align="center-left"/>
<attribute name="VALUE" x="34.29" y="111.76" size="1.778" layer="96" align="center-left"/>
</instance>
<instance part="IC2" gate="G$1" x="10.16" y="58.42" smashed="yes">
<attribute name="NAME" x="39.37" y="66.04" size="1.778" layer="95" align="center-left"/>
<attribute name="VALUE" x="39.37" y="63.5" size="1.778" layer="96" align="center-left"/>
</instance>
<instance part="IC3" gate="G$1" x="60.96" y="99.06" smashed="yes">
<attribute name="NAME" x="90.17" y="106.68" size="1.778" layer="95" align="center-left"/>
<attribute name="VALUE" x="90.17" y="104.14" size="1.778" layer="96" align="center-left"/>
</instance>
<instance part="X1" gate="G$1" x="83.82" y="68.58" smashed="yes">
<attribute name="VALUE" x="81.28" y="63.5" size="1.778" layer="96"/>
<attribute name="NAME" x="81.28" y="71.882" size="1.778" layer="95"/>
</instance>
</instances>
<busses>
</busses>
<nets>
<net name="VCC" class="0">
<segment>
<wire x1="-48.26" y1="91.44" x2="-45.72" y2="91.44" width="0.1524" layer="91"/>
<junction x="-45.72" y="91.44"/>
<pinref part="RP-E1" gate="G$1" pin="2"/>
<wire x1="-60.96" y1="91.44" x2="-45.72" y2="91.44" width="0.1524" layer="91"/>
<pinref part="RP-E1" gate="G$1" pin="1"/>
<junction x="-60.96" y="91.44"/>
<label x="-45.72" y="88.9" size="1.778" layer="95"/>
<label x="-45.72" y="88.9" size="1.778" layer="95"/>
</segment>
</net>
<net name="GND" class="0">
<segment>
<pinref part="RP-E1" gate="G$1" pin="26"/>
<wire x1="-45.72" y1="121.92" x2="-60.96" y2="121.92" width="0.1524" layer="91"/>
<pinref part="RP-E1" gate="G$1" pin="25"/>
<label x="-45.72" y="129.54" size="1.778" layer="95"/>
</segment>
<segment>
<pinref part="RP-E2" gate="G$1" pin="26"/>
<pinref part="RP-E2" gate="G$1" pin="25"/>
<wire x1="-60.96" y1="53.34" x2="-45.72" y2="53.34" width="0.1524" layer="91"/>
<junction x="-60.96" y="53.34"/>
<label x="-45.72" y="60.96" size="1.778" layer="95"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
<errors>
<approved hash="104,1,17.78,73.66,U1,VCC_2,VCC+3.3V,,,"/>
<approved hash="104,1,17.78,71.12,U1,VCC,VCC+3.3V,,,"/>
<approved hash="104,1,17.78,20.32,U2,VCC_2,VCC+3.3V,,,"/>
<approved hash="104,1,17.78,17.78,U2,VCC,VCC+3.3V,,,"/>
<approved hash="104,1,17.78,127,U3,VCC_2,VCC+3.3V,,,"/>
<approved hash="104,1,17.78,124.46,U3,VCC,VCC+3.3V,,,"/>
<approved hash="113,1,115.62,119.72,J1,,,,,"/>
<approved hash="113,1,115.62,66.3804,J2,,,,,"/>
<approved hash="113,1,-53.34,108.24,RP-E1,,,,,"/>
<approved hash="113,1,-53.34,39.6596,RP-E2,,,,,"/>
</errors>
</schematic>
</drawing>
<compatibility>
<note version="8.2" severity="warning">
Since Version 8.2, EAGLE supports online libraries. The ids
of those online libraries will not be understood (or retained)
with this version.
</note>
<note version="8.3" severity="warning">
Since Version 8.3, EAGLE supports URNs for individual library
assets (packages, symbols, and devices). The URNs of those assets
will not be understood (or retained) with this version.
</note>
<note version="8.3" severity="warning">
Since Version 8.3, EAGLE supports the association of 3D packages
with devices in libraries, schematics, and board files. Those 3D
packages will not be understood (or retained) with this version.
</note>
</compatibility>
</eagle>
