# make rpm이 사용한다. stage는 make install로 만든 설치 트리다.
%global debug_package %{nil}

Name:           nfkcgen
Version:        %{version}
Release:        1
Summary:        NFKC research string generator (GTK 4)
License:        See LICENSE in the source repository
URL:            https://github.com/kaede982006/nfkcgen
BuildArch:      x86_64

%description
GTK 4 desktop program that generates classic/modern type 1-4 experimental
strings and compares code point lengths before and after NFKC normalization.

%install
cp -a %{stage}/. %{buildroot}/

%files
/usr/bin/nfkcgen
/usr/share/applications/org.xisik.nfkcgen.desktop
