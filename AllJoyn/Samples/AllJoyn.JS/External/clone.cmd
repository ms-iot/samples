if NOT EXIST allseen mkdir allseen

cd allseen
if NOT EXIST core mkdir core
if NOT EXIST services mkdir services

cd core
if EXIST ajtcl goto :alljoyn-js
git clone --recursive https://git.allseenalliance.org/gerrit/core/ajtcl.git
cd ajtcl
git checkout RB15.04
cd ..

:alljoyn-js
if EXIST alljoyn-js goto :base_tcl
git clone --recursive https://git.allseenalliance.org/gerrit/core/alljoyn-js.git
cd alljoyn-js
git checkout RB15.04
cd ..

:base_tcl
cd ..\services
if EXIST base_tcl goto :exit
git clone --recursive https://git.allseenalliance.org/gerrit/services/base_tcl.git
cd base_tcl
git checkout RB15.04
cd ..
cd ..

:exit

