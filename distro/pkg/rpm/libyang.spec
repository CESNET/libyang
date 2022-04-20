Name: libyang
Version: {{ version }}
Release: {{ release }}%{?dist}
Summary: YANG data modeling language library
Url: https://github.com/CESNET/libyang
Source: %{url}/archive/v%{version}/%{name}-%{version}.tar.gz
License: BSD

BuildRequires:  cmake
BuildRequires:  doxygen
BuildRequires:  gcc
BuildRequires:  cmake(cmocka) >= 1.0.1
BuildRequires:  make
BuildRequires:  pkgconfig(libpcre2-8) >= 10.21

%package devel
Summary:    Development files for libyang
Requires:   %{name}%{?_isa} = %{version}-%{release}
Requires:   pcre2-devel

%package devel-doc
Summary:    Documentation of libyang API
Requires:   %{name}%{?_isa} = %{version}-%{release}

%package tools
Summary:        YANG validator tools
Requires:       %{name}%{?_isa} = %{version}-%{release}
# This was not properly split out before
Conflicts:      %{name} < 1.0.225-3

%description devel
Headers of libyang library.

%description devel-doc
Documentation of libyang API.

%description tools
YANG validator tools.

%description
Libyang is YANG data modeling language parser and toolkit
written (and providing API) in C.

%prep
%autosetup -p1

%build
%cmake -DCMAKE_BUILD_TYPE=RELWITHDEBINFO
%cmake_build

%if "x%{?suse_version}" == "x"
cd redhat-linux-build
%endif
make doc

%check
%if "x%{?suse_version}" == "x"
cd redhat-linux-build
%endif
ctest --output-on-failure -V %{?_smp_mflags}

%install
%cmake_install

mkdir -m0755 -p %{buildroot}/%{_docdir}/libyang
cp -a doc/html %{buildroot}/%{_docdir}/libyang/html

%files
%license LICENSE
%{_libdir}/libyang.so.2
%{_libdir}/libyang.so.2.*

%files tools
%{_bindir}/yanglint
%{_bindir}/yangre
%{_datadir}/man/man1/yanglint.1.gz
%{_datadir}/man/man1/yangre.1.gz

%files devel
%{_libdir}/libyang.so
%{_libdir}/pkgconfig/libyang.pc
%{_includedir}/libyang/*.h
%dir %{_includedir}/libyang/

%files devel-doc
%{_docdir}/libyang

%changelog
* {{ now }} Jakub Ružička <jakub.ruzicka@nic.cz> - {{ version }}-{{ release }}
- upstream package
