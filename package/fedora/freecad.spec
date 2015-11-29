
# Maintainers:  keep this list of plugins up to date
# List plugins in %%{_libdir}/freecad/lib, less '.so' and 'Gui.so', here
%global plugins Complete Drawing Fem FreeCAD Image Import Inspection Mesh MeshPart Part Points QtUnit Raytracing ReverseEngineering Robot Sketcher Start Web PartDesignGui _PartDesign Path PathGui Spreadsheet SpreadsheetGui area DraftUtils DraftUtils

# Some plugins go in the Mod folder instead of lib. Deal with those here:
%global mod_plugins Mod/PartDesign

Name:           freecad
Epoch:          1
Version:        0.16
Release:        git_master_5991
Summary:        A general purpose 3D CAD modeler
Group:          Applications/Engineering

License:        GPLv2+
URL:            http://sourceforge.net/apps/mediawiki/free-cad/
Source0:        freecad.tar.gz
Source101:      freecad.desktop
#Source102:      freecad.1
Source103:      freecad.appdata.xml
Source104:      freecad.sharedmimeinfo

#Patch0:         freecad-3rdParty.patch
#Patch1:         freecad-0.14-Xlib_h.patch
#Patch2:         freecad-0.14-smesh.patch
#Patch3:         freecad-0.14-DraftSnap.patch
#Patch4:         freecad-0.14-Version_h.patch

# Utilities
# Development Libraries
# Not used yet.
#BuildRequires:  ode-devel
BuildRequires:  git
BuildRequires:  Coin3
BuildRequires:  Coin3-devel
BuildRequires:  OCE-devel
BuildRequires:  OCE-draw
BuildRequires:  boost-devel
BuildRequires:  cmake
BuildRequires:  desktop-file-utils
BuildRequires:  dos2unix
BuildRequires:  doxygen
BuildRequires:  eigen3
BuildRequires:  eigen3-devel
BuildRequires:  freeimage-devel
BuildRequires:  gettext
BuildRequires:  graphviz
BuildRequires:  graphviz
BuildRequires:  libicu-devel
BuildRequires:  mesa-libGLU-devel
BuildRequires:  netgen-mesher-devel
BuildRequires:  pyside-tools
BuildRequires:  python-matplotlib
BuildRequires:  python-pivy
BuildRequires:  python-pyside
BuildRequires:  python-pyside-devel
BuildRequires:  python2-devel
BuildRequires:  qt-devel
BuildRequires:  qt-webkit-devel
BuildRequires:  shiboken
BuildRequires:  shiboken-devel
BuildRequires:  smesh
BuildRequires:  smesh-devel
BuildRequires:  swig
BuildRequires:  tbb-devel
BuildRequires:  xerces-c
BuildRequires:  xerces-c-devel
BuildRequires:  zlib-devel 

# For appdata
%if 0%{?fedora}
BuildRequires:  libappstream-glib
%endif

# Packages separated because they are noarch, but not optional so require them
# here.
Requires:       %{name}-data = %{epoch}:%{version}-%{release}
# Obsolete old doc package since it's required for functionality.
Obsoletes:      %{name}-doc < 0.13-5

# Needed for plugin support and is not a soname dependency.
%if ! 0%{?rhel} <= 6 && "%{_arch}" != "ppc64"
# python-pivy does not build on EPEL 6 ppc64.
Requires:       python-pivy
%endif
Requires:       hicolor-icon-theme
Requires:       python-matplotlib
Requires:       python-collada
Requires:       python-pyside

# plugins and private shared libs in %%{_libdir}/freecad/lib are private;
# prevent private capabilities being advertised in Provides/Requires
%define plugin_regexp /^\\\(libFreeCAD.*%(for i in %{plugins}; do echo -n "\\\|$i\\\|$iGui"; done)\\\)\\\(\\\|Gui\\\)\\.so/d
%{?filter_setup:
%filter_provides_in %{_libdir}/%{name}/lib
%filter_from_requires %{plugin_regexp}
%filter_from_provides %{plugin_regexp}
%filter_provides_in %{_libdir}/%{name}/Mod
%filter_requires_in %{_libdir}/%{name}/Mod
%filter_setup
}


%description
FreeCAD is a general purpose Open Source 3D CAD/MCAD/CAx/CAE/PLM modeler, aimed
directly at mechanical engineering and product design but also fits a wider
range of uses in engineering, such as architecture or other engineering
specialties. It is a feature-based parametric modeler with a modular software
architecture which makes it easy to provide additional functionality without
modifying the core system.


%package data
Summary:        Data files for FreeCAD
BuildArch:      noarch
Requires:       %{name} = %{epoch}:%{version}-%{release}

%description data
Data files for FreeCAD


%prep
%setup -q -n freecad

# Fix encodings
dos2unix -k src/Mod/Test/unittestgui.py \
            ChangeLog.txt \
            copying.lib \
            data/License.txt

# Removed bundled libraries

%build
rm -rf build && mkdir build && cd build

# Deal with cmake projects that tend to link excessively.
#LDFLAGS='-Wl,--as-needed'; export LDFLAGS

%cmake -DCMAKE_INSTALL_PREFIX=%{_libdir}/%{name} \
       -DCMAKE_INSTALL_DATADIR=%{_datadir}/%{name} \
       -DCMAKE_INSTALL_DOCDIR=%{_docdir}/%{name} \
       -DCMAKE_INSTALL_INCLUDEDIR=%{_includedir} \
       -DRESOURCEDIR=%{_datadir}/%{name} \
       -DFREECAD_USE_EXTERNAL_PIVY=TRUE \
       -DFREECAD_USE_EXTERNAL_SMESH=TRUE \
        ../
       #-DFREECAD_USE_EXTERNAL_SMESH=TRUE \
       #-DBUILD_FEM_NETGEN=TRUE \
       # ../
       
       

make %{?_smp_mflags}

%install
cd build
%make_install

# Symlink binaries to /usr/bin
mkdir -p %{buildroot}%{_bindir}
pushd %{buildroot}%{_bindir}
ln -s ../%{_lib}/freecad/bin/FreeCAD .
ln -s ../%{_lib}/freecad/bin/FreeCADCmd .
popd

# Install desktop file
desktop-file-install                                   \
    --dir=%{buildroot}%{_datadir}/applications         \
    %{SOURCE101}
sed -i 's,@lib@,%{_lib},g' %{buildroot}%{_datadir}/applications/%{name}.desktop

# Install desktop icon
install -pD -m 0644 ../src/Gui/Icons/%{name}.svg \
    %{buildroot}%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg


# Install MimeType file
mkdir -p %{buildroot}%{_datadir}/mime/packages
install -pm 0644 %{SOURCE104} %{buildroot}%{_datadir}/mime/packages/%{name}.xml

# Install appdata file
mkdir -p %{buildroot}%{_datadir}/appdata
install -pm 0644 %{SOURCE103} %{buildroot}%{_datadir}/appdata/

# Bug maintainers to keep %%{plugins} macro up to date.
#
# Make sure there are no plugins that need to be added to plugins macro
new_plugins=`ls %{buildroot}%{_libdir}/freecad/lib | sed -e  '%{plugin_regexp}'`
if [ -n "$new_plugins" ]; then
    echo -e "\n\n\n**** ERROR:\n" \
        "\nPlugins not caught by regexp:  " $new_plugins \
        "\n\nPlugins in %{_libdir}/freecad/lib do not exist in" \
         "\nspecfile %%{plugins} macro.  Please add these to" \
         "\n%%{plugins} macro at top of specfile and rebuild.\n****\n" 1>&2
    exit 1
fi
# Make sure there are no entries in the plugins macro that don't match plugins
for p in %{plugins}; do
    if [ -z "`ls %{buildroot}%{_libdir}/freecad/lib/$p*.so`" ]; then
        set +x
        echo -e "\n\n\n**** ERROR:\n" \
             "\nExtra entry in %%{plugins} macro with no matching plugin:" \
             "'$p'.\n\nPlease remove from %%{plugins} macro at top of" \
             "\nspecfile and rebuild.\n****\n" 1>&2
        exit 1
    fi
done



%check
%{?fedora:appstream-util validate-relax --nonet \
    %{buildroot}/%{_datadir}/appdata/*.appdata.xml}


%post
/bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
/usr/bin/update-desktop-database &> /dev/null || :
/usr/bin/update-mime-database %{_datadir}/mime &> /dev/null || :

%postun
if [ $1 -eq 0 ] ; then
    /bin/touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    /usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi
/usr/bin/update-desktop-database &> /dev/null || :
/usr/bin/update-mime-database %{_datadir}/mime &> /dev/null || :

%posttrans
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :


%files
%license copying.lib data/License.txt
%doc ChangeLog.txt
%exclude %{_docdir}/freecad/freecad.*
%{_bindir}/*
%{_datadir}/appdata/*.appdata.xml
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg
%{_datadir}/mime/packages/%{name}.xml
%dir %{_libdir}/%{name}
%{_libdir}/%{name}/bin/
%{_libdir}/%{name}/lib/
%{_libdir}/%{name}/Mod/

%files data
%{_datadir}/%{name}/
%{_docdir}/%{name}/freecad.q*

