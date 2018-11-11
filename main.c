#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include "luajit-2.0/dynasm/dasm_proto.h"
#include "luajit-2.0/dynasm/dasm_arm.h"
#include "bytecode.h"


#define SERVER_PORT 8000
#define CONNECT_TIMEOUT { .tv_sec = 0, .tv_usec = 100000 }
#define READ_TIMEOUT    { .tv_sec = 1, .tv_usec =      0 }
#define WRITE_TIMEOUT   { .tv_sec = 0, .tv_usec =  20000 }



|.arch arm
|.define CFUNCADDR, r10
|.define CARG1, r0
|.define CARG2, r1
|.define CARG3, r2
|.define CARG4, r3
|.define CRET1, r0
|.define CRET2, r1


static void* link_and_encode(dasm_State** d) {
	size_t sz;
	void* buf;
	dasm_link(d, &sz);
	buf = mmap(0, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	dasm_encode(d, buf);
	mprotect(buf, sz, PROT_READ | PROT_EXEC);
	return buf;
}

typedef struct run_state run_state_t;

typedef void (*func_ptr)(run_state_t*);
static func_ptr func_cache[sizeof(bytecode)/sizeof(bytecode[0])] = {};

// must be a power of 2
#define STACK_ITEMS 16

struct run_state {
	void *here;

	uint32_t stackp;
	// circular stack
	uint32_t stack[STACK_ITEMS];

	func_ptr *func_cache;

	int server_fd;

	void (*print_tos)(run_state_t*);
	void (*bye)(run_state_t*);
	void (*server_accept)(run_state_t*);
	void (*client_connect)(run_state_t*);
	void (*fd_read)(run_state_t*);
	void (*fd_write)(run_state_t*);
	void (*fd_close)(run_state_t*);
	func_ptr (*get_func)(run_state_t*, int);
};

static func_ptr compile(int func_index) {
	dasm_State* d;
	unsigned int npc = 8;
	unsigned int nextpc = 0;
	struct {
		int valid;
		unsigned int nextpc;
	} pending_labels[2] = {{}};

	|.section code
	dasm_init(&d, DASM_MAXSECTION);
	|.globals lbl_
	void* labels[lbl__MAX];
	dasm_setupglobal(&d, labels, lbl__MAX);
	|.actionlist actions
	dasm_setup(&d, actions);
	dasm_growpc(&d, npc);

	|.define aState, r9
	|.type state, run_state_t, aState

	dasm_State** Dst = &d;
	|.code
	|->ep:
	| str lr, [sp]
	| sub sp, sp, #4
	| mov aState, CARG1
	|1:

	for (uint16_t *program = bytecode[func_index]; *program != OP_END; ++program) {
		if (pending_labels[1].valid) {
			|=>pending_labels[1].nextpc:
		}
		pending_labels[1] = pending_labels[0];
		pending_labels[0].valid = 0;

		switch (*program) {
			case OP_IF:
				if (nextpc == npc) {
					npc *= 2;
					dasm_growpc(&d, npc);
				}
				| ldr r3, state->stackp
				| mov r2, aState
				| add r2, r2, #offsetof(run_state_t, stack)
				| add r2, r2, r3
				| ldr r2, [r2]
				| add r3, r3, #STACK_ITEMS*4-4
				| and r3, r3, #STACK_ITEMS*4-1
				| cmp r2, #0
				| str r3, state->stackp
				| beq =>nextpc
				pending_labels[0].valid = 1;
				pending_labels[0].nextpc = nextpc++;
				break;
			case OP_NUMBER:
				++program;
				| movw r1, #*program
				| ldr r3, state->stackp
				| add r3, r3, #4
				| and r3, r3, #STACK_ITEMS*4-1
				| str r3, state->stackp
				| mov r2, aState
				| add r2, r2, #offsetof(run_state_t, stack)
				| add r2, r2, r3
				| str r1, [r2]
				break;
			case OP_HERE:
				| ldr r3, state->stackp
				| add r3, r3, #4
				| and r3, r3, #STACK_ITEMS*4-1
				| str r3, state->stackp
				| mov r2, aState
				| add r2, r2, #offsetof(run_state_t, stack)
				| ldr r1, state->here
				| add r2, r2, r3
				| str r1, [r2]
				break;
			case OP_DUP:
				| ldr r3, state->stackp
				| mov r8, aState
				| add r8, r8, #offsetof(run_state_t, stack)
				| add r2, r8, r3
				| ldr r1, [r2]
				| add r3, r3, #4
				| and r3, r3, #STACK_ITEMS*4-1
				| str r3, state->stackp
				| add r2, r8, r3
				| str r1, [r2]
				break;
			case OP_OVER:
				| ldr r3, state->stackp
				| mov r8, aState
				| add r8, r8, #offsetof(run_state_t, stack)
				| add r3, r3, #STACK_ITEMS*4-4
				| and r3, r3, #STACK_ITEMS*4-1
				| add r2, r8, r3
				| ldr r1, [r2]
				| add r3, r3, #8
				| and r3, r3, #STACK_ITEMS*4-1
				| str r3, state->stackp
				| add r2, r8, r3
				| str r1, [r2]
				break;
			case OP_SWAP:
				| ldr r3, state->stackp
				| mov r8, aState
				| add r8, r8, #offsetof(run_state_t, stack)
				| add r3, r3, #STACK_ITEMS*4-4
				| and r3, r3, #STACK_ITEMS*4-1
				| add r2, r8, r3
				| ldr r1, [r2]
				| add r3, r3, #4
				| and r3, r3, #STACK_ITEMS*4-1
				| add r8, r8, r3
				| ldr r3, [r8]
				| str r3, [r2]
				| str r1, [r8]
				break;
			case OP_DROP:
				| ldr r3, state->stackp
				| add r3, r3, #STACK_ITEMS*4-4
				| and r3, r3, #STACK_ITEMS*4-1
				| str r3, state->stackp
				break;
			case OP_ADD:
			case OP_SUB:
			case OP_XOR:
			case OP_AND:
			case OP_SHL:
			case OP_MUL:
			case OP_EQUAL:
			case OP_LESS:
				| ldr r3, state->stackp
				| mov r8, aState
				| add r8, r8, #offsetof(run_state_t, stack)
				| add r3, r3, #STACK_ITEMS*4-4
				| and r3, r3, #STACK_ITEMS*4-1
				| str r3, state->stackp
				| add r2, r8, r3
				| ldr r1, [r2]
				| add r3, r3, #4
				| and r3, r3, #STACK_ITEMS*4-1
				| add r8, r8, r3
				| ldr r3, [r8]
				switch (*program) {
					case OP_ADD:
						| add r3, r1, r3
						break;
					case OP_SUB:
						| sub r3, r1, r3
						break;
					case OP_XOR:
						| eor r3, r1, r3
						break;
					case OP_AND:
						| and r3, r1, r3
						break;
					case OP_SHL:
						| lsr r3, r1, r3
						break;
					case OP_MUL:
						| mul r3, r3, r1
						break;
					case OP_EQUAL:
						| cmp r3, r1
						| moveq r3, #1
						| movne r3, #0
						break;
					case OP_LESS:
						| cmp r1, r3
						| movlo r3, #1
						| movhs r3, #0
						break;
				}
				| str r3, [r2]
				break;
			case OP_LOAD:
			case OP_CLOAD:
				| ldr r1, state->stackp
				| mov r2, aState
				| add r2, r2, #offsetof(run_state_t, stack)
				| add r2, r2, r1
				| ldr r1, [r2]
				if (*program == OP_LOAD) {
					| ldr r1, [r1]
				} else {
					| ldrb r1, [r1]
				}
				| str r1, [r2]
				break;
			case OP_STORE:
			case OP_CSTORE:
				| ldr r1, state->stackp
				| mov r8, aState
				| add r8, r8, #offsetof(run_state_t, stack)
				| add r2, r8, r1
				| ldr r3, [r2]
				| add r1, r1, #STACK_ITEMS*4-4
				| and r1, r1, #STACK_ITEMS*4-1
				| add r2, r8, r1
				| ldr r2, [r2]
				| add r1, r1, #STACK_ITEMS*4-4
				| and r1, r1, #STACK_ITEMS*4-1
				| str r1, state->stackp
				if (*program == OP_STORE) {
					| str r2, [r3]
				} else {
					| strb r2, [r3]
				}
				break;
			case OP_BYE:
			case OP_PRINT_TOS:
			case OP_READ:
			case OP_WRITE:
			case OP_CLOSE:
			case OP_ACCEPT:
			case OP_CONNECT:
				switch (*program) {
					case OP_BYE:
						| ldr CFUNCADDR, state->bye
						break;
					case OP_PRINT_TOS:
						| ldr CFUNCADDR, state->print_tos
						break;
					case OP_READ:
						| ldr CFUNCADDR, state->fd_read
						break;
					case OP_WRITE:
						| ldr CFUNCADDR, state->fd_write
						break;
					case OP_CLOSE:
						| ldr CFUNCADDR, state->fd_close
						break;
					case OP_ACCEPT:
						| ldr CFUNCADDR, state->server_accept
						break;
					case OP_CONNECT:
						| ldr CFUNCADDR, state->client_connect
						break;
				}
				| mov CARG1, aState
				| blx CFUNCADDR
				break;
			case OP_RETURN:
				| b <2
				| nop
				break;
			case OP_TAIL_CALL:
				| b <1
				| nop
				break;
			case OP_CALL:
				++program;
				| mov CARG1, aState
				| ldr CFUNCADDR, state->get_func
				| movw CARG2, #*program
				| blx CFUNCADDR
				| mov r3, CRET1
				| mov CARG1, aState
				| blx r3
				break;
		}
	}

	|2:
	| add sp, sp, #4
	| ldr lr, [sp]
	| bx lr
	| nop

	link_and_encode(&d);
	dasm_free(&d);

	return (void(*)(run_state_t*))labels[lbl_ep];
}


static uint32_t peek(run_state_t *state) {
	return state->stack[state->stackp / 4];
}

static uint32_t pop(run_state_t *state) {
	uint32_t value = peek(state);
	state->stackp = (state->stackp + STACK_ITEMS*4-4) & (STACK_ITEMS*4-1);
	return value;
}

static void push(run_state_t *state, uint32_t value) {
	state->stackp = (state->stackp + 4) & (STACK_ITEMS*4-1);
	state->stack[state->stackp / 4] = value;
}


static void print_tos(run_state_t *state) {
	printf("%u ", peek(state));
	fflush(stdout);
}


static void bye(run_state_t *state) {
	printf("Use our secret wisely! Until the next time...");
	exit(0);
}


static func_ptr get_func(run_state_t *state, int func_index) {
	if (state->func_cache[func_index] != NULL)
		return state->func_cache[func_index];
	return (state->func_cache[func_index] = compile(func_index));
}


static void server_init(run_state_t *state) {
	signal(SIGPIPE, SIG_IGN);
	state->server_fd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(state->server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERVER_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(state->server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		goto fail;
	if (listen(state->server_fd, 5) < 0)
		goto fail;
	return;
fail:;
	perror("聞難");
	exit(1);
}


static void server_accept(run_state_t *state) {
	while (1) {
		int fd = accept4(state->server_fd, NULL, NULL, SOCK_NONBLOCK);
		if (fd < 0)
			goto fail;

		struct sockaddr_in addr;
		socklen_t addr_len = sizeof(addr);
		if (getsockname(fd, (struct sockaddr *)&addr, &addr_len) < 0)
			goto fail;
		if (addr_len != sizeof(addr) || addr.sin_family != AF_INET)
			goto fail;

		push(state, ntohl(addr.sin_addr.s_addr));
		push(state, fd);
		return;
fail:;
		perror("受難");
		if (fd >= 0)
			close(fd);
	}
}


static void client_connect(run_state_t *state) {
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(pop(state));
	addr.sin_addr.s_addr = htonl(pop(state));

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		goto fail;
	fcntl(fd, F_SETFL, O_NONBLOCK);
	connect(fd, (struct sockaddr *)&addr, sizeof(addr));

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	struct timeval tv = CONNECT_TIMEOUT;

	if (select(fd + 1, NULL, &fds, NULL, &tv) != 1)
		goto fail;

	int err;
	socklen_t err_len = sizeof(err);
	getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &err_len);

	if (err_len != sizeof(err) || err != 0)
		goto fail;

	push(state, fd);
	return;
fail:;
	if (fd >= 0)
		close(fd);
	push(state, -1);
}


static void fd_read(run_state_t *state) {
	int fd = pop(state);
	uint8_t c;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	struct timeval tv = READ_TIMEOUT;

	if (select(fd + 1, &fds, NULL, NULL, &tv) == 1 && read(fd, &c, 1) == 1) {
		push(state, c);
	} else {
		push(state, -1);
	}
}


static void fd_write(run_state_t *state) {
	int fd = pop(state);
	uint8_t c = pop(state);

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	struct timeval tv = WRITE_TIMEOUT;
	if (select(fd + 1, NULL, &fds, NULL, &tv) == 1) {
		write(fd, &c, 1);
	}
}


static void fd_close(run_state_t *state) {
	int fd = pop(state);
	close(fd);
}


static void start_vm(int entry_func) {
	run_state_t state;
	state.stackp = 0;
	state.here = here_data;
	state.func_cache = func_cache;
	state.print_tos = print_tos;
	state.bye = bye;
	state.server_accept = server_accept;
	state.client_connect = client_connect;
	state.fd_read = fd_read;
	state.fd_write = fd_write;
	state.fd_close = fd_close;
	state.get_func = get_func;
	server_init(&state);
	state.get_func(&state, entry_func)(&state);
}


int main() {
	start_vm(0);
	return 0;
}
