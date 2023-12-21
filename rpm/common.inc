%define _buildshell /bin/bash

%define sbver   1.16.2_3_gd478f380

%define srcdir %{_builddir}/%buildsubdir
%define blddir %srcdir/build

%define build_x86_firmware 0
%define build_ppc_firmware 0
%define build_opensbi_firmware 0
%define kvm_available 0
%define legacy_qemu_kvm 0
%define force_fit_virtio_pxe_rom 1

%define with_xen 0%{!?_without_xen:1}

%if "%{?distribution}" == ""
%define distro private-build
%else
%define distro %{distribution}
%endif

# Items to exclude in ALP-based products
%if 0%{?suse_version} == 1600
%define with_xen 0
%endif

%bcond_with system_membarrier
%bcond_with malloc_trim

%bcond_with chkqtests

%ifarch x86_64
%bcond_without canokey
%else
%bcond_with canokey
%endif

# non-x86 archs still seem to have some issues with Link Time Optimization
%ifnarch %ix86 x86_64
%define _lto_cflags %{nil}
%endif

%define generic_qemu_description \
QEMU provides full machine emulation and cross architecture usage. It closely\
integrates with KVM and Xen virtualization, allowing for excellent performance.\
Many options are available for defining the emulated environment, including\
traditional devices, direct host device access, and interfaces specific to\
virtualization.

