/*
    clsync - file tree sync utility based on inotify
    
    Copyright (C) 2013  Dmitry Yu Okunev <dyokunev@ut.mephi.ru> 0x8E30679C
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CLSYNC_SOCKET_H
#define __CLSYNC_SOCKET_H

#ifdef __linux__
#include <linux/limits.h>
#endif

#include <pthread.h>
#include <stdint.h>

#include "clsync.h"
#include "ctx.h"

#define SOCKET_DEFAULT_PROT	0
#define SOCKET_DEFAULT_SUBPROT	SUBPROT0_TEXT

// buffer size
#define SOCKET_BUFSIZ			(1<<12)

#if PIC
#	define SOCKET_PROVIDER_LIBCLSYNC
#else
#	define SOCKET_PROVIDER_CLSYNC
#endif

#ifdef SOCKET_PROVIDER_LIBCLSYNC
#	define SOCKET_MAX SOCKET_MAX_LIBCLSYNC
#endif
#ifdef SOCKET_PROVIDER_CLSYNC
#	define SOCKET_MAX SOCKET_MAX_CLSYNC
#endif

struct socket_sockthreaddata;
struct sockcmd;

typedef int (*clsyncsock_cb_funct_t)(struct socket_sockthreaddata *thread, struct sockcmd *sockcmd_p, void *arg);
struct clsynccbqueue {
	uint64_t		 cmd_num;

	clsyncsock_cb_funct_t	 callback_funct;
	void			*callback_arg;
};
typedef struct clsynccbqueue clsynccbqueue_t;

struct clsyncsock {
	int sock;
	uint16_t prot;
	uint16_t subprot;

	uint64_t cmd_num;

	size_t		 cbqueue_len;
	clsynccbqueue_t  cbqueue[CLSYNCSOCK_WINDOW+1];
	clsynccbqueue_t *cbqueue_cache[4*CLSYNCSOCK_WINDOW+1];	// It's a hacky hash-table of size "CLSYNCSOCK_WINDOW*2"
};
typedef struct clsyncsock clsyncsock_t;

struct clsyncthread {
	clsyncsock_t *clsyncsock_p;
	void *arg;
	void *funct_arg_free;
};
typedef struct clsyncthread clsyncthread_t;

enum subprot0 {
	SUBPROT0_TEXT,
	SUBPROT0_BINARY,
};
typedef enum subprot0 subprot0_t;

enum clsyncsock_state {
	CLSTATE_NONE	= 0,
	CLSTATE_AUTH,
	CLSTATE_MAIN,
	CLSTATE_DYING,
	CLSTATE_DIED,
};
typedef enum clsyncsock_state clsyncsock_state_t;

enum sockcmd_id {
	SOCKCMD_REQUEST_NEGOTIATION	= 000,
	SOCKCMD_REPLY_NEGOTIATION	= 001,
	SOCKCMD_REPLY_ACK		= 150,
	SOCKCMD_REPLY_UNKNOWNCMD	= 160,
	SOCKCMD_REPLY_INVALIDCMDID	= 161,
	SOCKCMD_REPLY_EINVAL		= 162,
	SOCKCMD_REPLY_EEXIST		= 163,
	SOCKCMD_REPLY_EPERM		= 164,
	SOCKCMD_REPLY_ECUSTOM		= 199,
	SOCKCMD_REQUEST_VERSION		= 200,
	SOCKCMD_REQUEST_INFO		= 201,
	SOCKCMD_REQUEST_DUMP		= 202,
	SOCKCMD_REQUEST_LOGIN		= 210,
	SOCKCMD_REQUEST_SET		= 211,
	SOCKCMD_REQUEST_DIE		= 240,
	SOCKCMD_REQUEST_QUIT		= 250,
	SOCKCMD_REPLY_VERSION		= 300,
	SOCKCMD_REPLY_INFO		= 301,
	SOCKCMD_REPLY_DUMP		= 302,
	SOCKCMD_REPLY_LOGIN		= 310,
	SOCKCMD_REPLY_SET		= 311,
	SOCKCMD_REPLY_DIE		= 340,
	SOCKCMD_REPLY_BYE		= 350,
	SOCKCMD_REPLY_UNEXPECTEDEND	= 351,
	SOCKCMD_MAXID
};
typedef enum sockcmd_id sockcmd_id_t;

struct sockcmd_dat_negotiation {
	uint16_t prot;
	uint16_t subprot;
};
typedef struct sockcmd_dat_negotiation sockcmd_dat_negotiation_t;

struct sockcmd_dat_ack {
	uint64_t	 cmd_num;
	uint16_t	 cmd_id;
};
typedef struct sockcmd_dat_ack sockcmd_dat_ack_t;
#define sockcmd_dat_einval       sockcmd_dat_ack
#define sockcmd_dat_einval_t     sockcmd_dat_ack_t
#define sockcmd_dat_unknowncmd   sockcmd_dat_ack
#define sockcmd_dat_unknowncmd_t sockcmd_dat_ack_t

struct sockcmd_dat_invalidcmd {
	uint64_t	 cmd_num;
};
typedef struct sockcmd_dat_invalidcmd sockcmd_dat_invalidcmd_t;

struct sockcmd_dat_version {
	int		major;
	int		minor;
	char		revision[1<<8];
};
typedef struct sockcmd_dat_version sockcmd_dat_version_t;

struct sockcmd_dat_info {
	char		config_block[1<<8];
	char		label[1<<8];
	char		flags[OPTION_FLAGS];
	char		flags_set[OPTION_FLAGS];
};
typedef struct sockcmd_dat_info sockcmd_dat_info_t;

struct sockcmd_dat_dump {
	char		dir_path[PATH_MAX];
};
typedef struct sockcmd_dat_dump sockcmd_dat_dump_t;

struct sockcmd_dat_eexist {
	char		file_path[PATH_MAX];
};
typedef struct sockcmd_dat_eexist sockcmd_dat_eexist_t;

struct sockcmd_dat_eperm {
	char		descr[BUFSIZ];
};
typedef struct sockcmd_dat_eperm sockcmd_dat_eperm_t;

struct sockcmd_dat_set {
	char		key[BUFSIZ];
	char		value[BUFSIZ];
};
typedef struct sockcmd_dat_set sockcmd_dat_set_t;

struct sockcmd {
	uint64_t	 cmd_num;
	uint16_t	 cmd_id;
	size_t		 data_len;
	void		*data;
};
typedef struct sockcmd sockcmd_t;

enum sockprocflags {
	SOCKPROCFLAG_NONE		= 0x00,
};
typedef enum sockprocflags sockprocflags_t;

enum sockauth_id {
	SOCKAUTH_UNSET	= 0,
	SOCKAUTH_NULL,
	SOCKAUTH_PAM,
};
typedef enum sockauth_id sockauth_id_t;

struct socket_sockthreaddata;
typedef int (*clsyncsock_procfunct_t)(struct socket_sockthreaddata *, sockcmd_t *);
typedef void (*freefunct_t)(void *);
struct socket_sockthreaddata {
	int			 id;
	clsyncsock_procfunct_t	 procfunct;
	freefunct_t		 freefunct_arg;
	clsyncsock_t		*clsyncsock_p;
	void			*arg;
	clsyncsock_state_t	 state;
	sockauth_id_t		 authtype;
	int			*running;		// Pointer to interger with non-zero value to continue running
	sockprocflags_t		 flags;
	pthread_t		 thread;
};
typedef struct socket_sockthreaddata socket_sockthreaddata_t;

extern int socket_reply(clsyncsock_t *clsyncsock_p, sockcmd_t *sockcmd_p, sockcmd_id_t cmd_id, ...);
extern int socket_send(clsyncsock_t *clsyncsock, sockcmd_id_t cmd_id, ...);
extern int socket_send_cb(clsyncsock_t *clsyncsock_p, sockcmd_id_t cmd_id, clsyncsock_cb_funct_t cb, void *cb_arg, ...);
extern int socket_sendinvalid(clsyncsock_t *clsyncsock_p, sockcmd_t *sockcmd_p);
extern int socket_recv(clsyncsock_t *clsyncsock, sockcmd_t *sockcmd);
extern int socket_check_bysock(int sock);
extern clsyncsock_t *socket_accept(int sock);
extern int socket_cleanup(clsyncsock_t *clsyncsock_p);
extern int socket_close(clsyncsock_t *clsyncsock_p);
extern int socket_init();
extern int socket_deinit();
extern int socket_procclsyncsock(socket_sockthreaddata_t *arg);
extern clsyncsock_t *socket_connect_unix(const char *const socket_path);
extern clsyncsock_t *socket_listen_unix (const char *const socket_path);

extern socket_sockthreaddata_t *socket_thread_attach(clsyncsock_t *clsyncsock_p);
extern int socket_thread_start(socket_sockthreaddata_t *threaddata_p);

extern int clsyncsocks_num;
extern int clsyncsocks_count;
extern int clsyncsocks_last;

extern const char *const textmessage_args[];
extern const char *const textmessage_descr[];

#endif

