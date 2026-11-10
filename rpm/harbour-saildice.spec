Name:       harbour-saildice
Summary:    Real 3D polyhedral dice roller
Version:    1.0.0
Release:    1
License:    MIT
URL:        https://github.com/protongit/harbour-saildice
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  desktop-file-utils

%description
SailDice is a native Sailfish OS dice roller with real 3D polyhedral dice.
Configure up to twelve D4, D6, D8, D10, D12 and D20 dice, choose a color
for each one, roll them from the main page or the cover and review the
current session's roll history. Everything runs locally; no network access
is needed.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5
%make_build

%install
%qmake5_install
%{__strip} %{buildroot}%{_bindir}/%{name}

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
