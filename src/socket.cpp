

HostAddress::HostAddress(int port)
{
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = 0;
}

HostAddress::HostAddress(const char *ip, int port)
{
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = inet_addr(ip);
}

HostAddress::HostAddress(const sockaddr_in &addr)
{
    m_addr = addr;
}

HostAddress::~HostAddress(void)
{
}


const char *HostAddress::ip(void) const
{
    char* s = inet_ntoa(m_addr.sin_addr);
    strcpy(m_ipBuff, s);
    return m_ipBuff;
}

int HostAddress::port(void) const
{
    return ntohs(m_addr.sin_port);
}


Socket::Socket(socket_t sock)
{
    m_socket = sock;
}

Socket::~Socket(void)
{
}

Socket Socket::CreateSocket(void)
{
    socket_t sock = ::socket(AF_INET,SOCK_STREAM,0);
	return Socket(sock);
}

bool Socket::bind(const HostAddress & addr)
{
    if(::bind(m_socket,(sockaddr_in*)addr._sockaddr,sizeof(sockaddr_in)) != 0)
		return false;
	return true;
}

bool Socket::listen(int backlog)
{
    if (::listen(m_socket, backlog) != 0) {
        return false;
    }
    return true; 
}

int Socket::option(int level, int name, char *val, socketlen_t* vallen)
{
    return ::getsockopt(m_socket, level, name, val, vallen);
}

int Socket::setOption(int level, int name, char *val, socketlen_t vallen)
{
    return ::setsockopt(m_socket, level, name, val, vallen);
}

bool Socket::setNonBlocking(void)
{
    int flags;
    if ((flags = fcntl(m_socket, F_GETFL, NULL)) < 0) {
        return false;
    }
    if (fcntl(m_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        return false;
    }
    return true;
}

bool Socket::setReuseaddr(void)
{
    int reuse;
    socketlen_t len;

    reuse = 1;
    len = sizeof(reuse);

    return (setOption(SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, len) == 0);
}

bool Socket::setNoDelay(void)
{
    int nodelay;
    socketlen_t len;

    nodelay = 1;
    len = sizeof(nodelay);

    return (setOption(IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, len) == 0);
}

bool Socket::setKeepAlive(void)
{
    int val = 1;
    return (setOption(SOL_SOCKET, SO_KEEPALIVE, (char*)&val, sizeof(val)) == 0);
}

bool Socket::setSendBufferSize(int size)
{
    socketlen_t len;
    len = sizeof(size);
    return (setOption(SOL_SOCKET, SO_SNDBUF, (char*)&size, len) == 0);
}

bool Socket::setRecvBufferSize(int size)
{
    socketlen_t len;
    len = sizeof(size);
    return (setOption(SOL_SOCKET, SO_RCVBUF, (char*)&size, len) == 0);
}

bool Socket::setSendTimeout(int msec)
{
    timeval val;
    val.tv_sec = (msec / 1000);
    val.tv_usec = (msec - val.tv_sec * 1000) * 1000;

    return (setOption(SOL_SOCKET, SO_SNDTIMEO,(char*)&val,sizeof(val)) == 0);
}

bool Socket::setRecvTimeout(int msec)
{
    timeval val;
    val.tv_sec = (msec / 1000);
    val.tv_usec = (msec - val.tv_sec * 1000) * 1000;

    return (setOption(SOL_SOCKET, SO_RCVTIMEO,(char*)&val,sizeof(val)) == 0);
}

int Socket::sendBufferSize(void)
{
    int status, size;
    socketlen_t len;

    size = 0;
    len = sizeof(size);

    status = option(SOL_SOCKET, SO_SNDBUF, (char*)&size, &len);
    if (status < 0) {
        return -1;
    }

    return size;
}

int Socket::recvBufferSize(void)
{
    int status, size;
    socketlen_t len;

    size = 0;
    len = sizeof(size);

    status = option(SOL_SOCKET, SO_RCVBUF, (char*)&size, &len);
    if (status < 0) {
        return -1;
    }

    return size;
}

bool Socket::connect(const HostAddress &addr)
{
    if (::connect(m_socket, (sockaddr*)addr._sockaddr(), sizeof(sockaddr_in)) != 0) {
        return false;
    }

    return true;
}

int Socket::nonblocking_send(const char *buff, int size, int flag)
{
    int ret = ::send(m_socket, buff, size, flag);
    if (ret > 0) {
        return ret;
    } else if (ret == -1) {
        switch(errno) {
        case EAGAIN:
            return IOAgain;
        default:
            return IOError;
        }
    } else {
        return IOError;
    }
}

int Socket::nonblocking_recv(char *buff, int size, int flag)
{
    int ret = ::recv(m_socket, buff, size, flag);
    if (ret > 0) {
        return ret;
    } else if (ret == 0) {
        return IOError;
    } else if (ret == -1) {
        switch(errno) {
        case EAGAIN:
            return IOAgain;
        default:
            return IOError;
        }
    } else {
        return IOError;
    }
}

void Socket::close(void)
{
    if (m_socket != -1) {
        TcpSocket::close(m_socket);
        m_socket = -1;
    }
}

bool Socket::isNull(void) const
{
    return (m_socket < 0);
}

void Socket::close(socket_t sock)
{
    ::close(sock);
}



