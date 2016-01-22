Name: libopenzwave
%if 0%{?fedora} > 0
Group: Development/Libraries
%else
Group: Productivity/Networking/Other
%endif
Summary: Library to access Z-Wave interfaces
URL:http://code.google.com/p/open-zwave/
%if 0%{?suse_version} > 0
License: LGPL-2.0+
%else 
License: LGPLv2+
%endif
Version: 1.2.730
Release: 1
BuildRequires: gcc-c++ make libudev-devel doxygen graphviz
%if 0%{?fedora} >= 18
BuildRequires: systemd-devel pkgconfig
%else
%if 0%{?suse_version} >= 1220
BuildRequires: systemd-devel pkg-config
%else
BuildRequires: libudev-devel pkgconfig
%endif
%endif
Source0: libopenzwave-%{version}.tar.gz
#Source0: openzwave-%{version}.tar.gz

BuildRoot: %{_tmppath}/libopenzwave-root

%description
OpenZWave is an open-source, cross-platform library designed to enable anyone to
add support for Z-Wave home-automation devices to their applications, without 
requiring any in depth knowledge of the Z-Wave protocol.

Z-Wave employs a proprietary protocol which the owners, Sigma Designs, have 
chosen not to release into the public domain. There is also no official free 
or low-cost SDK that can be used to develop applications (The ControlThink SDK
is now tied exclusively to their own Z-Wave PC interface). The only way to 
obtain the protocol documentation and sample code is to purchase an expensive 
development kit, and sign a non-disclosure agreement (NDA) preventing the 
release of that knowledge.

OpenZWave was created to fill that gap. We do not have the official 
documentation, have signed no NDA, and are free to develop the library as we 
see fit. Our knowledge comes from existing bodies of open-source code 
(principally the Z-Wave parts of LinuxMCE), and through examining the 
messages sent by Z-Wave devices.

The goal of the project is to make a positive contribution to the Z-Wave 
community by creating a library that supports as much of the Z-Wave 
specification as possible, and that can be used as a "black-box" solution 
by anyone wanting to add Z-Wave to their application. It is NOT our aim 
to publish alternative documentation of the Z-Wave protocol, or to 
attempt to "punish" Sigma Designs for their decision to keep the 
protocol closed.

%package -n libopenzwave-devel
Summary: Open-ZWave header files
%if 0%{?fedora} > 0
Group: Development/Libraries
%else
Group: Development/Libraries/C and C++
%endif
Requires: %{name} = %{version}-%{release}
#BuildRequires: %{name}

%description -n libopenzwave-devel
header files needed when you want to compile your own 
applications using openzwave

%package -n openzwave
Summary: Open-ZWave Sample Executables
%if 0%{?fedora} > 0
Group: Development/Libraries
%else
Group: Development/Libraries/C and C++
%endif
Requires: %{name} = %{version}-%{release}
#BuildRequires: %{name}

%description -n openzwave
Sample Executables for OpenZWave

%prep

%setup -q 


%build
major_ver=$(echo %{version} | awk -F \. {'print $1'})
minor_ver=$(echo %{version} | awk -F \. {'print $2'})
revision=$(echo %{version} | awk -F \. {'print $3'})
VERSION_MAJ=$major_ver VERSION_MIN=$minor_ver VERSION_REV=$revision PREFIX=/usr sysconfdir=%{_sysconfdir}/openzwave/ includedir=%{_includedir} docdir=%{_defaultdocdir}/openzwave-%{version} instlibdir=%{_libdir} make

%install
rm -rf %{buildroot}/*
major_ver=$(echo %{version} | awk -F \. {'print $1'})
minor_ver=$(echo %{version} | awk -F \. {'print $2'})
revision=$(echo %{version} | awk -F \. {'print $3'})
mkdir -p %{buildroot}/%{_bindir}
mkdir -p %{buildroot}/%{_libdir}
mkdir -p %{buildroot}/%{_defaultdocdir}/openzwave-%{version}/
mkdir -p %{buildroot}/%{_sysconfdir}/
mkdir -p %{buildroot}/%{_includedir}/openzwave/
DESTDIR=%{buildroot} VERSION_MAJ=$major_ver VERSION_MIN=$minor_ver VERSION_REV=$revision PREFIX=/usr sysconfdir=%{_sysconfdir}/openzwave/ includedir=%{_includedir}/openzwave/ docdir=%{_defaultdocdir}/openzwave-%{version} instlibdir=%{_libdir} make install
cp -p INSTALL %{buildroot}/%{_defaultdocdir}/openzwave-%{version}/
cp -pr license %{buildroot}/%{_defaultdocdir}/openzwave-%{version}/
rm %{buildroot}%{_defaultdocdir}/openzwave-%{version}/Doxyfile.in
rm -rf %{buildroot}%{_defaultdocdir}/openzwave-%{version}/html/

%files
%defattr(-,root,root,-)
%{_libdir}/libopenzwave.so.*
%dir %{_defaultdocdir}/openzwave-%{version}
%doc %{_defaultdocdir}/openzwave-%{version}/default.htm
%doc %{_defaultdocdir}/openzwave-%{version}/general/
%doc %{_defaultdocdir}/openzwave-%{version}/images+css/
%doc %{_defaultdocdir}/openzwave-%{version}/license/
%doc %{_defaultdocdir}/openzwave-%{version}/INSTALL
%config(noreplace) %{_sysconfdir}/openzwave/



%files -n libopenzwave-devel
%defattr(-,root,root,-)
%{_includedir}/openzwave/
%{_libdir}/libopenzwave.so
%{_libdir}/pkgconfig/libopenzwave.pc
%dir %{_defaultdocdir}/openzwave-%{version}
%doc %{_defaultdocdir}/openzwave-%{version}/api/


%files -n openzwave
%defattr(-,root,root,-)
%{_bindir}/MinOZW


%post
/sbin/ldconfig 

%post -n libopenzwave-devel
/sbin/ldconfig 

%postun
/sbin/ldconfig 

%changelog
* Tue Feb 04 2014 Justin Hammond+justin@dynam.ac - 1.0.730-1
- Initial Release

