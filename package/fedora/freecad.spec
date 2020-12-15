# Setup python target for shiboken so the right cmake file is imported.
%global py_suffix %(%{__python3} -c "import sysconfig; print(sysconfig.get_config_var('SOABI'))")

# Maintainers:  keep this list of plugins up to date
# List plugins in %%{_libdir}/freecad/lib, less '.so' and 'Gui.so', here
%global plugins Complete DraftUtils Drawing Fem FreeCAD Image Import Inspection Mesh MeshPart Part PartDesign Path Points QtUnit Raytracing ReverseEngineering Robot Sketcher Spreadsheet Start Web PartDesignGui _PartDesign Spreadsheet SpreadsheetGui area

# Some configuration options for other environments
# rpmbuild --with=bundled_zipios:  use bundled version of zipios++
%global bundled_zipios %{?_with_bundled_zipios: 1} %{?!_with_bundled_zipios: 0}
# rpmbuild --without=bundled_pycxx:  don't use bundled version of pycxx
%global bundled_pycxx %{?_with_bundled_pycxx: 1} %{?!_with_bundled_pycxx: 0}
# rpmbuild --without=bundled_smesh:  don't use bundled version of Salome's Mesh
%global bundled_smesh %{?_with_bundled_smesh: 0} %{?!_with_bundled_smesh: 1}

%global commit a50ae33557e1992a938542b319cacb8b09d7fb08
%global short %(c=%{commit}; echo ${c:0:10})
%global date 20201125


Name:           freecad
Epoch:          1
Version:        0.19
Release:        0.3.%{date}git%{short}%{?dist}
Summary:        A general purpose 3D CAD modeler

License:        GPLv2+
URL:            http://freecadweb.org/
#Source0:        https://github.com/FreeCAD/FreeCAD/archive/%{version}%{?pre:_pre}/FreeCAD-%{version}%{?pre:_pre}.tar.gz
Source0:        https://github.com/FreeCAD/FreeCAD/archive/%{commit}/FreeCAD-%{version}.%{date}git%{short}.tar.gz
Source102:      freecad.1

Patch0:         freecad-0.15-zipios.patch
Patch1:         freecad-0.14-Version_h.patch
Patch2:         freecad-unbundled-pycxx.patch
Patch3:         freecad-vtk9.patch


# Utilities
BuildRequires:  cmake gcc-c++ gettext dos2unix
BuildRequires:  doxygen swig graphviz
BuildRequires:  gcc-gfortran
BuildRequires:  desktop-file-utils
%ifnarch ppc64
BuildRequires:  tbb-devel
%endif
# Development Libraries
BuildRequires:  freeimage-devel
BuildRequires:  libXmu-devel
BuildRequires:  mesa-libGL-devel
BuildRequires:  mesa-libGLU-devel
BuildRequires:  libglvnd-devel
BuildRequires:  opencascade-devel
%if 0%{?fedora} > 31
BuildRequires:  Coin4-devel
%else
BuildRequires:  Coin3-devel
%endif
BuildRequires:  python3-devel
BuildRequires:  python3-matplotlib
BuildRequires:  boost-devel boost-python3-devel
BuildRequires:  eigen3-devel
# Qt5 dependencies
BuildRequires:  cmake(Qt5Core)
BuildRequires:  cmake(Qt5Svg)
BuildRequires:  cmake(Qt5UiTools)
BuildRequires:  cmake(Qt5WebKit)
BuildRequires:  cmake(Qt5XmlPatterns)
#BuildRequires:  SoQt-devel
BuildRequires:  xerces-c xerces-c-devel
BuildRequires:  libspnav-devel
BuildRequires:  python3-shiboken2-devel
BuildRequires:  python3-pyside2-devel pyside2-tools
%if ! %{bundled_smesh}
BuildRequires:  smesh-devel
%endif
# Does not build with current versions of OCCT.
#BuildRequires:  netgen-mesher-devel
%if ! %{bundled_zipios}
BuildRequires:  zipios++-devel
%endif
%if ! %{bundled_pycxx}
BuildRequires:  python3-pycxx-devel
%endif
BuildRequires:  libicu-devel
BuildRequires:  vtk-devel
#BuildRequires:  openmpi-devel
BuildRequires:  med-devel
BuildRequires:  libkdtree++-devel

# For appdata
%if 0%{?fedora}
BuildRequires:  libappstream-glib
%endif

Requires:       python3-pivy
Requires:       python3-matplotlib
Requires:       python3-collada
Requires:       python3-pyside2
Requires:       qt5-assistant

Requires:       %{name}-data = %{epoch}:%{version}-%{release}

Provides:       bundled(smesh) = 5.1.2.2


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
%autosetup -p1 -n FreeCAD-%{commit}
# Remove bundled pycxx if we're not using it
%if ! %{bundled_pycxx}
rm -rf src/CXX
%endif

%if ! %{bundled_zipios}
rm -rf src/zipios++
#sed -i "s/zipios-config.h/zipios-config.hpp/g" \
#    src/Base/Reader.cpp src/Base/Writer.h
%endif

# Fix encodings
dos2unix -k src/Mod/Test/unittestgui.py \
            ChangeLog.txt \
            data/License.txt


%build
%cmake -DCMAKE_INSTALL_PREFIX=%{_libdir}/%{name} \
       -DCMAKE_INSTALL_DATADIR=%{_datadir}/%{name} \
       -DCMAKE_INSTALL_DOCDIR=%{_docdir}/%{name} \
       -DCMAKE_INSTALL_INCLUDEDIR=%{_includedir} \
       -DRESOURCEDIR=%{_datadir}/%{name} \
       -DPYTHON_EXECUTABLE=%{__python3} \
       -DPYSIDE_INCLUDE_DIR=/usr/include/PySide2 \
       -DPYSIDE_LIBRARY=%{_libdir}/libpyside2.%{py_suffix}.so \
       -DSHIBOKEN_INCLUDE_DIR=%{_includedir}/shiboken2 \
       -DSHIBOKEN_LIBRARY=%{_libdir}/libshiboken2.%{py_suffix}.so \
       -DBUILD_QT5=ON \
       -DOpenGL_GL_PREFERENCE=LEGACY \
%if 0%{?fedora} > 31
       -DCOIN3D_INCLUDE_DIR=%{_includedir}/Coin4 \
       -DCOIN3D_DOC_PATH=%{_datadir}/Coin4/Coin \
%else
       -DCOIN3D_INCLUDE_DIR=%{_includedir}/Coin3 \
       -DCOIN3D_DOC_PATH=%{_datadir}/Coin3/Coin \
%endif
       -DUSE_OCC=TRUE \
%if ! %{bundled_smesh}
       -DFREECAD_USE_EXTERNAL_SMESH=TRUE \
       -DSMESH_INCLUDE_DIR=%{_includedir}/smesh \
%endif
%if ! %{bundled_zipios}
       -DFREECAD_USE_EXTERNAL_ZIPIOS=TRUE \
%endif
%if ! %{bundled_pycxx}
       -DPYCXX_INCLUDE_DIR=$(pkg-config --variable=includedir PyCXX) \
       -DPYCXX_SOURCE_DIR=$(pkg-config --variable=srcdir PyCXX) \
%endif
       -DMEDFILE_INCLUDE_DIRS=%{_includedir}/med

%cmake_build


%install
%cmake_install

# Symlink binaries to /usr/bin
mkdir -p %{buildroot}%{_bindir}
ln -rs %{buildroot}%{_libdir}/freecad/bin/FreeCAD %{buildroot}%{_bindir}
ln -rs %{buildroot}%{_libdir}/freecad/bin/FreeCADCmd %{buildroot}%{_bindir}

# Move mis-installed files to the right location
# Need to figure out if FreeCAD can install correctly at some point.
mkdir -p %{buildroot}%{_datadir}
mv %{buildroot}%{_libdir}/%{name}/share/* \
   %{buildroot}%{_datadir}

# Install man page
install -pD -m 0644 %{SOURCE102} \
    %{buildroot}%{_mandir}/man1/%{name}.1

# Symlink manpage to other binary names
pushd %{buildroot}%{_mandir}/man1
ln -sf %{name}.1.gz FreeCAD.1.gz 
ln -sf %{name}.1.gz FreeCADCmd.1.gz
popd

# Remove obsolete Start_Page.html
rm -f %{buildroot}%{_docdir}/%{name}/Start_Page.html

# Belongs in %%license not %%doc
rm -f %{buildroot}%{_docdir}/freecad/ThirdPartyLibraries.html

# Bytecompile Python modules
%py_byte_compile %{__python3} %{buildroot}%{_libdir}/%{name}

%check
desktop-file-validate \
    %{buildroot}%{_datadir}/applications/org.freecadweb.FreeCAD.desktop
%{?fedora:appstream-util validate-relax --nonet \
    %{buildroot}%{_metainfodir}/*.appdata.xml}


%if 0%{?rhel} < 8
%post
/usr/bin/update-desktop-database &> /dev/null || :
/usr/bin/update-mime-database %{_datadir}/mime &> /dev/null || :

%postun
/usr/bin/update-desktop-database &> /dev/null || :
/usr/bin/update-mime-database %{_datadir}/mime &> /dev/null || :
%endif


%files
%license data/License.txt src/Doc/ThirdPartyLibraries.html
%doc ChangeLog.txt README.md
%exclude %{_docdir}/freecad/freecad.*
%{_bindir}/*
%{_metainfodir}/*.appdata.xml
%{_datadir}/applications/*.desktop
%{_datadir}/icons/hicolor/*/apps/*.png
%{_datadir}/icons/hicolor/scalable/apps/freecad.svg
%{_datadir}/icons/hicolor/scalable/apps/org.freecadweb.FreeCAD.svg
%{_datadir}/icons/hicolor/scalable/mimetypes/application-x-extension-fcstd.svg
%{_datadir}/pixmaps/freecad.xpm
%{_datadir}/mime/packages/*.xml
%{_datadir}/thumbnailers/FreeCAD.thumbnailer
%dir %{_libdir}/%{name}
%{_libdir}/%{name}/bin/
%{_libdir}/%{name}/%{_lib}/
%{_libdir}/%{name}/Ext/
%{_libdir}/%{name}/Mod/
%{_mandir}/man1/*.1.gz

%files data
%{_datadir}/%{name}/
%{_docdir}/%{name}/freecad.q*
