%define __jar_repack %{nil}
Name: jshu
Version: 1.0.0
Release: 20
Summary: Simplified unit test framework for shell script which produces junit-style xml results file
BuildArch: noarch
License: BSD
URL: https://github.com/AdrianDC/jshu
Source0: jshutest.inc
Source1: wrapper.sh

%description
Simplified unit test framework for shell script which produces junit-style xml results file (for Jenkins/Hudson).

%install
mkdir -p %{buildroot}/opt/jshu
install -m 755 %SOURCE0 %{buildroot}/opt/jshu
install -m 755 %SOURCE1 %{buildroot}/opt/jshu

%files
%defattr(-, root, root)
%dir /opt/jshu
/opt/jshu/jshutest.inc
/opt/jshu/wrapper.sh
%attr(755,root,root) /opt/jshu/jshutest.inc
%attr(755,root,root) /opt/jshu/wrapper.sh

%changelog
