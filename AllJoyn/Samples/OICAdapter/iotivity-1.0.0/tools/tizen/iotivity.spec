Name: iotivity
Version: 0.9.2
Release: 0
Summary: IoTivity Base Stack & IoTivity Services
Group: System Environment/Libraries
License: Apache-2.0
URL: https://www.iotivity.org/
Source0: %{name}-%{version}.tar.bz2
Source1001: %{name}.manifest
Source1002: %{name}-test.manifest
BuildRequires:	gettext, expat-devel
BuildRequires:	python, libcurl-devel
BuildRequires:	scons
BuildRequires:	openssl-devel
BuildRequires:  boost-devel
BuildRequires:  boost-thread
BuildRequires:  boost-system
BuildRequires:  boost-filesystem
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(uuid)
BuildRequires:  pkgconfig(capi-network-wifi)
BuildRequires:  pkgconfig(capi-network-bluetooth)
BuildRequires:  pkgconfig(capi-appfw-app-common)
BuildRequires:  pkgconfig(glib-2.0)
Requires(postun): /sbin/ldconfig
Requires(post): /sbin/ldconfig

%define release_mode false
%define secure_mode 0

%description
IoTivity Base (RICH & LITE) Stack & IoTivity Services

%package service
Summary: Development files for %{name}
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description service
The %{name}-service package contains service libraries files for
developing applications that use %{name}-service.

%package test
Summary: Development files for %{name}
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}

%description test
The %{name}-test package contains example files to show
how the iotivity works using %{name}-test

%package devel
Summary: Development files for %{name}
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: pkgconfig

%description devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%prep
%setup -q -n %{name}-%{version}
cp %{SOURCE1001} .
%if 0%{?tizen_version_major} < 3
cp %{SOURCE1002} .
%else
cp %{SOURCE1001} ./%{name}-test.manifest
%endif

%build
%define RPM_ARCH %{_arch}

%ifarch armv7l armv7hl armv7nhl armv7tnhl armv7thl
%define RPM_ARCH "armeabi-v7a"
%endif

%ifarch aarch64
%define RPM_ARCH "arm64"
%endif

%ifarch x86_64
%define RPM_ARCH "x86_64"
%endif

%ifarch %{ix86}
%define RPM_ARCH "x86"
%endif


scons -j 4 TARGET_OS=tizen TARGET_ARCH=%{RPM_ARCH} TARGET_TRANSPORT=IP RELEASE=%{release_mode} SECURED=%{secure_mode}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_includedir}
mkdir -p %{buildroot}%{_libdir}
mkdir -p %{buildroot}%{_bindir}


%if %{release_mode} == "true"
%define build_mode release
%else
%define build_mode debug
%endif

cp out/tizen/*/%{build_mode}/resource/examples/devicediscoveryclient %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/devicediscoveryserver %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/fridgeclient %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/fridgeserver %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/garageclient %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/garageserver %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/groupclient %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/groupserver %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/lightserver %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/presenceclient %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/presenceserver %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/roomclient %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/roomserver %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/simpleclient %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/simpleclientHQ %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/simpleclientserver %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/simpleserver %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/simpleserverHQ %{buildroot}%{_bindir}
cp out/tizen/*/%{build_mode}/resource/examples/threadingsample %{buildroot}%{_bindir}
if echo %{secure_mode}|grep -qi '1'; then
	cp out/tizen/*/%{build_mode}/libocpmapi.a %{buildroot}%{_libdir}
fi
cp out/tizen/*/%{build_mode}/libcoap.a %{buildroot}%{_libdir}
cp out/tizen/*/%{build_mode}/lib*.so %{buildroot}%{_libdir}

cp resource/csdk/stack/include/*.h %{buildroot}%{_includedir}
cp resource/csdk/logger/include/*.h %{buildroot}%{_includedir}
cp resource/csdk/ocrandom/include/*.h %{buildroot}%{_includedir}
cp -r resource/oc_logger/include/* %{buildroot}%{_includedir}
cp resource/include/*.h %{buildroot}%{_includedir}

cp service/things-manager/sdk/inc/*.h %{buildroot}%{_includedir}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/liboc.so
%{_libdir}/liboc_logger.so
%{_libdir}/liboc_logger_core.so
%{_libdir}/liboctbstack.so
%{_libdir}/libconnectivity_abstraction.so
%{_libdir}/lib*.a

%files service
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/libBMISensorBundle.so
%{_libdir}/libDISensorBundle.so
%{_libdir}/libresource_hosting.so
%{_libdir}/libTGMSDKLibrary.so
%{_libdir}/libHueBundle.so
%{_libdir}/librcs_client.so
%{_libdir}/librcs_common.so
%{_libdir}/librcs_container.so
%{_libdir}/librcs_server.so

%files test
%manifest %{name}-test.manifest
%defattr(-,root,root,-)
%{_bindir}/devicediscoveryclient
%{_bindir}/devicediscoveryserver
%{_bindir}/fridgeclient
%{_bindir}/fridgeserver
%{_bindir}/garageclient
%{_bindir}/garageserver
%{_bindir}/groupclient
%{_bindir}/groupserver
%{_bindir}/lightserver
%{_bindir}/presenceclient
%{_bindir}/presenceserver
%{_bindir}/roomclient
%{_bindir}/roomserver
%{_bindir}/simpleclient
%{_bindir}/simpleclientHQ
%{_bindir}/simpleclientserver
%{_bindir}/simpleserver
%{_bindir}/simpleserverHQ
%{_bindir}/threadingsample

%files devel
%defattr(-,root,root,-)
%{_includedir}/*
