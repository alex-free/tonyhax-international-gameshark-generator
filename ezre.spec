Name: thigsgen       
Version: 2.0.5
Summary: Generates save game files containing GameShark codes for Tonyhax International from txt files.
Release: 1
License: 3-BSD
URL: https://github.com/alex-free/tonyhax-international-gameshark-generator       
Packager: Alex Free

%description
Generates save game files containing GameShark codes for Tonyhax International from txt files.

%install
mkdir -p %{buildroot}/usr/bin
cp %{_sourcedir}/thigsgen %{buildroot}/usr/bin/

%files
/usr/bin/thigsgen
