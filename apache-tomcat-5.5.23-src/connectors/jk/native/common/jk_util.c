/*
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/***************************************************************************
 * Description: Utility functions (mainly configuration)                   *
 * Author:      Gal Shachor <shachor@il.ibm.com>                           *
 * Author:      Henri Gomez <hgomez@apache.org>                            *
 * Author:      Rainer Jung <rjung@apache.org>                             *
 * Version:     $Revision: 511326 $                                          *
 ***************************************************************************/


#include "jk_util.h"
#include "jk_ajp12_worker.h"
#include "jk_ajp13_worker.h"
#include "jk_ajp14_worker.h"
#include "jk_lb_worker.h"
#include "jk_mt.h"

#define SYSPROPS_OF_WORKER          ("sysprops")
#define STDERR_OF_WORKER            ("stderr")
#define STDOUT_OF_WORKER            ("stdout")
#define SECRET_OF_WORKER            ("secret")
#define MX_OF_WORKER                ("mx")
#define MS_OF_WORKER                ("ms")
#define CP_OF_WORKER                ("class_path")
#define BRIDGE_OF_WORKER            ("bridge")
#define JVM_OF_WORKER               ("jvm_lib")
#define LIBPATH_OF_WORKER           ("ld_path")
#define CMD_LINE_OF_WORKER          ("cmd_line")
#define NATIVE_LIB_OF_WORKER        ("native_lib")
#define HOST_OF_WORKER              ("host")
#define PORT_OF_WORKER              ("port")
#define TYPE_OF_WORKER              ("type")
#define CACHE_OF_WORKER_DEPRECATED  ("cachesize")
#define CACHE_OF_WORKER             ("connection_pool_size")
#define CACHE_OF_WORKER_MIN         ("connection_pool_minsize")
#define CACHE_TIMEOUT_DEPRECATED    ("cache_timeout")
#define CACHE_TIMEOUT_OF_WORKER     ("connection_pool_timeout")
#define RECOVERY_OPTS_OF_WORKER     ("recovery_options")
#define CONNECT_TIMEOUT_OF_WORKER   ("connect_timeout")
#define PREPOST_TIMEOUT_OF_WORKER   ("prepost_timeout")
#define REPLY_TIMEOUT_OF_WORKER     ("reply_timeout")
#define SOCKET_TIMEOUT_OF_WORKER    ("socket_timeout")
#define SOCKET_BUFFER_OF_WORKER     ("socket_buffer")
#define SOCKET_KEEPALIVE_OF_WORKER  ("socket_keepalive")
#define RECYCLE_TIMEOUT_DEPRECATED  ("recycle_timeout")
#define LOAD_FACTOR_OF_WORKER       ("lbfactor")
#define DISTANCE_OF_WORKER          ("distance")
#define BALANCED_WORKERS_DEPRECATED ("balanced_workers")
#define BALANCE_WORKERS             ("balance_workers")
#define STICKY_SESSION              ("sticky_session")
#define STICKY_SESSION_FORCE        ("sticky_session_force")
#define JVM_ROUTE_OF_WORKER_DEPRECATED ("jvm_route")
#define ROUTE_OF_WORKER             ("route")
#define DOMAIN_OF_WORKER            ("domain")
#define REDIRECT_OF_WORKER          ("redirect")
#define MOUNT_OF_WORKER             ("mount")
#define METHOD_OF_WORKER            ("method")
#define LOCK_OF_WORKER              ("lock")
#define IS_WORKER_DISABLED_DEPRECATED ("disabled")
#define IS_WORKER_STOPPED_DEPRECATED  ("stopped")
#define ACTIVATION_OF_WORKER        ("activation")
#define WORKER_RECOVER_TIME         ("recover_time")
#define WORKER_MAX_PACKET_SIZE      ("max_packet_size")
#define STYLE_SHEET_OF_WORKER       ("css")
#define NAMESPACE_OF_WORKER         ("ns")
#define XML_NAMESPACE_OF_WORKER     ("xmlns")
#define XML_DOCTYPE_OF_WORKER       ("doctype")
#define PROP_PREFIX_OF_WORKER       ("prefix")

#define READ_ONLY_OF_WORKER         ("read_only")
#define USER_OF_WORKER              ("user")
#define USER_CASE_OF_WORKER         ("user_case_insensitive")
#define GOOD_RATING_OF_WORKER       ("good")
#define BAD_RATING_OF_WORKER        ("bad")

#define DEFAULT_WORKER_TYPE         JK_AJP13_WORKER_NAME
#define SECRET_KEY_OF_WORKER        ("secretkey")
#define RETRIES_OF_WORKER           ("retries")
#define STATUS_FAIL_OF_WORKER       ("fail_on_status")

#define DEFAULT_WORKER              JK_AJP13_WORKER_NAME
#define WORKER_LIST_PROPERTY_NAME     ("worker.list")
#define LIST_PROPERTY_NAME            ("list")
#define WORKER_MAINTAIN_PROPERTY_NAME ("worker.maintain")
#define MAINTAIN_PROPERTY_NAME        ("maintain")
#define DEFAULT_MAINTAIN_TIME       (60)
#define DEFAULT_LB_FACTOR           (1)
#define DEFAULT_DISTANCE            (0)

#define TOMCAT32_BRIDGE_NAME        ("tomcat32")
#define TOMCAT33_BRIDGE_NAME        ("tomcat33")
#define TOMCAT40_BRIDGE_NAME        ("tomcat40")
#define TOMCAT41_BRIDGE_NAME        ("tomcat41")
#define TOMCAT50_BRIDGE_NAME        ("tomcat5")

#define HUGE_BUFFER_SIZE (8*1024)
#define LOG_LINE_SIZE    (1024)

#define MAKE_WORKER_PARAM(P)     \
        strcpy(buf, "worker.");  \
        strcat(buf, wname);      \
        strcat(buf, ".");        \
        strcat(buf, P)

/*
 * define the log format, we're using by default the one from error.log
 *
 * [Mon Mar 26 19:44:48 2001] [jk_uri_worker_map.c (155)]: Into jk_uri_worker_map_t::uri_worker_map_alloc
 * log format used by apache in error.log
 */
#ifndef JK_TIME_FORMAT
#define JK_TIME_FORMAT "[%a %b %d %H:%M:%S %Y] "
#endif

/* Visual C++ Toolkit 2003 support */
#if defined (_MSC_VER) && (_MSC_VER == 1310)
    extern long _ftol(double); /* defined by VC6 C libs */
    extern long _ftol2(double dblSource) { return _ftol(dblSource); }
#endif

static const char *list_properties[] = {
    BALANCE_WORKERS,
    MOUNT_OF_WORKER,
    USER_OF_WORKER,
    GOOD_RATING_OF_WORKER,
    BAD_RATING_OF_WORKER,
    "list",
    NULL
};

static const char *unique_properties[] = {
    SECRET_OF_WORKER,
    HOST_OF_WORKER,
    PORT_OF_WORKER,
    TYPE_OF_WORKER,
    CACHE_OF_WORKER_DEPRECATED,
    CACHE_OF_WORKER,
    CACHE_OF_WORKER_MIN,
    CACHE_TIMEOUT_DEPRECATED,
    CACHE_TIMEOUT_OF_WORKER,
    RECOVERY_OPTS_OF_WORKER,
    CONNECT_TIMEOUT_OF_WORKER,
    PREPOST_TIMEOUT_OF_WORKER,
    REPLY_TIMEOUT_OF_WORKER,
    SOCKET_TIMEOUT_OF_WORKER,
    SOCKET_BUFFER_OF_WORKER,
    SOCKET_KEEPALIVE_OF_WORKER,
    RECYCLE_TIMEOUT_DEPRECATED,
    LOAD_FACTOR_OF_WORKER,
    STICKY_SESSION,
    STICKY_SESSION_FORCE,
    JVM_ROUTE_OF_WORKER_DEPRECATED,
    ROUTE_OF_WORKER,
    DOMAIN_OF_WORKER,
    REDIRECT_OF_WORKER,
    METHOD_OF_WORKER,
    LOCK_OF_WORKER,
    IS_WORKER_DISABLED_DEPRECATED,
    IS_WORKER_STOPPED_DEPRECATED,
    ACTIVATION_OF_WORKER,
    WORKER_RECOVER_TIME,
    WORKER_MAX_PACKET_SIZE,
    STYLE_SHEET_OF_WORKER,
    READ_ONLY_OF_WORKER,
    RETRIES_OF_WORKER,
    WORKER_MAINTAIN_PROPERTY_NAME,
    NAMESPACE_OF_WORKER,
    XML_NAMESPACE_OF_WORKER,
    XML_DOCTYPE_OF_WORKER,
    PROP_PREFIX_OF_WORKER,
    STATUS_FAIL_OF_WORKER,
    USER_CASE_OF_WORKER,
    NULL
};

static const char *deprecated_properties[] = {
    SYSPROPS_OF_WORKER,
    STDERR_OF_WORKER,
    STDOUT_OF_WORKER,
    MX_OF_WORKER,
    MS_OF_WORKER,
    CP_OF_WORKER,
    BRIDGE_OF_WORKER,
    JVM_OF_WORKER,
    LIBPATH_OF_WORKER,
    CMD_LINE_OF_WORKER,
    NATIVE_LIB_OF_WORKER,
    CACHE_OF_WORKER_DEPRECATED,
    CACHE_TIMEOUT_DEPRECATED,
    RECYCLE_TIMEOUT_DEPRECATED,
    BALANCED_WORKERS_DEPRECATED,
    JVM_ROUTE_OF_WORKER_DEPRECATED,
    IS_WORKER_DISABLED_DEPRECATED,
    IS_WORKER_STOPPED_DEPRECATED,
    NULL
};

static const char *supported_properties[] = {
    SYSPROPS_OF_WORKER,
    STDERR_OF_WORKER,
    STDOUT_OF_WORKER,
    SECRET_OF_WORKER,
    MX_OF_WORKER,
    MS_OF_WORKER,
    CP_OF_WORKER,
    BRIDGE_OF_WORKER,
    JVM_OF_WORKER,
    LIBPATH_OF_WORKER,
    CMD_LINE_OF_WORKER,
    NATIVE_LIB_OF_WORKER,
    HOST_OF_WORKER,
    PORT_OF_WORKER,
    TYPE_OF_WORKER,
    CACHE_OF_WORKER_DEPRECATED,
    CACHE_OF_WORKER,
    CACHE_OF_WORKER_MIN,
    CACHE_TIMEOUT_DEPRECATED,
    CACHE_TIMEOUT_OF_WORKER,
    RECOVERY_OPTS_OF_WORKER,
    CONNECT_TIMEOUT_OF_WORKER,
    PREPOST_TIMEOUT_OF_WORKER,
    REPLY_TIMEOUT_OF_WORKER,
    SOCKET_TIMEOUT_OF_WORKER,
    SOCKET_BUFFER_OF_WORKER,
    SOCKET_KEEPALIVE_OF_WORKER,
    RECYCLE_TIMEOUT_DEPRECATED,
    LOAD_FACTOR_OF_WORKER,
    DISTANCE_OF_WORKER,
    BALANCED_WORKERS_DEPRECATED,
    BALANCE_WORKERS,
    STICKY_SESSION,
    STICKY_SESSION_FORCE,
    JVM_ROUTE_OF_WORKER_DEPRECATED,
    ROUTE_OF_WORKER,
    DOMAIN_OF_WORKER,
    REDIRECT_OF_WORKER,
    MOUNT_OF_WORKER,
    METHOD_OF_WORKER,
    LOCK_OF_WORKER,
    IS_WORKER_DISABLED_DEPRECATED,
    IS_WORKER_STOPPED_DEPRECATED,
    ACTIVATION_OF_WORKER,
    WORKER_RECOVER_TIME,
    WORKER_MAX_PACKET_SIZE,
    STYLE_SHEET_OF_WORKER,
    NAMESPACE_OF_WORKER,
    XML_NAMESPACE_OF_WORKER,
    XML_DOCTYPE_OF_WORKER,
    PROP_PREFIX_OF_WORKER,
    READ_ONLY_OF_WORKER,
    USER_OF_WORKER,
    USER_CASE_OF_WORKER,
    GOOD_RATING_OF_WORKER,
    BAD_RATING_OF_WORKER,
    SECRET_KEY_OF_WORKER,
    RETRIES_OF_WORKER,
    STATUS_FAIL_OF_WORKER,
    LIST_PROPERTY_NAME,
    MAINTAIN_PROPERTY_NAME
};

/* All entries need to have fixed length 8 chars! */
static const char *jk_level_verbs[] = {
    "[" JK_LOG_TRACE_VERB "] ",
    "[" JK_LOG_DEBUG_VERB "] ",
    "[" JK_LOG_INFO_VERB "]  ",
    "[" JK_LOG_WARN_VERB "]  ",
    "[" JK_LOG_ERROR_VERB "] ",
    "[" JK_LOG_EMERG_VERB "] ",
    NULL
};

const char *jk_get_bool(int v)
{
    if (v == 0)
        return "False";
    else
        return "True";
}

int jk_get_bool_code(const char *v, int def)
{
    if (!v) {
        return def;
    }
    else if (!strcasecmp(v, "off") ||
             *v == 'F' || *v == 'f' ||
             *v == 'N' || *v == 'n' ||
             *v == '0') {
        return 0;
    }
    else if (!strcasecmp(v, "on") ||
             *v == 'T' || *v == 't' ||
             *v == 'Y' || *v == 'y' ||
             *v == '1') {
        return 1;
    }
    return def;
}

/* Sleep for 100ms */
void jk_sleep(int ms)
{
#ifdef OS2
    DosSleep(ms);
#elif defined(BEOS)
    snooze(ms * 1000);
#elif defined(NETWARE)
    delay(ms);
#elif defined(WIN32)
    Sleep(ms);
#else
    struct timeval tv;
    tv.tv_usec = ms * 1000;
    tv.tv_sec = 0;
    select(0, NULL, NULL, NULL, &tv);
#endif
}

static int set_time_str(char *str, int len, const char *jk_log_fmt)
{
    time_t t = time(NULL);
    struct tm *tms;

    tms = localtime(&t);
    if (jk_log_fmt)
        return (int)strftime(str, len, jk_log_fmt, tms);
    return (int)strftime(str, len, JK_TIME_FORMAT, tms);
}

/* Write at most n characters to the buffer in str, return the
 * number of chars written or -1 if the buffer would have been
 * overflowed.
 *
 * This is portable to any POSIX-compliant system that has /dev/null
 */
#if !defined(HAVE_VSNPRINTF) && !defined(HAVE_APR)
static FILE *f = NULL;
static int vsnprintf(char *str, size_t n, const char *fmt, va_list ap)
{
    int res;

    if (f == NULL)
        f = fopen("/dev/null", "w");
    if (f == NULL)
        return -1;

    setvbuf(f, str, _IOFBF, n);

    res = vfprintf(f, fmt, ap);

    if (res > 0 && res < n) {
        res = vsprintf(str, fmt, ap);
    }
    return res;
}
#endif
#if !defined(HAVE_SNPRINTF) && !defined(HAVE_APR)
static int snprintf(char *str, size_t n, const char *fmt, ...)
{
    va_list ap;
    int res;

    va_start(ap, fmt);
    res = vsnprintf(str, n, fmt, ap);
    va_end(ap);
    return res;
}
#endif

static int JK_METHOD log_to_file(jk_logger_t *l, int level, const char *what)
{
    if (l &&
        (l->level <= level || level == JK_LOG_REQUEST_LEVEL) &&
        l->logger_private && what) {
        size_t sz = strlen(what);
        if (sz) {
            file_logger_t *p = l->logger_private;
            if (fwrite(what, 1, sz, p->logfile)) {
                /* [V] Flush the dam' thing! */
                fflush(p->logfile);
            }
        }

        return JK_TRUE;
    }

    return JK_FALSE;
}

int jk_parse_log_level(const char *level)
{
    if (0 == strcasecmp(level, JK_LOG_TRACE_VERB)) {
        return JK_LOG_TRACE_LEVEL;
    }

    if (0 == strcasecmp(level, JK_LOG_DEBUG_VERB)) {
        return JK_LOG_DEBUG_LEVEL;
    }

    if (0 == strcasecmp(level, JK_LOG_INFO_VERB)) {
        return JK_LOG_INFO_LEVEL;
    }

    if (0 == strcasecmp(level, JK_LOG_WARN_VERB)) {
        return JK_LOG_WARNING_LEVEL;
    }

    if (0 == strcasecmp(level, JK_LOG_ERROR_VERB)) {
        return JK_LOG_ERROR_LEVEL;
    }

    if (0 == strcasecmp(level, JK_LOG_EMERG_VERB)) {
        return JK_LOG_EMERG_LEVEL;
    }

    return JK_LOG_DEF_LEVEL;
}

int jk_open_file_logger(jk_logger_t **l, const char *file, int level)
{
    if (l && file) {
        jk_logger_t *rc = (jk_logger_t *)malloc(sizeof(jk_logger_t));
        file_logger_t *p = (file_logger_t *) malloc(sizeof(file_logger_t));
        if (rc && p) {
            rc->log = log_to_file;
            rc->level = level;
            rc->log_fmt = NULL;
            rc->logger_private = p;
#ifdef AS400
            p->logfile = fopen(file, "a+, o_ccsid=0");
#else
            p->logfile = fopen(file, "a+");
#endif
            if (p->logfile) {
                *l = rc;
                return JK_TRUE;
            }
        }
        if (rc) {
            free(rc);
        }
        if (p) {
            free(p);
        }

        *l = NULL;
    }
    return JK_FALSE;
}

int jk_close_file_logger(jk_logger_t **l)
{
    if (l && *l) {
        file_logger_t *p = (*l)->logger_private;
        if (p) {
            fflush(p->logfile);
            fclose(p->logfile);
            free(p);
        }
        free(*l);
        *l = NULL;

        return JK_TRUE;
    }
    return JK_FALSE;
}

int jk_log(jk_logger_t *l,
           const char *file, int line, const char *funcname, int level,
           const char *fmt, ...)
{
    int rc = 0;
    /* Need to reserve space for newline and terminating zero byte. */
#ifdef WIN32
    static int usable_size = HUGE_BUFFER_SIZE-3;
#else
    static int usable_size = HUGE_BUFFER_SIZE-2;
#endif
    if (!l || !file || !fmt) {
        return -1;
    }

    if ((l->level <= level) || (level == JK_LOG_REQUEST_LEVEL)) {
#ifdef NETWARE
        /* On NetWare, this can get called on a thread that has a limited stack so */
        /* we will allocate and free the temporary buffer in this function         */
        char *buf;
#else
        char buf[HUGE_BUFFER_SIZE];
#endif
        char *f = (char *)(file + strlen(file) - 1);
        va_list args;
        int used = 0;

        while (f != file && '\\' != *f && '/' != *f) {
            f--;
        }
        if (f != file) {
            f++;
        }

#ifdef NETWARE
        buf = (char *)malloc(HUGE_BUFFER_SIZE);
        if (NULL == buf)
            return -1;
#endif
        used = set_time_str(buf, usable_size, l->log_fmt);

        if (line) {
            /* Log [pid:threadid] for all levels except REQUEST. */
            /* This information helps to correlate lines from different logs. */
            /* Performance is no issue, because with production log levels */
            /* we only call it often, if we have a lot of errors */
#ifdef USE_SPRINTF              /* until we get a snprintf function */
            rc = sprintf(&buf[used], "[%04d:%04d] ", getpid(),
                            jk_gettid());
#else
            rc = snprintf(&buf[used], usable_size - used,
                             "[%04d:%04d] ", getpid(), jk_gettid());
#endif
            used += rc;
            if (rc < 0 || usable_size - used < 8) {
                return 0;
            }
            strcat(buf, jk_level_verbs[level]);
            used += 8;

            if (funcname) {
                rc = (int)strlen(funcname) + 2;
                if (usable_size - used >= rc) {
                    strcat(buf, funcname);
                    strcat(buf, "::");
                    used += rc;
                }
            }

#ifdef USE_SPRINTF              /* until we get a snprintf function */
            rc = sprintf(&buf[used], "%s (%d): ", f, line);
#else
            rc = snprintf(&buf[used], usable_size - used,
                             "%s (%d): ", f, line);
#endif
            used += rc;
            if (rc < 0 || usable_size - used < 0) {
                return 0;           /* [V] not sure what to return... */
            }
        }

        va_start(args, fmt);
#ifdef USE_VSPRINTF             /* until we get a vsnprintf function */
        rc = vsprintf(buf + used, fmt, args);
#else
        rc = vsnprintf(buf + used, usable_size - used, fmt, args);
#endif
        if ( rc <= usable_size - used ) {
            used += rc;
        } else {
            used = usable_size;
        }
#ifdef WIN32
        buf[used++] = '\r';
#endif
        buf[used] = '\n';
        buf[used+1] = 0;
        va_end(args);
        l->log(l, level, buf);
#ifdef NETWARE
        free(buf);
#endif
    }

    return rc;
}

const char *jk_get_worker_type(jk_map_t *m, const char *wname)
{
    char buf[1024];

    if (!m || !wname) {
        return NULL;
    }
    MAKE_WORKER_PARAM(TYPE_OF_WORKER);
    return jk_map_get_string(m, buf, DEFAULT_WORKER_TYPE);
}

const char *jk_get_worker_route(jk_map_t *m, const char *wname, const char *def)
{
    char buf[1024];
    const char *v;
    if (!m || !wname) {
        return NULL;
    }
    MAKE_WORKER_PARAM(ROUTE_OF_WORKER);
    v = jk_map_get_string(m, buf, NULL);
    if (v) {
        return v;
    }
    /* Try old jvm_route directive */
    MAKE_WORKER_PARAM(JVM_ROUTE_OF_WORKER_DEPRECATED);
    return jk_map_get_string(m, buf, def);
}

const char *jk_get_worker_domain(jk_map_t *m, const char *wname, const char *def)
{
    char buf[1024];
    if (!m || !wname) {
        return NULL;
    }
    MAKE_WORKER_PARAM(DOMAIN_OF_WORKER);
    return jk_map_get_string(m, buf, def);
}

const char *jk_get_worker_redirect(jk_map_t *m, const char *wname, const char *def)
{
    char buf[1024];
    if (!m || !wname) {
        return NULL;
    }
   MAKE_WORKER_PARAM(REDIRECT_OF_WORKER);
    return jk_map_get_string(m, buf, def);
}

const char *jk_get_worker_secret(jk_map_t *m, const char *wname)
{
    char buf[1024];

    if (!m || !wname) {
        return NULL;
    }

    MAKE_WORKER_PARAM(SECRET_OF_WORKER);

    return jk_map_get_string(m, buf, NULL);
}

/* [V] I suggest that the following general purpose functions be used.       */
/*     More should be added (double etc.), but now these were enough for me. */
/*     Functions that can be simulated with these should be "deprecated".    */

int jk_get_worker_str_prop(jk_map_t *m,
                           const char *wname, const char *pname, const char **prop)
{
    char buf[1024];

    if (m && prop && wname && pname) {
        MAKE_WORKER_PARAM(pname);
        *prop = jk_map_get_string(m, buf, NULL);
        if (*prop) {
            return JK_TRUE;
        }
    }
    return JK_FALSE;
}

int jk_get_worker_int_prop(jk_map_t *m,
                           const char *wname, const char *pname, int *prop)
{
    char buf[1024];

    if (m && prop && wname && pname) {
        int i;
        MAKE_WORKER_PARAM(pname);
        i = jk_map_get_int(m, buf, -1);
        if (-1 != i) {
            *prop = i;
            return JK_TRUE;
        }
    }
    return JK_FALSE;
}

const char *jk_get_worker_host(jk_map_t *m, const char *wname, const char *def)
{
    char buf[1024];

    if (!m || !wname) {
        return NULL;
    }

    MAKE_WORKER_PARAM(HOST_OF_WORKER);

    return jk_map_get_string(m, buf, def);
}

int jk_get_worker_port(jk_map_t *m, const char *wname, int def)
{
    char buf[1024];

    if (!m || !wname) {
        return -1;
    }

    MAKE_WORKER_PARAM(PORT_OF_WORKER);

    return jk_map_get_int(m, buf, def);
}

static int def_cache_size = -1;
int jk_get_worker_def_cache_size(int protocol)
{
    if (def_cache_size < 1) {
        if (protocol == AJP14_PROTO)
            def_cache_size = AJP14_DEF_CACHE_SZ;
        else
            def_cache_size = AJP13_DEF_CACHE_SZ;
    }
    return def_cache_size;
}

void jk_set_worker_def_cache_size(int sz)
{
    def_cache_size = sz;
}

int jk_get_worker_cache_size(jk_map_t *m, const char *wname, int def)
{
    char buf[1024];
    int rv;

    if (!m || !wname) {
        return -1;
    }

    MAKE_WORKER_PARAM(CACHE_OF_WORKER);
    if ((rv = jk_map_get_int(m, buf, -1)) >= 0)
        return rv;
    MAKE_WORKER_PARAM(CACHE_OF_WORKER_DEPRECATED);
    return jk_map_get_int(m, buf, def);
}

int jk_get_worker_cache_size_min(jk_map_t *m, const char *wname, int def)
{
    char buf[1024];

    if (!m || !wname) {
        return -1;
    }

    MAKE_WORKER_PARAM(CACHE_OF_WORKER_MIN);
    return jk_map_get_int(m, buf, def);
}

int jk_get_worker_socket_timeout(jk_map_t *m, const char *wname, int def)
{
    char buf[1024];

    if (!m || !wname) {
        return -1;
    }

    MAKE_WORKER_PARAM(SOCKET_TIMEOUT_OF_WORKER);

    return jk_map_get_int(m, buf, def);
}

int jk_get_worker_recover_timeout(jk_map_t *m, const char *wname, int def)
{
    char buf[1024];

    if (!m || !wname) {
        return -1;
    }

    MAKE_WORKER_PARAM(WORKER_RECOVER_TIME);

    return jk_map_get_int(m, buf, def);
}

int jk_get_worker_socket_buffer(jk_map_t *m, const char *wname, int def)
{
    char buf[1024];
    int i;
    if (!m || !wname) {
        return -1;
    }

    MAKE_WORKER_PARAM(SOCKET_BUFFER_OF_WORKER);

    i = jk_map_get_int(m, buf, 0);
    if (i > 0 && i < def)
        i = def;
    return i;
}

int jk_get_worker_socket_keepalive(jk_map_t *m, const char *wname, int def)
{
    char buf[1024];

    if (!m || !wname) {
        return -1;
    }

    MAKE_WORKER_PARAM(SOCKET_KEEPALIVE_OF_WORKER);

    return jk_map_get_bool(m, buf, def);
}

int jk_get_worker_cache_timeout(jk_map_t *m, const char *wname, int def)
{
    char buf[1024];
    int rv;

    if (!m || !wname) {
        return -1;
    }

    MAKE_WORKER_PARAM(CACHE_TIMEOUT_OF_WORKER);
    if ((rv = jk_map_get_int(m, buf, -1)) >= 0)
        return rv;
    MAKE_WORKER_PARAM(CACHE_TIMEOUT_DEPRECATED);

    return jk_map_get_int(m, buf, def);
}

int jk_get_worker_connect_timeout(jk_map_t *m, const char *wname, int def)
{
    char buf[1024];

    if (!m || !wname) {
        return -1;
    }

    MAKE_WORKER_PARAM(CONNECT_TIMEOUT_OF_WORKER);

    return jk_map_get_int(m, buf, def);
}

int jk_get_worker_prepost_timeout(jk_map_t *m, const char *wname, int def)
{
    char buf[1024];

    if (!m || !wname) {
        return -1;
    }

    MAKE_WORKER_PARAM(PREPOST_TIMEOUT_OF_WORKER);

    return jk_map_get_int(m, buf, def);
}

int jk_get_worker_reply_timeout(jk_map_t *m, const char *wname, int def)
{
    char buf[1024];

    if (!m || !wname) {
        return -1;
    }

    MAKE_WORKER_PARAM(REPLY_TIMEOUT_OF_WORKER);

    return jk_map_get_int(m, buf, def);
}

int jk_get_worker_recycle_timeout(jk_map_t *m, const char *wname, int def)
{
    return def;
}

int jk_get_worker_retries(jk_map_t *m, const char *wname, int def)
{
    char buf[1024];
    int rv;
    if (!m || !wname) {
        return -1;
    }

    MAKE_WORKER_PARAM(RETRIES_OF_WORKER);

    rv = jk_map_get_int(m, buf, def);
    if (rv < 1)
        rv = 1;

    return rv;
}

int jk_get_worker_recovery_opts(jk_map_t *m, const char *wname, int def)
{
    char buf[1024];

    if (!m || !wname) {
        return -1;
    }

    MAKE_WORKER_PARAM(RECOVERY_OPTS_OF_WORKER);

    return jk_map_get_int(m, buf, def);
}

const char *jk_get_worker_secret_key(jk_map_t *m, const char *wname)
{
    char buf[1024];

    if (!m || !wname) {
        return NULL;
    }

    MAKE_WORKER_PARAM(SECRET_KEY_OF_WORKER);
    return jk_map_get_string(m, buf, NULL);
}

int jk_get_worker_list(jk_map_t *m, char ***list, unsigned *num_of_workers)
{
    if (m && list && num_of_workers) {
        char **ar = jk_map_get_string_list(m,
                                        WORKER_LIST_PROPERTY_NAME,
                                        num_of_workers,
                                        DEFAULT_WORKER);
        if (ar) {
            *list = ar;
            return JK_TRUE;
        }
        *list = NULL;
        *num_of_workers = 0;
    }

    return JK_FALSE;
}

int jk_get_is_worker_disabled(jk_map_t *m, const char *wname)
{
    int rc = JK_TRUE;
    char buf[1024];
    if (m && wname) {
        int value;
        MAKE_WORKER_PARAM(IS_WORKER_DISABLED_DEPRECATED);
        value = jk_map_get_bool(m, buf, 0);
        if (!value)
            rc = JK_FALSE;
    }
    return rc;
}

int jk_get_is_worker_stopped(jk_map_t *m, const char *wname)
{
    int rc = JK_TRUE;
    char buf[1024];
    if (m && wname) {
        int value;
        MAKE_WORKER_PARAM(IS_WORKER_STOPPED_DEPRECATED);
        value = jk_map_get_bool(m, buf, 0);
        if (!value)
            rc = JK_FALSE;
    }
    return rc;
}

int jk_get_worker_activation(jk_map_t *m, const char *wname)
{
    char buf[1024];
    const char *v;
    if (!m || !wname) {
        return JK_LB_ACTIVATION_ACTIVE;
    }

    MAKE_WORKER_PARAM(ACTIVATION_OF_WORKER);
    v = jk_map_get_string(m, buf, NULL);
    if (v) {
        return jk_lb_get_activation_code(v);
    }
    else if (jk_get_is_worker_stopped(m, wname))
        return JK_LB_ACTIVATION_STOPPED;
    else if (jk_get_is_worker_disabled(m, wname))
        return JK_LB_ACTIVATION_DISABLED;
    else
        return JK_LB_ACTIVATION_DEF;
}

int jk_get_lb_factor(jk_map_t *m, const char *wname)
{
    char buf[1024];

    if (!m || !wname) {
        return DEFAULT_LB_FACTOR;
    }

    MAKE_WORKER_PARAM(LOAD_FACTOR_OF_WORKER);

    return jk_map_get_int(m, buf, DEFAULT_LB_FACTOR);
}

int jk_get_distance(jk_map_t *m, const char *wname)
{
    char buf[1024];

    if (!m || !wname) {
        return DEFAULT_DISTANCE;
    }

    MAKE_WORKER_PARAM(DISTANCE_OF_WORKER);

    return jk_map_get_int(m, buf, DEFAULT_DISTANCE);
}

int jk_get_is_sticky_session(jk_map_t *m, const char *wname)
{
    int rc = JK_TRUE;
    char buf[1024];
    if (m && wname) {
        int value;
        MAKE_WORKER_PARAM(STICKY_SESSION);
        value = jk_map_get_bool(m, buf, 1);
        if (!value)
            rc = JK_FALSE;
    }
    return rc;
}

int jk_get_is_sticky_session_force(jk_map_t *m, const char *wname)
{
    int rc = JK_FALSE;
    char buf[1024];
    if (m && wname) {
        int value;
        MAKE_WORKER_PARAM(STICKY_SESSION_FORCE);
        value = jk_map_get_bool(m, buf, 0);
        if (value)
            rc = JK_TRUE;
    }
    return rc;
}

int jk_get_lb_method(jk_map_t *m, const char *wname)
{
    char buf[1024];
    const char *v;
    if (!m || !wname) {
        return JK_LB_METHOD_DEF;
    }

    MAKE_WORKER_PARAM(METHOD_OF_WORKER);
    v = jk_map_get_string(m, buf, NULL);
    return jk_lb_get_method_code(v);
}

int jk_get_lb_lock(jk_map_t *m, const char *wname)
{
    char buf[1024];
    const char *v;
    if (!m || !wname) {
        return JK_LB_LOCK_DEF;
    }

    MAKE_WORKER_PARAM(LOCK_OF_WORKER);
    v = jk_map_get_string(m, buf, NULL);
    return jk_lb_get_lock_code(v);
}

int jk_get_max_packet_size(jk_map_t *m, const char *wname)
{
    char buf[1024];
    int sz;

    if (!m || !wname) {
        return DEF_BUFFER_SZ;
    }

    MAKE_WORKER_PARAM(WORKER_MAX_PACKET_SIZE);
    sz = jk_map_get_int(m, buf, DEF_BUFFER_SZ);
    sz = JK_ALIGN(sz, 1024);
    if (sz < DEF_BUFFER_SZ)
        sz = DEF_BUFFER_SZ;
    else if (sz > 64*1024)
        sz = 64*1024;

    return sz;
}

int jk_get_worker_fail_on_status(jk_map_t *m, const char *wname)
{
    char buf[1024];

    if (!m || !wname) {
        return 0;
    }

    MAKE_WORKER_PARAM(STATUS_FAIL_OF_WORKER);
    return jk_map_get_int(m, buf, 0);

}

int jk_get_worker_user_case_insensitive(jk_map_t *m, const char *wname)
{
    int rc = JK_FALSE;
    char buf[1024];
    if (m && wname) {
        int value;
        MAKE_WORKER_PARAM(USER_CASE_OF_WORKER);
        value = jk_map_get_bool(m, buf, 0);
        if (value)
            rc = JK_TRUE;
    }
    return rc;

}

const char *jk_get_worker_style_sheet(jk_map_t *m, const char *wname, const char *def)
{
    char buf[1024];

    if (!m || !wname) {
        return NULL;
    }

    MAKE_WORKER_PARAM(STYLE_SHEET_OF_WORKER);

    return jk_map_get_string(m, buf, def);
}

const char *jk_get_worker_name_space(jk_map_t *m, const char *wname, const char *def)
{
    const char *rc;
    char buf[1024];
    if (!m || !wname) {
        return NULL;
    }
    MAKE_WORKER_PARAM(NAMESPACE_OF_WORKER);
    rc = jk_map_get_string(m, buf, def);
    if (*rc == '-')
        return "";
    else
        return rc;
}

const char *jk_get_worker_xmlns(jk_map_t *m, const char *wname, const char *def)
{
    const char *rc;
    char buf[1024];
    if (!m || !wname) {
        return NULL;
    }
    MAKE_WORKER_PARAM(XML_NAMESPACE_OF_WORKER);
    rc = jk_map_get_string(m, buf, def);
    if (*rc == '-')
        return "";
    else
        return rc;
}

const char *jk_get_worker_xml_doctype(jk_map_t *m, const char *wname, const char *def)
{
    char buf[1024];
    if (!m || !wname) {
        return NULL;
    }
    MAKE_WORKER_PARAM(XML_DOCTYPE_OF_WORKER);
    return jk_map_get_string(m, buf, def);
}

const char *jk_get_worker_prop_prefix(jk_map_t *m, const char *wname, const char *def)
{
    char buf[1024];
    if (!m || !wname) {
        return NULL;
    }
    MAKE_WORKER_PARAM(PROP_PREFIX_OF_WORKER);
    return jk_map_get_string(m, buf, def);
}

int jk_get_is_read_only(jk_map_t *m, const char *wname)
{
    int rc = JK_FALSE;
    char buf[1024];
    if (m && wname) {
        int value;
        MAKE_WORKER_PARAM(READ_ONLY_OF_WORKER);
        value = jk_map_get_bool(m, buf, 0);
        if (value)
            rc = JK_TRUE;
    }
    return rc;
}

int jk_get_worker_user_list(jk_map_t *m,
                            const char *wname,
                            char ***list, unsigned int *num)
{
    char buf[1024];

    if (m && list && num && wname) {
        char **ar = NULL;

        MAKE_WORKER_PARAM(USER_OF_WORKER);
        ar = jk_map_get_string_list(m, buf, num, NULL);
        if (ar) {
            *list = ar;
            return JK_TRUE;
        }
        *list = NULL;
        *num = 0;
    }

    return JK_FALSE;
}

int jk_get_worker_good_rating(jk_map_t *m,
                              const char *wname,
                              char ***list, unsigned int *num)
{
    char buf[1024];

    if (m && list && num && wname) {
        char **ar = NULL;

        MAKE_WORKER_PARAM(GOOD_RATING_OF_WORKER);
        ar = jk_map_get_string_list(m, buf, num, NULL);
        if (ar) {
            *list = ar;
            return JK_TRUE;
        }
        *list = NULL;
        *num = 0;
    }

    return JK_FALSE;
}

int jk_get_worker_bad_rating(jk_map_t *m,
                             const char *wname,
                             char ***list, unsigned int *num)
{
    char buf[1024];

    if (m && list && num && wname) {
        char **ar = NULL;

        MAKE_WORKER_PARAM(BAD_RATING_OF_WORKER);
        ar = jk_map_get_string_list(m, buf, num, NULL);
        if (ar) {
            *list = ar;
            return JK_TRUE;
        }
        *list = NULL;
        *num = 0;
    }

    return JK_FALSE;
}

int jk_get_lb_worker_list(jk_map_t *m,
                          const char *wname,
                          char ***list, unsigned int *num_of_workers)
{
    char buf[1024];

    if (m && list && num_of_workers && wname) {
        char **ar = NULL;

        MAKE_WORKER_PARAM(BALANCE_WORKERS);
        ar = jk_map_get_string_list(m, buf, num_of_workers, NULL);
        if (ar) {
            *list = ar;
            return JK_TRUE;
        }
        /* Try old balanced_workers directive */
        MAKE_WORKER_PARAM(BALANCED_WORKERS_DEPRECATED);
        ar = jk_map_get_string_list(m, buf, num_of_workers, NULL);
        if (ar) {
            *list = ar;
            return JK_TRUE;
        }
        *list = NULL;
        *num_of_workers = 0;
    }

    return JK_FALSE;
}

int jk_get_worker_mount_list(jk_map_t *m,
                             const char *wname,
                             char ***list, unsigned int *num_of_maps)
{
    char buf[1024];

    if (m && list && num_of_maps && wname) {
        char **ar = NULL;

        MAKE_WORKER_PARAM(MOUNT_OF_WORKER);
        ar = jk_map_get_string_list(m, buf, num_of_maps, NULL);
        if (ar) {
            *list = ar;
            return JK_TRUE;
        }
        *list = NULL;
        *num_of_maps = 0;
    }

    return JK_FALSE;
}

int jk_get_worker_maintain_time(jk_map_t *m)
{
    return jk_map_get_int(m, WORKER_MAINTAIN_PROPERTY_NAME,
                          DEFAULT_MAINTAIN_TIME);
}

int jk_get_worker_mx(jk_map_t *m, const char *wname, unsigned *mx)
{
    char buf[1024];

    if (m && mx && wname) {
        int i;
        MAKE_WORKER_PARAM(MX_OF_WORKER);

        i = jk_map_get_int(m, buf, -1);
        if (-1 != i) {
            *mx = (unsigned)i;
            return JK_TRUE;
        }
    }

    return JK_FALSE;
}

int jk_get_worker_ms(jk_map_t *m, const char *wname, unsigned *ms)
{
    char buf[1024];

    if (m && ms && wname) {
        int i;
        MAKE_WORKER_PARAM(MS_OF_WORKER);

        i = jk_map_get_int(m, buf, -1);
        if (-1 != i) {
            *ms = (unsigned)i;
            return JK_TRUE;
        }
    }

    return JK_FALSE;
}

int jk_get_worker_classpath(jk_map_t *m, const char *wname, const char **cp)
{
    char buf[1024];

    if (m && cp && wname) {
        MAKE_WORKER_PARAM(CP_OF_WORKER);

        *cp = jk_map_get_string(m, buf, NULL);
        if (*cp) {
            return JK_TRUE;
        }
    }

    return JK_FALSE;
}

int jk_get_worker_bridge_type(jk_map_t *m, const char *wname, unsigned *bt)
{
    char buf[1024];
    const char *type;

    if (m && bt && wname) {
        MAKE_WORKER_PARAM(BRIDGE_OF_WORKER);

        type = jk_map_get_string(m, buf, NULL);

        if (type) {
            if (!strcasecmp(type, TOMCAT32_BRIDGE_NAME))
                *bt = TC32_BRIDGE_TYPE;
            else if (!strcasecmp(type, TOMCAT33_BRIDGE_NAME))
                *bt = TC33_BRIDGE_TYPE;
            else if (!strcasecmp(type, TOMCAT40_BRIDGE_NAME))
                *bt = TC40_BRIDGE_TYPE;
            else if (!strcasecmp(type, TOMCAT41_BRIDGE_NAME))
                *bt = TC41_BRIDGE_TYPE;
            else if (!strcasecmp(type, TOMCAT50_BRIDGE_NAME))
                *bt = TC50_BRIDGE_TYPE;

            return JK_TRUE;
        }
    }

    return JK_FALSE;
}

int jk_get_worker_jvm_path(jk_map_t *m, const char *wname, const char **vm_path)
{
    char buf[1024];

    if (m && vm_path && wname) {
        MAKE_WORKER_PARAM(JVM_OF_WORKER);

        *vm_path = jk_map_get_string(m, buf, NULL);
        if (*vm_path) {
            return JK_TRUE;
        }
    }

    return JK_FALSE;
}

/* [V] This is unused. currently. */
int jk_get_worker_callback_dll(jk_map_t *m, const char *wname, const char **cb_path)
{
    char buf[1024];

    if (m && cb_path && wname) {
        MAKE_WORKER_PARAM(NATIVE_LIB_OF_WORKER);

        *cb_path = jk_map_get_string(m, buf, NULL);
        if (*cb_path) {
            return JK_TRUE;
        }
    }

    return JK_FALSE;
}

int jk_get_worker_cmd_line(jk_map_t *m, const char *wname, const char **cmd_line)
{
    char buf[1024];

    if (m && cmd_line && wname) {
        MAKE_WORKER_PARAM(CMD_LINE_OF_WORKER);

        *cmd_line = jk_map_get_string(m, buf, NULL);
        if (*cmd_line) {
            return JK_TRUE;
        }
    }

    return JK_FALSE;
}


int jk_file_exists(const char *f)
{
    if (f) {
        struct stat st;
#ifdef AS400
        if ((0 == stat(f, &st)) && (st.st_mode & _S_IFREG)) {
#else
        if ((0 == stat(f, &st)) && (st.st_mode & S_IFREG)) {
#endif
            return JK_TRUE;
        }
    }
    return JK_FALSE;
}

static int jk_is_some_property(const char *prp_name, const char *suffix, const char *sep)
{
    char buf[1024];

    if (prp_name && suffix) {
        size_t prp_name_len;
        size_t suffix_len;
        strcpy(buf, sep);
        strcat(buf, suffix);
        prp_name_len = strlen(prp_name);
        suffix_len = strlen(buf);
        if (prp_name_len >= suffix_len) {
            const char *prp_suffix = prp_name + prp_name_len - suffix_len;
            if (0 == strcmp(buf, prp_suffix)) {
                return JK_TRUE;
            }
        }
    }

    return JK_FALSE;
}

int jk_is_path_property(const char *prp_name)
{
    return jk_is_some_property(prp_name, "path", "_");
}

int jk_is_cmd_line_property(const char *prp_name)
{
    return jk_is_some_property(prp_name, CMD_LINE_OF_WORKER, ".");
}

int jk_is_list_property(const char *prp_name)
{
    const char **props = &list_properties[0];
    while (*props) {
        if (jk_is_some_property(prp_name, *props, "."))
            return JK_TRUE;
        props++;
    }
    return JK_FALSE;
}

int jk_is_unique_property(const char *prp_name)
{
    const char **props = &unique_properties[0];
    while (*props) {
        if (jk_is_some_property(prp_name, *props, "."))
            return JK_TRUE;
        props++;
    }
    return JK_FALSE;
}

int jk_is_deprecated_property(const char *prp_name)
{
    const char **props = &deprecated_properties[0];
    while (*props) {
        if (jk_is_some_property(prp_name, *props, "."))
            return JK_TRUE;
        props++;
    }
    return JK_FALSE;
}
/*
 * Check that property is a valid one (to prevent user typos).
 * Only property starting with worker.
 */
int jk_is_valid_property(const char *prp_name)
{
    const char **props;
    if (memcmp(prp_name, "worker.", 7))
        return JK_TRUE;

    props = &supported_properties[0];
    while (*props) {
        if (jk_is_some_property(prp_name, *props, "."))
            return JK_TRUE;
        props++;
    }
    return JK_FALSE;
}

int jk_get_worker_stdout(jk_map_t *m, const char *wname, const char **stdout_name)
{
    char buf[1024];

    if (m && stdout_name && wname) {
        MAKE_WORKER_PARAM(STDOUT_OF_WORKER);

        *stdout_name = jk_map_get_string(m, buf, NULL);
        if (*stdout_name) {
            return JK_TRUE;
        }
    }

    return JK_FALSE;
}

int jk_get_worker_stderr(jk_map_t *m, const char *wname, const char **stderr_name)
{
    char buf[1024];

    if (m && stderr_name && wname) {
        MAKE_WORKER_PARAM(STDERR_OF_WORKER);

        *stderr_name = jk_map_get_string(m, buf, NULL);
        if (*stderr_name) {
            return JK_TRUE;
        }
    }

    return JK_FALSE;
}

int jk_get_worker_sysprops(jk_map_t *m, const char *wname, const char **sysprops)
{
    char buf[1024];

    if (m && sysprops && wname) {
        MAKE_WORKER_PARAM(SYSPROPS_OF_WORKER);

        *sysprops = jk_map_get_string(m, buf, NULL);
        if (*sysprops) {
            return JK_TRUE;
        }
    }

    return JK_FALSE;
}

int jk_get_worker_libpath(jk_map_t *m, const char *wname, const char **libpath)
{
    char buf[1024];

    if (m && libpath && wname) {
        MAKE_WORKER_PARAM(LIBPATH_OF_WORKER);

        *libpath = jk_map_get_string(m, buf, NULL);
        if (*libpath) {
            return JK_TRUE;
        }
    }

    return JK_FALSE;
}

char **jk_parse_sysprops(jk_pool_t *p, const char *sysprops)
{
    char **rc = NULL;
#if defined(AS400) || defined(_REENTRANT)
    char *lasts;
#endif

    if (p && sysprops) {
        char *prps = jk_pool_strdup(p, sysprops);
        if (prps && strlen(prps)) {
            unsigned num_of_prps;

            for (num_of_prps = 1; *sysprops; sysprops++) {
                if ('*' == *sysprops) {
                    num_of_prps++;
                }
            }

            rc = jk_pool_alloc(p, (num_of_prps + 1) * sizeof(char *));
            if (rc) {
                unsigned i = 0;
#if defined(AS400) || defined(_REENTRANT)
                char *tmp = strtok_r(prps, "*", &lasts);
#else
                char *tmp = strtok(prps, "*");
#endif

                while (tmp && i < num_of_prps) {
                    rc[i] = tmp;
#if defined(AS400) || defined(_REENTRANT)
                    tmp = strtok_r(NULL, "*", &lasts);
#else
                    tmp = strtok(NULL, "*");
#endif
                    i++;
                }
                rc[i] = NULL;
            }
        }
    }

    return rc;
}

void jk_append_libpath(jk_pool_t *p, const char *libpath)
{
    char *env = NULL;
    char *current = getenv(PATH_ENV_VARIABLE);

    if (current) {
        env = jk_pool_alloc(p, strlen(PATH_ENV_VARIABLE) +
                            strlen(current) + strlen(libpath) + 5);
        if (env) {
            sprintf(env, "%s=%s%c%s",
                    PATH_ENV_VARIABLE, libpath, PATH_SEPERATOR, current);
        }
    }
    else {
        env = jk_pool_alloc(p, strlen(PATH_ENV_VARIABLE) +
                            strlen(libpath) + 5);
        if (env) {
            sprintf(env, "%s=%s", PATH_ENV_VARIABLE, libpath);
        }
    }

    if (env) {
        putenv(env);
    }
}

void jk_init_ws_service(jk_ws_service_t *s)
{
    s->ws_private = NULL;
    s->pool = NULL;
    s->method = NULL;
    s->protocol = NULL;
    s->req_uri = NULL;
    s->remote_addr = NULL;
    s->remote_host = NULL;
    s->remote_user = NULL;
    s->auth_type = NULL;
    s->query_string = NULL;
    s->server_name = NULL;
    s->server_port = 80;
    s->server_software = NULL;
    s->content_length = 0;
    s->is_chunked = 0;
    s->no_more_chunks = 0;
    s->content_read = 0;
    s->is_ssl = JK_FALSE;
    s->ssl_cert = NULL;
    s->ssl_cert_len = 0;
    s->ssl_cipher = NULL;
    s->ssl_session = NULL;
    s->headers_names = NULL;
    s->headers_values = NULL;
    s->num_headers = 0;
    s->attributes_names = NULL;
    s->attributes_values = NULL;
    s->num_attributes = 0;
    s->route = NULL;
    s->retries = JK_RETRIES;
    s->add_log_items = NULL;
}

#ifdef _MT_CODE_PTHREAD
int jk_gettid()
{
    pthread_t t = pthread_self();
#ifdef AS400
    /* OS400 use 64 bits ThreadId, get only low 32 bits for now */
    pthread_id_np_t       tid;
    pthread_getunique_np(&t, &tid);
    return ((int)(tid.intId.lo & 0xFFFFFFFF));
#else
    int tid = (int)(t & 0xFFFF);
    return tid;
#endif /* AS400 */
}
#endif
