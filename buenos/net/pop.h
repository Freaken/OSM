/*
 * POP (Packet Oriented Protocol) protocol layer
 *
 * Copyright (C) 2003 Juha Aatrokoski, Timo Lilja,
 *   Leena Salmela, Teemu Takanen, Aleksi Virtanen.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: pop.h,v 1.5 2003/05/17 17:33:37 jaatroko Exp $
 *
 */

#ifndef NET_POP_H
#define NET_POP_H

#include "lib/types.h"
#include "net/network.h"
#include "net/socket.h"

/* POP packet header, 8 bytes, immediately after the network frame
 * header (see network.c)
 */
typedef struct {
    uint16_t source_port __attribute__ ((packed));
    uint16_t dest_port   __attribute__ ((packed));
    uint32_t size        __attribute__ ((packed)); /* payload size */
} pop_header_t;


/* POP queue data type */
typedef struct {
    void *frame;            /* the incoming packet */
    sock_t socket;          /* socket this frame belongs to */
    uint32_t timestamp;     /* when this frame was put into the queue */
    network_address_t from; /* address of the sender */
    int busy;               /* is this queue entry in use? */
} pop_queue_t;


void pop_init();
int pop_push_frame(network_address_t fromaddr,
		   network_address_t toaddr,
		   uint32_t protocol_id,
		   void *frame);


#endif /* NET_POP_H */
