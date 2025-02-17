/*
 * Copyright © 2019 Red Hat, Inc.
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

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include "queue.h"
#include "rpminspect.h"
#include "inspect.h"

/*
 * Debugging mode toggle, set at runtime.
 */
bool debug_mode = false;

/*
 * Ensure the array of inspections is only defined once.
 */

struct inspect inspections[] = {
    /*
     * { INSPECT_TYPE (add to inspect.h),
     *   "short name",
     *   bool--true if for single build, false if before&after required,
     *   &function_pointer },
     *
     * NOTE: long descriptions are inspect.h and returned by inspection_desc()
     */
    { INSPECT_LICENSE,       "license",       true,  &inspect_license },
    { INSPECT_EMPTYRPM,      "emptyrpm",      true,  &inspect_emptyrpm },
    { INSPECT_LOSTPAYLOAD,   "lostpayload",   false, &inspect_lostpayload },
    { INSPECT_METADATA,      "metadata",      true,  &inspect_metadata },
    { INSPECT_MANPAGE,       "manpage",       true,  &inspect_manpage },
    { INSPECT_XML,           "xml",           true,  &inspect_xml },
    { INSPECT_ELF,           "elf",           true,  &inspect_elf },
    { INSPECT_DESKTOP,       "desktop",       true,  &inspect_desktop },
    { INSPECT_DISTTAG,       "disttag",       true,  &inspect_disttag },
    { INSPECT_SPECNAME,      "specname",      true,  &inspect_specname },
    { INSPECT_MODULARITY,    "modularity",    true,  &inspect_modularity },
    { INSPECT_JAVABYTECODE,  "javabytecode",  true,  &inspect_javabytecode },
    { INSPECT_CHANGEDFILES,  "changedfiles",  false, &inspect_changedfiles },
    { INSPECT_MOVEDFILES,    "movedfiles",    false, &inspect_movedfiles },
    { INSPECT_REMOVEDFILES,  "removedfiles",  false, &inspect_removedfiles },
    { INSPECT_ADDEDFILES,    "addedfiles",    true,  &inspect_addedfiles },
    { INSPECT_UPSTREAM,      "upstream",      false, &inspect_upstream },
    { INSPECT_OWNERSHIP,     "ownership",     true,  &inspect_ownership },
    { INSPECT_SHELLSYNTAX,   "shellsyntax",   true,  &inspect_shellsyntax },
    { INSPECT_ANNOCHECK,     "annocheck",     true,  &inspect_annocheck },
    { INSPECT_DSODEPS,       "dsodeps",       false, &inspect_dsodeps },
    { INSPECT_FILESIZE,      "filesize",      false, &inspect_filesize },
    { INSPECT_PERMISSIONS,   "permissions",   true,  &inspect_permissions },
    { INSPECT_CAPABILITIES,  "capabilities",  true,  &inspect_capabilities },
    { INSPECT_KMOD,          "kmod",          false, &inspect_kmod },
    { INSPECT_ARCH,          "arch",          false, &inspect_arch },
    { INSPECT_SUBPACKAGES,   "subpackages",   false, &inspect_subpackages },
    { INSPECT_CHANGELOG,     "changelog",     false, &inspect_changelog },
    { INSPECT_PATHMIGRATION, "pathmigration", true,  &inspect_pathmigration },
    { INSPECT_LTO,           "lto",           true,  &inspect_lto },
    { INSPECT_SYMLINKS,      "symlinks",      true,  &inspect_symlinks },
    { INSPECT_FILES,         "files",         true,  &inspect_files },
    { INSPECT_TYPES,         "types",         false, &inspect_types },
    { INSPECT_ABIDIFF,       "abidiff",       false, &inspect_abidiff },
    { INSPECT_KMIDIFF,       "kmidiff",       false, &inspect_kmidiff },
    { INSPECT_CONFIG,        "config",        false, &inspect_config },
    { INSPECT_DOC,           "doc",           false, &inspect_doc },
    { INSPECT_PATCHES,       "patches",       true,  &inspect_patches },
    { INSPECT_VIRUS,         "virus",         true,  &inspect_virus },
    { INSPECT_POLITICS,      "politics",      true,  &inspect_politics },
    { INSPECT_BADFUNCS,      "badfuncs",      true,  &inspect_badfuncs },
    { INSPECT_RUNPATH,       "runpath",       true,  &inspect_runpath },
    { 0, NULL, false, NULL }
};

/**
 * @brief Iterate over each file in each package in a build.
 *
 * Inspect each "after" file in each peer of an inspection.  If the
 * foreach_peer_file_func returns false for any file, the result will
 * be false.  foreach_peer_file_func is run on each file even if an
 * earlier file fails. This allows for multiple errors to be collected
 * for a single inspection.
 *
 * @param ri Pointer to the struct rpminspect used for the program.
 * @param inspection Name of currently running inspection.
 * @param callback Callback function to iterate over each file.
 * @param use_ignore True to skip files that match entries in the
 *        ignore section of the configuration file, false otherwise.
 * @return True if the check_fn passed for each file, false otherwise.
 */
bool foreach_peer_file(struct rpminspect *ri, const char *inspection, foreach_peer_file_func check_fn, bool use_ignore)
{
    rpmpeer_entry_t *peer;
    rpmfile_entry_t *file;
    bool result = true;

    assert(ri != NULL);
    assert(check_fn != NULL);

    TAILQ_FOREACH(peer, ri->peers, items) {
        /* Disappearing subpackages are caught by INSPECT_EMPTYRPM */
        if (peer->after_files == NULL || TAILQ_EMPTY(peer->after_files)) {
            continue;
        }

        TAILQ_FOREACH(file, peer->after_files, items) {
            /* Ignore files we should be ignoring */
            if (use_ignore && ignore_path(ri, inspection, file->localpath, peer->after_root)) {
                continue;
            }

            if (!check_fn(ri, file)) {
                result = false;
            }
        }
    }

    return result;
}

/**
 * @brief Return inspection ID given its name string.
 *
 * @param name Inspection name.
 */
uint64_t inspection_id(const char *name)
{
    if (name == NULL) {
        return INSPECT_NULL;
    } else if (!strcmp(name, NAME_LICENSE)) {
        return INSPECT_LICENSE;
    } else if (!strcmp(name, NAME_EMPTYRPM)) {
        return INSPECT_EMPTYRPM;
    } else if (!strcmp(name, NAME_LOSTPAYLOAD)) {
        return INSPECT_LOSTPAYLOAD;
    } else if (!strcmp(name, NAME_METADATA)) {
        return INSPECT_METADATA;
    } else if (!strcmp(name, NAME_MANPAGE)) {
        return INSPECT_MANPAGE;
    } else if (!strcmp(name, NAME_XML)) {
        return INSPECT_XML;
    } else if (!strcmp(name, NAME_ELF)) {
        return INSPECT_ELF;
    } else if (!strcmp(name, NAME_DESKTOP)) {
        return INSPECT_DESKTOP;
    } else if (!strcmp(name, NAME_DISTTAG)) {
        return INSPECT_DISTTAG;
    } else if (!strcmp(name, NAME_SPECNAME)) {
        return INSPECT_SPECNAME;
    } else if (!strcmp(name, NAME_MODULARITY)) {
        return INSPECT_MODULARITY;
    } else if (!strcmp(name, NAME_JAVABYTECODE)) {
        return INSPECT_JAVABYTECODE;
    } else if (!strcmp(name, NAME_CHANGEDFILES)) {
        return INSPECT_CHANGEDFILES;
    } else if (!strcmp(name, NAME_MOVEDFILES)) {
        return INSPECT_MOVEDFILES;
    } else if (!strcmp(name, NAME_REMOVEDFILES)) {
        return INSPECT_REMOVEDFILES;
    } else if (!strcmp(name, NAME_ADDEDFILES)) {
        return INSPECT_ADDEDFILES;
    } else if (!strcmp(name, NAME_UPSTREAM)) {
        return INSPECT_UPSTREAM;
    } else if (!strcmp(name, NAME_OWNERSHIP)) {
        return INSPECT_OWNERSHIP;
    } else if (!strcmp(name, NAME_SHELLSYNTAX)) {
        return INSPECT_SHELLSYNTAX;
    } else if (!strcmp(name, NAME_ANNOCHECK)) {
        return INSPECT_ANNOCHECK;
    } else if (!strcmp(name, NAME_DSODEPS)) {
        return INSPECT_DSODEPS;
    } else if (!strcmp(name, NAME_FILESIZE)) {
        return INSPECT_FILESIZE;
    } else if (!strcmp(name, NAME_PERMISSIONS)) {
        return INSPECT_PERMISSIONS;
    } else if (!strcmp(name, NAME_CAPABILITIES)) {
        return INSPECT_CAPABILITIES;
    } else if (!strcmp(name, NAME_KMOD)) {
        return INSPECT_KMOD;
    } else if (!strcmp(name, NAME_ARCH)) {
        return INSPECT_ARCH;
    } else if (!strcmp(name, NAME_SUBPACKAGES)) {
        return INSPECT_SUBPACKAGES;
    } else if (!strcmp(name, NAME_CHANGELOG)) {
        return INSPECT_CHANGELOG;
    } else if (!strcmp(name, NAME_PATHMIGRATION)) {
        return INSPECT_PATHMIGRATION;
    } else if (!strcmp(name, NAME_LTO)) {
        return INSPECT_LTO;
    } else if (!strcmp(name, NAME_SYMLINKS)) {
        return INSPECT_SYMLINKS;
    } else if (!strcmp(name, NAME_FILES)) {
        return INSPECT_FILES;
    } else if (!strcmp(name, NAME_TYPES)) {
        return INSPECT_TYPES;
    } else if (!strcmp(name, NAME_ABIDIFF)) {
        return INSPECT_ABIDIFF;
    } else if (!strcmp(name, NAME_KMIDIFF)) {
        return INSPECT_KMIDIFF;
    } else if (!strcmp(name, NAME_CONFIG)) {
        return INSPECT_CONFIG;
    } else if (!strcmp(name, NAME_DOC)) {
        return INSPECT_DOC;
    } else if (!strcmp(name, NAME_PATCHES)) {
        return INSPECT_PATCHES;
    } else if (!strcmp(name, NAME_VIRUS)) {
        return INSPECT_VIRUS;
    } else if (!strcmp(name, NAME_POLITICS)) {
        return INSPECT_POLITICS;
    } else if (!strcmp(name, NAME_BADFUNCS)) {
        return INSPECT_BADFUNCS;
    } else if (!strcmp(name, NAME_RUNPATH)) {
        return INSPECT_RUNPATH;
    } else {
        return INSPECT_NULL;
    }
}

/**
 * @brief Return inspection description string given its ID.
 *
 * Return the long description for the specified inspection.
 *
 * @param id Inspection ID constant.
 */
const char *inspection_desc(const uint64_t inspection)
{
    switch (inspection) {
        case INSPECT_LICENSE:
            return DESC_LICENSE;
        case INSPECT_EMPTYRPM:
            return DESC_EMPTYRPM;
        case INSPECT_LOSTPAYLOAD:
            return DESC_LOSTPAYLOAD;
        case INSPECT_METADATA:
            return DESC_METADATA;
        case INSPECT_MANPAGE:
            return DESC_MANPAGE;
        case INSPECT_XML:
            return DESC_XML;
        case INSPECT_ELF:
            return DESC_ELF;
        case INSPECT_DESKTOP:
            return DESC_DESKTOP;
        case INSPECT_DISTTAG:
            return DESC_DISTTAG;
        case INSPECT_SPECNAME:
            return DESC_SPECNAME;
        case INSPECT_MODULARITY:
            return DESC_MODULARITY;
        case INSPECT_JAVABYTECODE:
            return DESC_JAVABYTECODE;
        case INSPECT_CHANGEDFILES:
            return DESC_CHANGEDFILES;
        case INSPECT_MOVEDFILES:
            return DESC_MOVEDFILES;
        case INSPECT_REMOVEDFILES:
            return DESC_REMOVEDFILES;
        case INSPECT_ADDEDFILES:
            return DESC_ADDEDFILES;
        case INSPECT_UPSTREAM:
            return DESC_UPSTREAM;
        case INSPECT_OWNERSHIP:
            return DESC_OWNERSHIP;
        case INSPECT_SHELLSYNTAX:
            return DESC_SHELLSYNTAX;
        case INSPECT_ANNOCHECK:
            return DESC_ANNOCHECK;
        case INSPECT_DSODEPS:
            return DESC_DSODEPS;
        case INSPECT_FILESIZE:
            return DESC_FILESIZE;
        case INSPECT_PERMISSIONS:
            return DESC_PERMISSIONS;
        case INSPECT_CAPABILITIES:
            return DESC_CAPABILITIES;
        case INSPECT_KMOD:
            return DESC_KMOD;
        case INSPECT_ARCH:
            return DESC_ARCH;
        case INSPECT_SUBPACKAGES:
            return DESC_SUBPACKAGES;
        case INSPECT_CHANGELOG:
            return DESC_CHANGELOG;
        case INSPECT_PATHMIGRATION:
            return DESC_PATHMIGRATION;
        case INSPECT_LTO:
            return DESC_LTO;
        case INSPECT_SYMLINKS:
            return DESC_SYMLINKS;
        case INSPECT_FILES:
            return DESC_FILES;
        case INSPECT_TYPES:
            return DESC_TYPES;
        case INSPECT_ABIDIFF:
            return DESC_ABIDIFF;
        case INSPECT_KMIDIFF:
            return DESC_KMIDIFF;
        case INSPECT_CONFIG:
            return DESC_CONFIG;
        case INSPECT_DOC:
            return DESC_DOC;
        case INSPECT_PATCHES:
            return DESC_PATCHES;
        case INSPECT_VIRUS:
            return DESC_VIRUS;
        case INSPECT_POLITICS:
            return DESC_POLITICS;
        case INSPECT_BADFUNCS:
            return DESC_BADFUNCS;
        case INSPECT_RUNPATH:
            return DESC_RUNPATH;
        default:
            return NULL;
    }
}

/**
 * @brief Return inspection description string given its results
 * header.
 *
 * Return the long description for the specified inspection.
 *
 * @param header Inspection header string.
 */
const char *inspection_header_to_desc(const char *header)
{
    uint64_t i = 0;

    if (!strcmp(header, NAME_METADATA)) {
        i = INSPECT_METADATA;
    } else if (!strcmp(header, NAME_EMPTYRPM)) {
        i = INSPECT_EMPTYRPM;
    } else if (!strcmp(header, NAME_LOSTPAYLOAD)) {
        i = INSPECT_LOSTPAYLOAD;
    } else if (!strcmp(header, NAME_LICENSE)) {
        i = INSPECT_LICENSE;
    } else if (!strcmp(header, NAME_ELF)) {
        i = INSPECT_ELF;
    } else if (!strcmp(header, NAME_MANPAGE)) {
        i = INSPECT_MANPAGE;
    } else if (!strcmp(header, NAME_XML)) {
        i = INSPECT_XML;
    } else if (!strcmp(header, NAME_DESKTOP)) {
        i = INSPECT_DESKTOP;
    } else if (!strcmp(header, NAME_DISTTAG)) {
        i = INSPECT_DISTTAG;
    } else if (!strcmp(header, NAME_SPECNAME)) {
        i = INSPECT_SPECNAME;
    } else if (!strcmp(header, NAME_MODULARITY)) {
        i = INSPECT_MODULARITY;
    } else if (!strcmp(header, NAME_JAVABYTECODE)) {
        i = INSPECT_JAVABYTECODE;
    } else if (!strcmp(header, NAME_CHANGEDFILES)) {
        i = INSPECT_CHANGEDFILES;
    } else if (!strcmp(header, NAME_MOVEDFILES)) {
        i = INSPECT_MOVEDFILES;
    } else if (!strcmp(header, NAME_REMOVEDFILES)) {
        i = INSPECT_REMOVEDFILES;
    } else if (!strcmp(header, NAME_ADDEDFILES)) {
        i = INSPECT_ADDEDFILES;
    } else if (!strcmp(header, NAME_UPSTREAM)) {
        i = INSPECT_UPSTREAM;
    } else if (!strcmp(header, NAME_OWNERSHIP)) {
        i = INSPECT_OWNERSHIP;
    } else if (!strcmp(header, NAME_SHELLSYNTAX)) {
        i = INSPECT_SHELLSYNTAX;
    } else if (!strcmp(header, NAME_DSODEPS)) {
        i = INSPECT_DSODEPS;
    } else if (!strcmp(header, NAME_FILESIZE)) {
        i = INSPECT_FILESIZE;
    } else if (!strcmp(header, NAME_PERMISSIONS)) {
        i = INSPECT_PERMISSIONS;
    } else if (!strcmp(header, NAME_CAPABILITIES)) {
        i = INSPECT_CAPABILITIES;
    } else if (!strcmp(header, NAME_KMOD)) {
        i = INSPECT_KMOD;
    } else if (!strcmp(header, NAME_ARCH)) {
        i = INSPECT_ARCH;
    } else if (!strcmp(header, NAME_SUBPACKAGES)) {
        i = INSPECT_SUBPACKAGES;
    } else if (!strcmp(header, NAME_CHANGELOG)) {
        i = INSPECT_CHANGELOG;
    } else if (!strcmp(header, NAME_PATHMIGRATION)) {
        i = INSPECT_PATHMIGRATION;
    } else if (!strcmp(header, NAME_LTO)) {
        i = INSPECT_LTO;
    } else if (!strcmp(header, NAME_SYMLINKS)) {
        i = INSPECT_SYMLINKS;
    } else if (!strcmp(header, NAME_FILES)) {
        i = INSPECT_FILES;
    } else if (!strcmp(header, NAME_TYPES)) {
        i = INSPECT_TYPES;
    } else if (!strcmp(header, NAME_ABIDIFF)) {
        i = INSPECT_ABIDIFF;
    } else if (!strcmp(header, NAME_KMIDIFF)) {
        i = INSPECT_KMIDIFF;
    } else if (!strcmp(header, NAME_CONFIG)) {
        i = INSPECT_CONFIG;
    } else if (!strcmp(header, NAME_DOC)) {
        i = INSPECT_DOC;
    } else if (!strcmp(header, NAME_PATCHES)) {
        i = INSPECT_PATCHES;
    } else if (!strcmp(header, NAME_VIRUS)) {
        i = INSPECT_VIRUS;
    } else if (!strcmp(header, NAME_POLITICS)) {
        i = INSPECT_POLITICS;
    } else if (!strcmp(header, NAME_BADFUNCS)) {
        i = INSPECT_BADFUNCS;
    } else if (!strcmp(header, NAME_RUNPATH)) {
        i = INSPECT_RUNPATH;
    }

    return inspection_desc(i);
}
