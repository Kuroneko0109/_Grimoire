#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <libgrimoire/grid/peer.h>
#include <libgrimoire/system/memory.h>

typedef struct priv_peer priv_peer_t;

struct priv_peer {
	peer_t public;

	int fd;

	struct sockaddr_un addr_un;
};

void peer_set_fd_uds(peer_t * this, int fd)
{
	priv_peer_t * priv = (priv_peer_t *)this;
	
	if(-1 != priv->fd)
		this->close(this);

	priv->fd = fd;
}

void peer_set_blk_uds(peer_t * this)
{
	priv_peer_t * priv = (priv_peer_t *)this;
	int flags;

	if(-1 != priv->fd)
	{
		flags = fcntl(priv->fd, F_GETFL, 0);
		fcntl(priv->fd, F_SETFL, flags | O_NONBLOCK);
	}
}

void peer_set_nblk_uds(peer_t * this)
{
	priv_peer_t * priv = (priv_peer_t *)this;
	int flags;

	if(-1 != priv->fd)
	{
		flags = fcntl(priv->fd, F_GETFL, 0);
		fcntl(priv->fd, F_SETFL, flags & ~O_NONBLOCK);
	}
}

ssize_t peer_read_uds(peer_t * this, void * dst, size_t size)
{
	priv_peer_t * priv = (priv_peer_t *)this;
	return read(priv->fd, dst, size);
}

ssize_t peer_write_uds(peer_t * this, void * src, size_t size)
{
	priv_peer_t * priv = (priv_peer_t *)this;
	if(-1 == priv->fd)
		return -1;

	return write(priv->fd, src, size);
}

void peer_destroy_uds(peer_t * this)
{
	priv_peer_t * priv = (priv_peer_t *)this;

	if(-1 != priv->fd)
		close(priv->fd);
	
	free(this);
}

int peer_open_uds(peer_t * this)
{
	priv_peer_t * priv = (priv_peer_t *)this;
	int res;

	/* already opened */
	if(-1 != priv->fd)
		return -1;

	priv->fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(-1 == priv->fd)
		return -1;

	res = connect(priv->fd, (struct sockaddr *)&priv->addr_un, sizeof(struct sockaddr_un));

	return res;
}

void peer_close_uds(peer_t * this)
{
	priv_peer_t * priv = (priv_peer_t *)this;

	if(-1 != priv->fd)
		close(priv->fd);
	priv->fd = -1;
}

void peer_set_addr_uds(peer_t * this, const char * addr_str)
{
	priv_peer_t * priv = (priv_peer_t *)this;

	memset(&priv->addr_un, 0, sizeof(struct sockaddr_un));
	priv->addr_un.sun_family = AF_UNIX;
	strcpy(priv->addr_un.sun_path, addr_str);
}

void peer_set_port_uds(peer_t * this, uint16_t port)
{
	priv_peer_t * priv = (priv_peer_t *)this;
	printf("%s(%d) %p %d\n", __func__, __LINE__, priv, port);
}

void peer_dump_uds(peer_t * this)
{
	priv_peer_t * priv = (priv_peer_t *)this;
	printf("%s(%d) %p\n", __func__, __LINE__, priv);
}

peer_t * create_peer_uds(void)
{
	priv_peer_t * private;
	peer_t * public;

	private = galloc(sizeof(priv_peer_t));
	public = &private->public;

	private->fd = -1;
	memset(&private->addr_un, 0, sizeof(struct sockaddr_un));

	public->set_addr = peer_set_addr_uds;
	public->set_fd = peer_set_fd_uds;
	public->read = peer_read_uds;
	public->write = peer_write_uds;
	public->set_blk = peer_set_blk_uds;
	public->set_nblk = peer_set_nblk_uds;
	public->set_port = peer_set_port_uds;
	public->dump = peer_dump_uds;
	public->destroy = peer_destroy_uds;
	public->open = peer_open_uds;
	public->close = peer_close_uds;

	return public;
}
