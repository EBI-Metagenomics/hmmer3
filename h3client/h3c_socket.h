#ifndef H3C_SOCKET_H
#define H3C_SOCKET_H

struct h3c_socket;
struct h3r;

struct h3c_socket *h3c_socket_new(void);
int                h3c_socket_dial(struct h3c_socket *, char const *ip, int port, long timeout);
int                h3c_socket_warmup(struct h3c_socket *);
int                h3c_socket_send(struct h3c_socket *, char const cmd[static 1], char const seq[static 1]);
int                h3c_socket_recv(struct h3c_socket *, struct h3r *);
int                h3c_socket_hangup(struct h3c_socket *);
void               h3c_socket_del(struct h3c_socket const *);

#endif
