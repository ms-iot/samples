%define PREFIX /usr/apps/com.oic.ri.sample
%define ROOTDIR  %{_builddir}/%{name}-%{version}

Name: com-oic-ri-sample
Version:    0.1
Release:    1
Summary: Tizen adapter interfacesample application
URL: http://slp-source.sec.samsung.net
Source: %{name}-%{version}.tar.gz
License: Apache-2.0
Group: Applications/OICSample
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: boost-devel
BuildRequires: boost-thread
BuildRequires: boost-system
BuildRequires: boost-filesystem
BuildRequires: pkgconfig(capi-network-wifi)
BuildRequires: pkgconfig(capi-network-bluetooth)
BuildRequires: scons
BuildRequires: com-oic-ri


%description
OIC RIsample application

%prep
%setup -q

%build

scons TARGET_OS=tizen -c
scons TARGET_OS=tizen TARGET_TRANSPORT=%{TARGET_TRANSPORT} SECURED=%{SECURED} RELEASE=%{RELEASE} ROUTING=%{ROUTING}

%install

mkdir -p %{buildroot}%{_datadir}/packages
mkdir -p %{buildroot}/%{_sysconfdir}/smack/accesses2.d
mkdir -p %{buildroot}/usr/apps/com.oic.ri.sample/bin/
mkdir -p %{buildroot}/usr/apps/com.oic.ri.sample/bin/internal

cp -rf %{ROOTDIR}/com.oic.ri.sample.xml %{buildroot}/%{_datadir}/packages
cp -rf %{ROOTDIR}/scons/occlient %{buildroot}/usr/apps/com.oic.ri.sample/bin/
cp -rf %{ROOTDIR}/scons/ocserver %{buildroot}/usr/apps/com.oic.ri.sample/bin/
cp -rf %{ROOTDIR}/scons/ocrouting %{buildroot}/usr/apps/com.oic.ri.sample/bin/

%files
%manifest com.oic.ri.sample.manifest
%defattr(-,root,root,-)
/usr/apps/com.oic.ri.sample/bin/occlient
/usr/apps/com.oic.ri.sample/bin/ocserver
/usr/apps/com.oic.ri.sample/bin/ocrouting
/%{_datadir}/packages/com.oic.ri.sample.xml


