%define PREFIX /usr/apps/com.oic.ca.sample
%define ROOTDIR  %{_builddir}/%{name}-%{version}

Name: com-oic-ca-sample
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
BuildRequires: com-oic-ca


%description
OIC interfacesample application

%prep
%setup -q

%build

scons TARGET_OS=tizen -c
scons TARGET_OS=tizen TARGET_TRANSPORT=%{TARGET_TRANSPORT} SECURED=%{SECURED} RELEASE=%{RELEASE}

%install

mkdir -p %{buildroot}%{_datadir}/packages
mkdir -p %{buildroot}/%{_sysconfdir}/smack/accesses2.d
mkdir -p %{buildroot}/usr/apps/com.oic.ca.sample/bin/
mkdir -p %{buildroot}/usr/apps/com.oic.ca.sample/bin/internal

cp -rf %{ROOTDIR}/com.oic.ca.sample.xml %{buildroot}/%{_datadir}/packages
cp -rf %{ROOTDIR}/scons/ca_sample %{buildroot}/usr/apps/com.oic.ca.sample/bin/

%files
%manifest com.oic.ca.sample.manifest
%defattr(-,root,root,-)
/usr/apps/com.oic.ca.sample/bin/ca_sample
/%{_datadir}/packages/com.oic.ca.sample.xml


