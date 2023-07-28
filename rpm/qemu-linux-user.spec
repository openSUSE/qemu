#
# spec file for package qemu-linux-user
#
# Copyright (c) 2023 SUSE LLC
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://bugs.opensuse.org/
#

%include %{_sourcedir}/common.inc

%ifarch %ix86 x86_64 s390x
%define legacy_qemu_kvm 1
%endif

Name:           qemu-linux-user
URL:            https://www.qemu.org/
Summary:        CPU emulator for user space
License:        BSD-2-Clause AND BSD-3-Clause AND GPL-2.0-only AND GPL-2.0-or-later AND LGPL-2.1-or-later AND MIT
Group:          System/Emulators/PC
Version:        %qemuver
Release:        0
Source:         %{srcname}-%{srcver}.tar.xz
Source1:        common.inc
Source303:      README.PACKAGING
Source1000:     qemu-rpmlintrc
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
BuildRequires:  glib2-devel-static >= 2.56
BuildRequires:  glibc-devel-static
BuildRequires:  pcre-devel-static
BuildRequires:  zlib-devel-static
# we must not install the qemu-linux-user package when under QEMU build
%if 0%{?qemu_user_space_build:1}
#!BuildIgnore:  post-build-checks
%endif
BuildRequires:  fdupes
BuildRequires:  gcc-c++
BuildRequires:  git-core
BuildRequires:  meson
BuildRequires:  ninja >= 1.7
BuildRequires:  python3-base >= 3.6
BuildRequires:  python3-setuptools

%description
QEMU provides CPU emulation along with other related capabilities. This package
provides programs to run user space binaries and libraries meant for another
architecture. The syscall interface is intercepted and execution below the
syscall layer occurs on the native hardware and operating system.

%files
%doc README.rst VERSION
%license COPYING COPYING.LIB LICENSE
%_bindir/qemu-aarch64
%_bindir/qemu-aarch64_be
%_bindir/qemu-alpha
%_bindir/qemu-arm
%_bindir/qemu-armeb
%_bindir/qemu-cris
%_bindir/qemu-hexagon
%_bindir/qemu-hppa
%_bindir/qemu-i386
%_bindir/qemu-m68k
%_bindir/qemu-microblaze
%_bindir/qemu-microblazeel
%_bindir/qemu-mips
%_bindir/qemu-mips64
%_bindir/qemu-mips64el
%_bindir/qemu-mipsel
%_bindir/qemu-mipsn32
%_bindir/qemu-mipsn32el
%_bindir/qemu-nios2
%_bindir/qemu-or1k
%_bindir/qemu-ppc
%_bindir/qemu-ppc64
%_bindir/qemu-ppc64le
%_bindir/qemu-riscv32
%_bindir/qemu-riscv64
%_bindir/qemu-s390x
%_bindir/qemu-sh4
%_bindir/qemu-sh4eb
%_bindir/qemu-sparc
%_bindir/qemu-sparc32plus
%_bindir/qemu-sparc64
%_bindir/qemu-x86_64
%_bindir/qemu-xtensa
%_bindir/qemu-xtensaeb
%_bindir/qemu-binfmt
%_bindir/qemu-*-binfmt
%_sbindir/qemu-binfmt-conf.sh

%prep
%autosetup -n %{srcname}-%{srcver}

%build

%if %{legacy_qemu_kvm}
# FIXME: Why are we copying the s390 specific one?
cp %{rpmfilesdir}/supported.s390.txt docs/supported.rst
sed -i '/^\ \ \ about\/index.*/i \ \ \ supported.rst' docs/index.rst
%endif

find . -iname ".git" -exec rm -rf {} +

mkdir -p %blddir
cd %blddir

%srcdir/configure \
	--static \
	--prefix=%_prefix \
	--sysconfdir=%_sysconfdir \
	--libdir=%_libdir \
	--libexecdir=%_libexecdir \
	--localstatedir=%_localstatedir \
	--docdir=%_docdir \
	--firmwarepath=%_datadir/%name \
        --python=%_bindir/python3 \
	--extra-cflags="%{optflags}" \
	--with-git-submodules=ignore \
%if "%{_lto_cflags}" != "%{nil}"
	--enable-lto \
%endif
%if %{with system_membarrier}
	--enable-membarrier \
%else
	--disable-membarrier \
%endif
	--disable-blobs \
	--disable-bochs \
	--disable-capstone \
	--disable-cloop \
	--disable-dmg \
	--disable-docs \
	--disable-fdt \
	--disable-fuzzing \
	--disable-gio \
	--disable-iconv \
	--disable-kvm \
	--disable-libdaxctl \
	--disable-malloc-trim \
	--disable-modules \
	--disable-multiprocess \
	--disable-parallels \
	--disable-pie \
	--disable-plugins \
	--disable-qcow1 \
	--disable-qed \
	--disable-qom-cast-debug \
	--disable-replication \
	--disable-safe-stack \
	--disable-slirp \
	--disable-stack-protector \
	--disable-strip \
	--disable-system --enable-linux-user \
	--disable-tcg-interpreter \
	--disable-tools --disable-guest-agent \
	--disable-tpm \
	--disable-vdi \
	--disable-vhost-crypto \
	--disable-vhost-kernel \
	--disable-vhost-net \
	--disable-vhost-scsi \
	--disable-vhost-user \
	--disable-vhost-user-blk-server \
	--disable-vhost-vsock \
	--disable-vnc \
	--disable-vvfat \
	--enable-coroutine-pool \
        --disable-linux-io-uring \
        --disable-vhost-user-fs \
        --disable-xkbcommon \
	--without-default-devices

echo "=== Content of config-host.mak: ==="
cat config-host.mak
echo "=== ==="

%make_build

%install
cd %blddir

%make_build install DESTDIR=%{buildroot}

rm -rf %{buildroot}%_datadir/qemu/keymaps
unlink %{buildroot}%_datadir/qemu/trace-events-all
install -d -m 755 %{buildroot}%_sbindir
install -m 755 scripts/qemu-binfmt-conf.sh %{buildroot}%_sbindir
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-aarch64-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-aarch64_be-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-alpha-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-arm-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-armeb-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-cris-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-hexagon-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-hppa-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-i386-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-m68k-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-microblaze-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-microblazeel-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-mips-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-mips64-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-mips64el-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-mipsel-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-mipsn32-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-mipsn32el-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-nios2-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-or1k-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-ppc-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-ppc64-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-ppc64le-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-riscv32-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-riscv64-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-s390x-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-sh4-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-sh4eb-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-sparc-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-sparc32plus-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-sparc64-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-x86_64-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-xtensa-binfmt
ln -s qemu-binfmt %{buildroot}%_bindir/qemu-xtensaeb-binfmt

%fdupes -s %{buildroot}

%check
cd %blddir

%ifarch %ix86
%define qemu_arch i386
%endif
%ifarch x86_64
%define qemu_arch x86_64
%endif
%ifarch %arm
%define qemu_arch arm
%endif
%ifarch aarch64
%define qemu_arch aarch64
%endif
%ifarch ppc
%define qemu_arch ppc
%endif
%ifarch ppc64
%define qemu_arch ppc64
%endif
%ifarch ppc64le
%define qemu_arch ppc64le
%endif
%ifarch s390x
%define qemu_arch s390x
%endif

%ifarch %ix86 x86_64 %arm aarch64 ppc ppc64 ppc64le s390x
%ifnarch %arm
%{qemu_arch}-linux-user/qemu-%{qemu_arch} %_bindir/ls > /dev/null
%endif
%endif

%make_build check-softfloat

%changelog
