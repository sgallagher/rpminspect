/*
 * Copyright (C) 2020  Red Hat, Inc.
 * Author(s):  David Cantrell <dcantrell@redhat.com>
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <err.h>
#include <limits.h>
#include <rpm/header.h>
#include <rpm/rpmtag.h>

#ifdef _COMPAT_QUEUE
#include "compat/queue.h"
#else
#include <sys/queue.h>
#endif

#include "rpminspect.h"

/* Globals */
static string_list_t *firstargs = NULL;
static string_list_t *suppressions = NULL;
static struct hsearch_data *debug_info_dir1_table;
static struct hsearch_data *debug_info_dir2_table;
static struct hsearch_data *headers_dir1_table;
static struct hsearch_data *headers_dir2_table;
static abi_list_t *abi = NULL;

static severity_t check_abi(const severity_t sev, const long int threshold, const char *path, const char *pkg, long int *compat)
{
    ENTRY e;
    ENTRY *eptr;
    abi_pkg_entry_t *pkgentry = NULL;
    abi_entry_t *entry = NULL;
    string_entry_t *dsoentry = NULL;
    char pathcopy[PATH_MAX + 1];
    char *basefn = NULL;

    /* no ABI compat level data */
    if (abi == NULL) {
        return sev;
    }

    assert(path != NULL);
    assert(pkg != NULL);

    /* iterate over the ABI compat level structure */
    TAILQ_FOREACH(entry, abi, items) {
        /* look for an entry for this package at this level */
        e.key = (char *) pkg;
        hsearch_r(e, FIND, &eptr, entry->pkgs);

        /* not found, continue */
        if (eptr == NULL) {
            continue;
        }

        /* we have a entry at this level for this package */
        pkgentry = (abi_pkg_entry_t *) eptr->data;
        *compat = entry->level;

        /* is the ABI level above the threshold? */
        if (entry->level > threshold) {
            if (pkgentry->all) {
                return RESULT_INFO;
            } else {
                /* do specific matching on the DSO name */
                TAILQ_FOREACH(dsoentry, pkgentry->dsos, items) {
                    if ((dsoentry->data[0] == '/') && strprefix(path, dsoentry->data)) {
                        return RESULT_INFO;
                    } else {
                        /* do a soft match against the file basename */
                        basefn = strncpy(pathcopy, path, PATH_MAX);
                        assert(basefn != NULL);
                        basefn = basename(pathcopy);
                        assert(basefn != NULL);

                        if (strprefix(basefn, dsoentry->data)) {
                            return RESULT_INFO;
                        }
                    }
                }
            }
        }

        /* this far means a match was found, stop looking */
        break;
    }

    return sev;
}

static bool abidiff_driver(struct rpminspect *ri, rpmfile_entry_t *file) {
    bool result = true;
    bool rebase = false;
    string_list_t *argv = NULL;
    string_list_t *local_suppressions = NULL;
    string_list_t *local_d1 = NULL;
    string_list_t *local_d2 = NULL;
    string_list_t *local_h1 = NULL;
    string_list_t *local_h2 = NULL;
    string_list_t *arglist = NULL;
    string_entry_t *entry = NULL;
    const char *arch = NULL;
    const char *name = NULL;
    ENTRY e;
    ENTRY *eptr;
    int exitcode = 0;
    int status = 0;
    char *cmd = NULL;
    char *details = NULL;
    struct result_params params;
    bool report = false;
    long int compat_level = 0;

    assert(ri != NULL);
    assert(file != NULL);
    assert(firstargs != NULL);

    /* skip source packages */
    if (headerIsSource(file->rpm_header)) {
        return true;
    }

    /* skip anything without a peer */
    if (file->peer_file == NULL) {
        return true;
    }

    /* skip anything that is not an ELF shared library file.  */
    if (!S_ISREG(file->st.st_mode) || !is_elf_shared_library(file->fullpath)) {
        return true;
    }

    /* get the package architecture */
    arch = get_rpm_header_arch(file->rpm_header);

    /* build the abidiff command */
    argv = list_copy(firstargs);

    if (suppressions && !TAILQ_EMPTY(suppressions)) {
        local_suppressions = list_copy(suppressions);
        TAILQ_CONCAT(argv, local_suppressions, items);
    }

    /* debug dir args */
    e.key = (char *) arch;
    hsearch_r(e, FIND, &eptr, debug_info_dir1_table);

    if (eptr != NULL) {
        arglist = (string_list_t *) eptr->data;

        if (arglist && !TAILQ_EMPTY(arglist)) {
            local_d1 = list_copy(arglist);
            TAILQ_CONCAT(argv, local_d1, items);
        }
    }

    e.key = (char *) arch;
    hsearch_r(e, FIND, &eptr, debug_info_dir2_table);

    if (eptr != NULL) {
        arglist = (string_list_t *) eptr->data;

        if (arglist && !TAILQ_EMPTY(arglist)) {
            local_d2 = list_copy(arglist);
            TAILQ_CONCAT(argv, local_d2, items);
        }
    }

    /* header dir args */
/* XXX - not supported yet in abidiff
    e.key = (char *) arch;
    hsearch_r(e, FIND, &eptr, headers_dir1_table);

    if (eptr != NULL) {
        arglist = (string_list_t *) eptr->data;

        if (arglist && !TAILQ_EMPTY(arglist)) {
            local_h1 = list_copy(arglist);
            TAILQ_CONCAT(argv, local_h1, items);
        }
    }

    e.key = (char *) arch;
    hsearch_r(e, FIND, &eptr, headers_dir2_table);

    if (eptr != NULL) {
        arglist = (string_list_t *) eptr->data;

        if (arglist && !TAILQ_EMPTY(arglist)) {
            local_h2 = list_copy(arglist);
            TAILQ_CONCAT(argv, local_h2, items);
        }
    }
*/

    /* the before build */
    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    entry->data = strdup(file->peer_file->fullpath);
    assert(entry->data != NULL);
    TAILQ_INSERT_TAIL(argv, entry, items);

    /* the after build */
    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    entry->data = strdup(file->fullpath);
    assert(entry->data != NULL);
    TAILQ_INSERT_TAIL(argv, entry, items);

    /* run abidiff */
    details = sl_run_cmd(&exitcode, argv);

    /* determine if this is a rebase build */
    rebase = is_rebase(ri);

    /* report the results */
    init_result_params(&params);
    params.header = HEADER_ABIDIFF;
    params.waiverauth = WAIVABLE_BY_ANYONE;
    params.remedy = REMEDY_ABIDIFF;
    params.arch = arch;

    if (!WIFEXITED(exitcode)) {
        cmd = list_to_string(argv, " ");
        xasprintf(&details, "Command: %s", cmd);
        params.msg = _("ABI comparison ended unexpectedly.");
        params.verb = VERB_FAILED;
        params.noun = ABIDIFF_CMD;
        report = true;
    } else {
        status = WEXITSTATUS(exitcode);

        if ((status & ABIDIFF_ERROR) || (status & ABIDIFF_USAGE_ERROR)) {
            params.severity = RESULT_VERIFY;
            params.verb = VERB_FAILED;
            params.noun = ABIDIFF_CMD;
            report = true;
        }

        if (!rebase && (status & ABIDIFF_ABI_CHANGE)) {
            params.severity = RESULT_VERIFY;
            params.verb = VERB_CHANGED;
            params.noun = _("ABI");
            report = true;
        }

        if (!rebase && (status & ABIDIFF_ABI_INCOMPATIBLE_CHANGE)) {
            params.severity = RESULT_BAD;
            params.verb = VERB_CHANGED;
            params.noun = _("ABI");
            report = true;
        }

        /* check the ABI compat level list */
        name = headerGetString(file->rpm_header, RPMTAG_NAME);
        params.severity = check_abi(params.severity, ri->abi_security_threshold, file->localpath, name, &compat_level);

        /* add additional details */
        if (report) {
            if (!strcmp(file->peer_file->localpath, file->localpath)) {
                if (compat_level) {
                    xasprintf(&params.msg, _("Comparing old vs. new version of %s in package %s with ABI compatibility level %ld on %s revealed ABI differences."), file->localpath, name, compat_level, arch);
                } else {
                    xasprintf(&params.msg, _("Comparing old vs. new version of %s in package %s on %s revealed ABI differences."), file->localpath, name, arch);
                }
            } else {
                if (compat_level) {
                    xasprintf(&params.msg, _("Comparing from %s to %s in package %s with ABI compatibility level %ld on %s revealed ABI differences."), file->peer_file->localpath, file->localpath, name, compat_level, arch);
                } else {
                    xasprintf(&params.msg, _("Comparing from %s to %s in package %s on %s revealed ABI differences."), file->peer_file->localpath, file->localpath, name, arch);
                }
            }
        }
    }

    if (report) {
        params.file = file->localpath;
        params.details = details;
        add_result(ri, &params);
        free(params.msg);
        result = false;
    }

    /* cleanup */
    list_free(argv, free);
    free(local_suppressions);
    free(local_d1);
    free(local_d2);
    free(local_h1);
    free(local_h2);
    free(cmd);
    free(details);

    return result;
}

/*
 * Main driver for the 'abidiff' inspection.
 */
bool inspect_abidiff(struct rpminspect *ri) {
    bool result = false;
    string_entry_t *entry = NULL;
    size_t num_arches = 0;
    struct result_params params;

    assert(ri != NULL);

    /* get the ABI compat level data if there is any */
    abi = read_abi(ri->vendor_data_dir, ri->product_release);

    /* if there's a .abignore file in the after SRPM, we need to use it */
    suppressions = get_abi_suppressions(ri, ri->abidiff_suppression_file);

    /* number of architectures in the builds we have, for hash table size */
    num_arches = list_len(ri->arches);

    /* get the debug info dirs */
    debug_info_dir1_table = get_abi_dir_arg(ri, num_arches, DEBUGINFO_SUFFIX, ABI_DEBUG_INFO_DIR1, DEBUG_PATH, BEFORE_BUILD);
    debug_info_dir2_table = get_abi_dir_arg(ri, num_arches, DEBUGINFO_SUFFIX, ABI_DEBUG_INFO_DIR2, DEBUG_PATH, AFTER_BUILD);

    /* get the header dirs */
    headers_dir1_table = get_abi_dir_arg(ri, num_arches, NULL, ABI_HEADERS_DIR1, INCLUDE_PATH, BEFORE_BUILD);
    headers_dir2_table = get_abi_dir_arg(ri, num_arches, NULL, ABI_HEADERS_DIR2, INCLUDE_PATH, AFTER_BUILD);

    /* build the list of first command line arguments */
    firstargs = calloc(1, sizeof(*firstargs));
    assert(firstargs != NULL);
    TAILQ_INIT(firstargs);

    entry = calloc(1, sizeof(*entry));
    assert(entry != NULL);
    entry->data = strdup(ABIDIFF_CMD);
    TAILQ_INSERT_TAIL(firstargs, entry, items);

    if (ri->abidiff_extra_args) {
        entry = calloc(1, sizeof(*entry));
        assert(entry != NULL);
        entry->data = strdup(ri->abidiff_extra_args);
        TAILQ_INSERT_TAIL(firstargs, entry, items);
    }

    /* run the main inspection */
    result = foreach_peer_file(ri, abidiff_driver, true);

    /* clean up */
    free_abi(abi);
    list_free(firstargs, free);
    list_free(suppressions, free);
    free_argv_table(ri, debug_info_dir1_table);
    free_argv_table(ri, debug_info_dir2_table);
    free_argv_table(ri, headers_dir1_table);
    free_argv_table(ri, headers_dir2_table);

    /* report the inspection results */
    if (result) {
        init_result_params(&params);
        params.waiverauth = NOT_WAIVABLE;
        params.header = HEADER_ABIDIFF;
        params.severity = RESULT_OK;
        add_result(ri, &params);
    }

    return result;
}
