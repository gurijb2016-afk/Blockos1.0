#include "ipc.h"
#include "generic.h"

static ipc_pipe_t  g_pipes[IPC_MAX_PIPES];
static ipc_queue_t g_queues[IPC_MAX_QUEUES];
static ipc_shmem_t g_shmem[IPC_MAX_SHMEM];

void ipc_init(void) {
    memset(g_pipes,  0, sizeof(g_pipes));
    memset(g_queues, 0, sizeof(g_queues));
    memset(g_shmem,  0, sizeof(g_shmem));
}

/* PIPE */
int ipc_pipe_create(uint32_t rpid, uint32_t wpid) {
    for (int i = 0; i < IPC_MAX_PIPES; i++) {
        if (!g_pipes[i].active) {
            g_pipes[i].active = true;
            g_pipes[i].head = g_pipes[i].tail = g_pipes[i].count = 0;
            g_pipes[i].reader_pid = rpid;
            g_pipes[i].writer_pid = wpid;
            return i;
        }
    }
    return -1;
}

bool ipc_pipe_write(int fd, const uint8_t* data, uint32_t size) {
    if (fd < 0 || fd >= IPC_MAX_PIPES || !g_pipes[fd].active) return false;
    ipc_pipe_t* p = &g_pipes[fd];
    for (uint32_t i = 0; i < size; i++) {
        if (p->count >= IPC_PIPE_SIZE) return false;
        p->data[p->tail % IPC_PIPE_SIZE] = data[i];
        p->tail++; p->count++;
    }
    return true;
}

uint32_t ipc_pipe_read(int fd, uint8_t* buf, uint32_t size) {
    if (fd < 0 || fd >= IPC_MAX_PIPES || !g_pipes[fd].active) return 0;
    ipc_pipe_t* p = &g_pipes[fd];
    uint32_t n = 0;
    while (n < size && p->count > 0) {
        buf[n++] = p->data[p->head % IPC_PIPE_SIZE];
        p->head++; p->count--;
    }
    return n;
}

bool ipc_pipe_close(int fd) {
    if (fd < 0 || fd >= IPC_MAX_PIPES) return false;
    g_pipes[fd].active = false;
    return true;
}

/* MESSAGE QUEUE */
int ipc_queue_create(uint32_t owner) {
    for (int i = 0; i < IPC_MAX_QUEUES; i++) {
        if (!g_queues[i].active) {
            g_queues[i].active = true;
            g_queues[i].owner_pid = owner;
            g_queues[i].head = g_queues[i].tail = g_queues[i].count = 0;
            return i;
        }
    }
    return -1;
}

bool ipc_queue_send(int qid, uint32_t type, const uint8_t* data, uint32_t size) {
    if (qid < 0 || qid >= IPC_MAX_QUEUES || !g_queues[qid].active) return false;
    ipc_queue_t* q = &g_queues[qid];
    if (q->count >= IPC_MSG_MAX) return false;
    ipc_message_t* msg = &q->messages[q->tail % IPC_MSG_MAX];
    msg->type = type;
    msg->size = size < IPC_MSG_MAX_SIZE ? size : IPC_MSG_MAX_SIZE;
    memcpy(msg->data, data, msg->size);
    q->tail++; q->count++;
    return true;
}

bool ipc_queue_recv(int qid, ipc_message_t* msg) {
    if (qid < 0 || qid >= IPC_MAX_QUEUES || !g_queues[qid].active) return false;
    ipc_queue_t* q = &g_queues[qid];
    if (q->count == 0) return false;
    *msg = q->messages[q->head % IPC_MSG_MAX];
    q->head++; q->count--;
    return true;
}

bool ipc_queue_destroy(int qid) {
    if (qid < 0 || qid >= IPC_MAX_QUEUES) return false;
    g_queues[qid].active = false;
    return true;
}

/* SHARED MEMORY */
int ipc_shmem_create(uint32_t key, uint32_t size) {
    for (int i = 0; i < IPC_MAX_SHMEM; i++) {
        if (!g_shmem[i].active) {
            g_shmem[i].addr = malloc(size);
            if (!g_shmem[i].addr) return -1;
            g_shmem[i].size = size;
            g_shmem[i].key  = key;
            g_shmem[i].active = true;
            return i;
        }
    }
    return -1;
}

void* ipc_shmem_attach(int id) {
    if (id < 0 || id >= IPC_MAX_SHMEM || !g_shmem[id].active) return NULL;
    return g_shmem[id].addr;
}

bool ipc_shmem_detach(int id) { return id >= 0 && id < IPC_MAX_SHMEM; }

bool ipc_shmem_destroy(int id) {
    if (id < 0 || id >= IPC_MAX_SHMEM || !g_shmem[id].active) return false;
    free(g_shmem[id].addr);
    g_shmem[id].active = false;
    return true;
}
