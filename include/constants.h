/*
 * Copyright 2018 Red Hat, Inc.
 * Author(s): David Cantrell <dcantrell@redhat.com>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/**
 * @file constants.h
 * @author David Cantrell &lt;dcantrell@redhat.com&gt;
 * @date 2018-2020
 * @brief Constants and other defaults for librpminspect.
 * @copyright LGPL-3.0-or-later
 */

#ifndef _LIBRPMINSPECT_CONSTANTS_H
#define _LIBRPMINSPECT_CONSTANTS_H

/**
 * @defgroup CONSTANTS Constants and Other Defaults
 *
 * @{
 */

/**
 * @defgroup INTERNAL Internal defaults used throughout librpminspect
 *
 * @{
 */

/**
 * @def DEFAULT_MESSAGE_DIGEST
 *
 * Default message digest to use internally.  The definition comes
 * from an enum in rpminspect.h.
 */
#define DEFAULT_MESSAGE_DIGEST SHA256SUM

/**
 * @def SHARED_LIB_PREFIX
 * Filename prefix for shared libraries.
 */
#define SHARED_LIB_PREFIX "lib"

/**
 * @def SHARED_LIB_SUFFIX
 *
 * Filename suffix for shared libraries.  Note that this is really
 * more a substring that appears in shared library names given how ELF
 * libraries are installed and managed.
 */
#define SHARED_LIB_SUFFIX ".so"

/**
 * @def DEFAULT_TTY_WIDTH
 *
 * Fallback tty width if TIOCGWINSZ fails.
 */
#define DEFAULT_TTY_WIDTH 80

/** @} */

/**
 * @defgroup CONFIG Configuration defaults
 *
 * @{
 */

/**
 * @def COMMAND_NAME
 * The standard name for the rpminspect command.
 */
#define COMMAND_NAME "rpminspect"

/**
 * @def SOFTWARE_NAME
 * Software name.  Used in Koji XMLRPC calls and logging.
 */
#define SOFTWARE_NAME "librpminspect"

/**
 * @def CFGFILE_DIR
 * Configuration file directory.
 */
#define CFGFILE_DIR "/usr/share/rpminspect"

/**
 * @def CFGFILE
 * Default configuration file.
 */
#define CFGFILE "rpminspect.yaml"

/**
 * @def DEFAULT_WORKDIR
 * Default working directory location.  rpminspect will create
 * subdirectories within this directory so multiple concurrent jobs
 * can run.
 */
#define DEFAULT_WORKDIR "/var/tmp/rpminspect"

/**
 * @def VENDOR_DATA_DIR
 * Default location for the vendor-specific data.  These files should
 * be provided by the vendor-specific rpminspect-data package.
 */
#define VENDOR_DATA_DIR "/usr/share/rpminspect"

/**
 * @def INSPECTIONS
 * Name of the [inspections] section in the config file.
 */
#define INSPECTIONS "inspections"

/** @} */

/**
 * @defgroup VENDOR_DATA_SUBDIRS Vendor Data Subdirectories
 *
 * With a few exceptions, the files within vendor data subdirectories
 * will correspond to the product release rpminspect is using rules
 * from.  For example, if the product release is "fc35", then
 * rpminspect will expect files named "fc35" in these subdirectories.
 * If an expected file is not found in the subdirectory, that is the
 * same as rpminspect having no product release rules specific to that
 * data set.
 *
 * The notable exception is the license database, which is named in
 * the rpminspect configuration file and generally corresponds to the
 * vendor and not a specific product release.
 * @{
 */

/**
 * @def ABI_DIR
 * Name of the ABI checking subdirectory in VENDOR_DATA_DIR.
 */
#define ABI_DIR "abi"

/**
 * @def CAPABILITIES_DIR
 * Name of the capabilities(7) subdirectory in VENDOR_DATA_DIR.
 */
#define CAPABILITIES_DIR "capabilities"

/**
 * @def LICENSES_DIR
 * Name of the license database subdirectory in VENDOR_DATA_DIR.
 */
#define LICENSES_DIR "licenses"

/**
 * @def FILEINFO_DIR
 * Name of the fileinfo (stat(2)) list subdirectory in VENDOR_DATA_DIR.
 */
#define FILEINFO_DIR "fileinfo"

/**
 * @def REBASEABLE_DIR
 * Name of the VENDOR_DATA_DIR subdirectory listing rebaseable
 * packages.  A rebaseable package is one where the version number
 * changes between the before and after build but is expected.
 * Putting the package on the rebaseable list prevents rpminspect from
 * reporting an unexpected package rebase.
 */
#define REBASEABLE_DIR "rebaseable"

/**
 * @def POLITICS_DIR
 * Name of the VENDOR_DATA_DIR subdirectory with political related
 * file inclusion and exclusion rules.
 */
#define POLITICS_DIR "politics"

/**
 * @def SECURITY_DIR
 * Name of the VENDOR_DATA_DIR subdirectory with security related
 * package rules.
 */
#define SECURITY_DIR "security"

/**
 * @def ICONS_DIR
 * Name of the VENDOR_DATA_DIR subdirectory with standard icon names
 * defined per product.
 */
#define ICONS_DIR "icons"

/** @} */

/**
 * @defgroup DESKTOP Desktop Constants
 * Constants related to desktop files.  Paths and where to find icons,
 * for instance.
 * @{
 */

/**
 * @def DESKTOP_ENTRY_FILES_DIR
 * Standard location for desktop entry files.
 */
#define DESKTOP_ENTRY_FILES_DIR "/usr/share/applications"

/** @} */

/**
 * @defgroup COMMANDS Commands used by different inspections
 * Avoid explicit paths, rpminspect assumes these commands are
 * available in the $PATH.
 *
 * @{
 */

/**
 * @def MSGUNFMT_CMD
 * Executable providing msgunfmt(1)
 */
#define MSGUNFMT_CMD "msgunfmt"

/**
 * @def DESKTOP_FILE_VALIDATE_CMD
 * Executable providing desktop-file-validate(1)
 */
#define DESKTOP_FILE_VALIDATE_CMD "desktop-file-validate"

/**
 * @def ANNOCHECK_CMD
 * Executable providing annocheck(1)
 */
#define ANNOCHECK_CMD "annocheck"

/**
 * @def ABIDIFF_CMD
 * Executable providing abidiff(1)
 */
#define ABIDIFF_CMD "abidiff"

/**
 * @def ABI_SUPPRESSION
 * The command line option for abidiff(1) to use the suppression
 * specification file in the named file.  rpminspect will find the
 * suppression specification file in t he source package, but needs to
 * know the name of the command line option.
 */
#define ABI_SUPPRESSIONS "--suppressions"

/**
 * @def ABI_DEBUG_INFO_DIR1
 * The command line option for abidiff(1) to specify the location of
 * debugging symbols for files in the before build.  For RPM packages,
 * this is a path to the corresponding directory in a debuginfo
 * package.
 */
#define ABI_DEBUG_INFO_DIR1 "--debug-info-dir1"

/**
 * @def ABI_DEBUG_INFO_DIR2
 * The command line option for abidiff(1) to specify the location of
 * debugging symbols for files in the after build.  For RPM packages,
 * this is a path to the corresponding directory in a debuginfo
 * package.
 */
#define ABI_DEBUG_INFO_DIR2 "--debug-info-dir2"

/**
 * @def ABI_HEADERS_DIR1
 * The command line option for abidiff(1) to specify the location of
 * public headers for the files in the before build.
 */
#define ABI_HEADERS_DIR1 "--headers-dir1"

/**
 * @def ABI_HEADERS_DIR2
 * The command line option for abidiff(1) to specify the location of
 * public headers for the files in the after build.
 */
#define ABI_HEADERS_DIR2 "--headers-dir2"

/**
 * @def KMIDIFF_CMD
 * Executable providing kmidiff(1), the Kernel Module Interface
 * comparison tool.
 */
#define KMIDIFF_CMD "kmidiff"

/**
 * @def KMIDIFF_VMLINUX1
 * The command line option to kmidiff(1) specifying the location of
 * the Linux kernel file in the before build.
 */
#define KMIDIFF_VMLINUX1 "--vmlinux1"

/**
 * @def KMIDIFF_VMLINUX2
 * The command line option to kmidiff(1) specifying the location of
 * the Linux kernel file in the after build.
 */
#define KMIDIFF_VMLINUX2 "--vmlinux2"

/**
 * @def KMIDIFF_KMI_WHITELIST
 * The command line option to kmidiff(1) used to specify the kernel
 * module interface whitelist.  rpminspect will find this file in the
 * source package, but it needs to know the command line option to use
 * with kmidiff(1).
 */
#define KMIDIFF_KMI_WHITELIST "--kmi-whitelist"

/** @} */

/**
 * @defgroup SYSTEM System defaults
 *
 * @{
 */

/**
 * @def SRPM_ARCH_NAME
 * The architecture name Koji uses for source RPMs.
 */
#define SRPM_ARCH_NAME "src"

/**
 * @def RPM_NOARCH_NAME
 * The architecture name Koji uses for binary RPMs built with
 * 'BuildArch: noarch'
 */
#define RPM_NOARCH_NAME "noarch"

/**
 * @def BIN_OWNER
 * Default executable file owner.
 */
#define BIN_OWNER "root"

/**
 * @def BIN_GROUP
 * Default executable file group.
 */
#define BIN_GROUP "root"

/**
 * @def BUILD_ID_DIR
 * Build ID subdirectory name.
 */
#define BUILD_ID_DIR "/.build-id/"

/**
 * @def DEBUGINFO_SUFFIX
 * The debuginfo package name suffix string.
 */
#define DEBUGINFO_SUFFIX "-debuginfo"

/**
 * @def DEBUGSOURCE_SUFFIX
 * The debugsource package name suffix string.
 */
#define DEBUGSOURCE_SUFFIX "-debugsource"

/**
 * @def DEBUG_PATH
 * Where debuginfo packages are installed.
 */
#define DEBUG_PATH "/usr/lib/debug/"

/**
 * @def DEBUG_SRC_PATH
 * Where debugsource packages are installed.
 */
#define DEBUG_SRC_PATH "/usr/src/debug/"

/**
 * @def DEBUG_SUBSTRING
 * The substring that can appear in filenames installed with debugging
 * information.
 */
#define DEBUG_SUBSTRING "debug"

/**
 * @def KERNEL_MODULES_DIR
 * Linux loadable kernel modules subdirectory.
 */
#define KERNEL_MODULES_DIR "/lib/modules/"

/**
 * @def ABIDIFF_SUPPRESSION_FILE
 * Default ABI suppression file in source packages.  If this file is
 * found in a source package, it will be passed to abidiff(1) using
 * the ABI_SUPPRESSIONS option.
 */
#define ABI_SUPPRESSION_FILE ".abignore"

/**
 * @def INCLUDE_PATH
 * Default header file path.
 */
#define INCLUDE_PATH "/usr/include/"

/**
 * @def KERNEL_FILENAMES
 * Default array of kernel executable filename possibilities.
 */
#define KERNEL_FILENAMES { "vmlinux", "vmlinuz", NULL }

/**
 * @def KMI_IGNORE_PATTERN
 * File pattern used to find the Kernel Module Interface ignore list
 * for kmidiff(1).  The substring '${ARCH}' will be replaced with the
 * package architecture.
 */
#define KMI_IGNORE_PATTERN "/lib/modules/kabi-current/kabi_whitelist_${ARCH}"

/** @} */

/**
 * @defgroup EXTENSIONS Filename extensions
 *
 * @{
 */

/**
 * @def RPM_FILENAME_EXTENSION
 * RPM filename extension
 */
#define RPM_FILENAME_EXTENSION ".rpm"

/**
 * @def SPEC_FILENAME_EXTENSION
 * RPM spec filename extension
 */
#define SPEC_FILENAME_EXTENSION ".spec"

/**
 * @def JAR_FILENAME_EXTENSION
 * Java jar filename extension
 */
#define JAR_FILENAME_EXTENSION ".jar"

/**
 * @def CLASS_FILENAME_EXTENSION
 * Java class filename extension
 */
#define CLASS_FILENAME_EXTENSION ".class"

/**
 * @def EGGINFO_FILENAME_EXTENSION
 * Python egg-info filename extension
 */
#define EGGINFO_FILENAME_EXTENSION ".egg-info"

/**
 * @def GZIPPED_FILENAME_EXTENSION
 * Gzip filename extension
 */
#define GZIPPED_FILENAME_EXTENSION ".gz"

/**
 * @def DESKTOP_FILENAME_EXTENSION
 * Desktop filename extension
 */
#define DESKTOP_FILENAME_EXTENSION ".desktop"

/**
 * @def DIRECTORY_FILENAME_EXTENSION
 * Directory filename extension
 */
#define DIRECTORY_FILENAME_EXTENSION ".directory"

/**
 * @def MO_FILENAME_EXTENSION
 * Machine object filename extension (compiled translation data)
 */
#define MO_FILENAME_EXTENSION ".mo"

/**
 * @def PYTHON_PYC_FILE_EXTENSION
 * Python bytecode filename extension
 */
#define PYTHON_PYC_FILE_EXTENSION ".pyc"

/**
 * @def PYTHON_PYO_FILE_EXTENSION
 * Python optimized bytecode filename extension
 */
#define PYTHON_PYO_FILE_EXTENSION ".pyo"

/**
 * @def KERNEL_MODULE_FILENAME_EXTENSION
 * Linux loadable kernel module filename extension.  Note that kernel
 * modules can be compressed with a typical compression appened after
 * this string.  rpminspect can handle those cases.
 */
#define KERNEL_MODULE_FILENAME_EXTENSION ".ko"

/**
 * @def SVG_FILENAME_EXTENSION
 * Scalable vector graphics filename extension
 */
#define SVG_FILENAME_EXTENSION ".svg"

/**
 * @def STATIC_LIB_FILENAME_EXTENSION
 * Static ELF library filename extension
 */
#define STATIC_LIB_FILENAME_EXTENSION ".a"

/**
 * @def ELF_LIB_EXTENSION
 * ELF shared library extension.  Note that this extension appears in
 * the middle of ELF library filenames because a version number comes
 * after it.  For files that end with just '.so', we do not care so
 * much because rpminspect will match the files correctly.  That's why
 * this extension ends with a period.
 */
#define ELF_LIB_EXTENSION ".so."

/** @} */

/**
 * @defgroup SPEC RPM spec file defaults
 *
 * @{
 */

/**
 * @def SPEC_MACRO_DEFINE
 * How macros are defined in spec files.
 */
#define SPEC_MACRO_DEFINE      "%define"

/**
 * @def SPEC_MACRO_GLOBAL
 * How global macros are defined in spec files.
 */
#define SPEC_MACRO_GLOBAL      "%global"

/**
 * @def SPEC_SECTION_CHANGELOG
 * How the change log is specified in the spec file.
 */
#define SPEC_SECTION_CHANGELOG "%changelog"

/**
 * @def SPEC_SECTION_FILES
 * How file listings are specified in the spec file.
 */
#define SPEC_SECTION_FILES     "%files"

/**
 * @def SPEC_TAG_RELEASE
 * The name of RPMTAG_RELEASE in a spec file.
 */
#define SPEC_TAG_RELEASE       "Release:"

/**
 * @def SPEC_DISTTAG
 * The distribution "dist tag" typically used in SPEC_TAG_RELEASE
 * strings.  Common in Fedora Linux and related distributions, but may
 * not be consistently defined in other RPM-based distributions.
 */
#define SPEC_DISTTAG           "%{?dist}"

/** @} */

/**
 * @defgroup RPMBUILD_SUBDIRS Subdirectory names used by rpmbuild(1)
 *
 * Names of rpmbuild(1) subdirectories.  rpminspect models an rpmbuild
 * layout similar to how rpmbuild is used for distributions like
 * Fedora Linux.  The main idea is a top level directory is defined
 * (topdir) and all of the other directories used by rpmbuild are
 * within topdir.  For the purposes of rpminspect, the topdir
 * definition here is a subdirectory in the working directory location
 * rpminspect is using.
 *
 * @{
 */

/**
 * @def RPMBUILD_TOPDIR
 * The rpmbuild(1) top directory.  The directory where all other
 * rpmbuild(1) subdirectories live.
 */
#define RPMBUILD_TOPDIR        "rpmbuild"

/**
 * @def RPMBUILD_BUILDDIR
 * The BUILD subdirectory name under RPMBUILD_TOPDIR.
 */
#define RPMBUILD_BUILDDIR      "BUILD"


/**
 * @def RPMBUILD_BUILDROOTDIR
 * The BUILDROOT subdirectory name under RPMBUILD_TOPDIR.
 */
#define RPMBUILD_BUILDROOTDIR  "BUILDROOT"

/**
 * @def RPMBUILD_RPMDIR
 * The RPMS subdirectory name under RPMBUILD_TOPDIR.
 */
#define RPMBUILD_RPMDIR        "RPMS"

/**
 * @def RPMBUILD_SOURCEDIR
 * The SOURCES subdirectory name under RPMBUILD_TOPDIR.
 */
#define RPMBUILD_SOURCEDIR     "SOURCES"

/**
 * @def RPMBUILD_SPECDIR
 * The SPECS subdirectory name under RPMBUILD_TOPDIR.
 */
#define RPMBUILD_SPECDIR       "SPECS"

/**
 * @def RPMBUILD_SRPMSDIR
 * The SRPMS subdirectory name under RPMBUILD_TOPDIR.
 */
#define RPMBUILD_SRPMDIR       "SRPMS"

/** @} */

/**
 * @defgroup ABI_DEFAULTS abi inspection defaults
 *
 * @{
 */

/**
 * @def DEFAULT_ABI_SECURITY_THRESHOLD
 * Default ABI compat level security reporting threshold.
 */
#define DEFAULT_ABI_SECURITY_THRESHOLD 2

/** @} */

/**
 * @defgroup PATCHES_DEFAULTS patches inspection defaults
 *
 * @{
 */

/**
 * @def DEFAULT_PATCH_FILE_THRESHOLD
 * Default file count reporting threshold for 'patches'
 */
#define DEFAULT_PATCH_FILE_THRESHOLD 20

/**
 * @def DEFAULT_PATCH_LINE_THRESHOLD
 * Default line count reporting threshold for 'patches'
 */
#define DEFAULT_PATCH_LINE_THRESHOLD 5000

/** @} */

/**
 * @defgroup RUNPATH_DEFAULTS runpath inspection defaults
 *
 * @{
 */

/**
 * @def RUNPATH_ORIGIN_STR
 * The value of the DT_RPATH or DT_RUNPATH $ORIGIN string.
 */
#define RUNPATH_ORIGIN_STR "$ORIGIN"

/** @} */

/** @} */

#endif
