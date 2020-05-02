#ifndef _ASIF_MGR_H_
#define _ASIF_MGR_H_
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith -- Confidential and Proprietary.
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	asif_mgr
//
//	Description:	
//
//	FileName:	asif_mgr.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       08/18/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|
#include <sys/timeb.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <process.h>
#include "rt_mqueue.hpp"

#define MAX_MS_CONNECTIONS 48
#define MAX_MS_CONNECTIONS_EVER 64

const int MAX_MSG_SIZE = 1024;

// list of mqueues used in asif_mgr, these names are a local nomenclature
// (local to the asif_mgr process)
//
enum
en_asif_mqueues
{
	AMMQ_TO_MSG = 0,
	AMMQ_TO_DEV,
	AMMQ_FROM_DEV,
	AMMQ_TO_SCHED,
	AMMQ_FROM_SCHED,
	AMMQ_TO_STAT,
	AMMQ_FROM_STAT,
	NUM_AMMQ_VALS
};

struct
ms_raw_message
{
	embeddedHeader	msg_header;
	char		msg_data[MAX_MSG_SIZE - sizeof(embeddedHeader)];
};

//-----------------------------------------------------------------------------|
// ms_connection
//
// This is one instance of a socket connection to a MS (master schedule)
// client.  This object corresponds with one MS client, acknowledging and
// passing on it's commands and sending it response messages that have been
// generated.
//
class
ms_connection
{
private:
	int				sock_fd;
public:
	// methods
					ms_connection(int new_socket_fd);
					~ms_connection();
	unsigned long	appl_entity;
	void * 			run_ms_connection(void * p_vargs);
	ssize_t 		get_complete_ms_msg(ms_raw_message *p_buf, size_t size);
	int 			process_complete_ms_msg(ms_raw_message *p_buf, size_t size);
	ssize_t			read_from_ms_connection(char * buf, size_t size);
	ssize_t 		send_msg_to_ms_connection(char * buf, size_t size);
	void 			print_sock_fd();
	void			shutdown_ms_connection();
};


//-----------------------------------------------------------------------------|
// ms_sockets_mgr
//
// This object (and only one is allowed to exist) starts up and listens
// for MS connection requests and creates separate threads to transact
// with them in the form of ms_connection objects.
//
// When a connection is broken, the ms_connection is cleaned up.
// main() will kill this thread (and only instance of this class) when
// it is shutting down for any reason.
//
class
ms_sockets_mgr
{
private:
	int					port_number;
	int					max_ms_connections;
	struct sockaddr_in	ms_sock_sin;
	size_t 				ms_sock_sin_size;


public:
	ms_connection *		ms[MAX_MS_CONNECTIONS_EVER];
	int					num_ms_connections;
	// methods
						ms_sockets_mgr(int ms_port_num, int max_connections);
						~ms_sockets_mgr();
	ms_connection *		get_appl_entity_connection(unsigned long appl_entity);
	void * 				listen_for_ms_connections(void * p_vargs);
	int					remove_ms_connection(ms_connection * p_ms_conn);
	void 				close_all_ms_connections();
};

#endif	// _ASIF_MGR_H_
