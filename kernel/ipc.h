#ifndef IPC_H
#define IPC_H
#include "types.h"
#define IPC_MAX_PIPES    32
#define IPC_MAX_QUEUES   32
#define IPC_MAX_SHMEM    16
#define IPC_PIPE_SIZE    4096
#define IPC_MSG_MAX_SIZE 512
#define IPC_MSG_MAX      64
typedef struct {
    uint8_t  data[IPC_PIPE_SIZE];
    uint32_t head, tail, count;
    bool     active;
    uint32_t reader_pid, writer_pid;
} ipc_pipe_t;
typedef struct {
    uint8_t  data[IPC_MSG_MAX_SIZE];
    uint32_t size;
    uint32_t sender_pid;
    uint32_t type;
} ipc_message_t;
typedef struct {
    ipc_message_t messages[IPC_MSG_MAX];
    uint32_t head, tail, count;
    bool active; uint32_t owner_pid;
} ipc_queue_t;
typedef struct {
    void*    addr;
    uint32_t size;
    uint32_t key;
    bool     active;
    uint32_t owner_pid;
} ipc_shmem_t;
void ipc_init(void);
int  ipc_pipe_create(uint32_t reader_pid, uint32_t writer_pid);
bool ipc_pipe_write(int fd, const uint8_t* data, uint32_t size);
uint32_t ipc_pipe_read(int fd, uint8_t* buf, uint32_t size);
bool ipc_pipe_close(int fd);
int  ipc_queue_create(uint32_t owner_pid);
bool ipc_queue_send(int qid, uint32_t type, const uint8_t* data, uint32_t size);
bool ipc_queue_recv(int qid, ipc_message_t* msg);
bool ipc_queue_destroy(int qid);
int  ipc_shmem_create(uint32_t key, uint32_t size);
void* ipc_shmem_attach(int id);
bool ipc_shmem_detach(int id);
bool ipc_shmem_destroy(int id);
#endif
