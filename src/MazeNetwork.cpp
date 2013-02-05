// Jon Bardin GPL 2012


#include "MemoryLeak.h"


static int reformat_null(void * ctx) {
    //LOGV("reformat_null\n");
    return 1;
}


static int reformat_boolean(void * ctx, int boolean) {
    //LOGV("reformat_boolean %d\n", boolean);
    return 1;
}


static int reformat_number(void *ctx, const char *s, size_t l) {
    MazeNetwork *n = (MazeNetwork *)ctx;

    //LOGV("reformat_number enter %d\n", n->m_State);

    switch(n->m_State) {
      case 4:
        // first arg of request_registration is int player id
        n->m_Arg0 = strtof(s, (char **)NULL);
        break;

      case 5:
        // first arg of update_player is int player id
        n->m_Arg0 = strtof(s, (char **)NULL);
        n->m_State = 6;
        break;

      case 6:
        // second arg is float
        n->m_Arg1 = strtof(s, (char **)NULL);
        n->m_State = 7;
        break;
    
      case 7:
        // thirg arg is float
        n->m_Arg2 = strtof(s, (char **)NULL);
        n->m_State = 8;
        break;

      case 8:
        // thirg arg is float
        n->m_Arg3 = strtof(s, (char **)NULL);
        n->m_State = 9;
        break;

      case 9:
        // thirg arg is float
        n->m_Arg4 = strtof(s, (char **)NULL);
        n->m_State = 10;
        break;

      case 10:
        // thirg arg is float
        n->m_Arg5 = strtof(s, (char **)NULL);
        n->m_State = 11;
        break;
    };

    //LOGV("reformat_number exit arg0 %d %f\n", n->m_State, n->m_Arg0);
    return 1;
}


static int reformat_string(void *ctx, const unsigned char *stringVal, size_t stringLen) {
    MazeNetwork *n = (MazeNetwork *)ctx;
    
    //LOGV("reformat_string enter %d\n", n->m_State);

    if (3 == n->m_State) {
      if (0 == strncmp("request_registration", (const char *)stringVal, stringLen)) {
        n->m_State = 4;
      }
      if (0 == strncmp("update_player", (const char *)stringVal, stringLen)) {
        n->m_State = 5;
      }
    }

    //LOGV("reformat_string exit %d\n", n->m_State);

    return 1;
}


static int reformat_map_key(void *ctx, const unsigned char *stringVal, size_t stringLen) {
    MazeNetwork *n = (MazeNetwork *)ctx;

    if (0 == n->m_State) {
      if (0 == strncmp("stream", (const char *)stringVal, stringLen)) {
        n->m_State = 1;
      }
    }

    //LOGV("reformat_map_key %d\n", n->m_State);

    return 1;
}


static int reformat_start_map(void * ctx) {
    //MazeNetwork *n = (MazeNetwork *)ctx;
    //LOGV("reformat_start_map %d\n", n->m_State);
    return 1;
}


static int reformat_end_map(void * ctx) {
    //LOGV("reformat_end_map\n");
    return 1;
}


static int reformat_start_array(void * ctx) {
    MazeNetwork *n = (MazeNetwork *)ctx;

    //LOGV("reformat_start_array enter %d\n", n->m_State);

    if (1 == n->m_State) {
      n->m_State = 2;
    } else if (2 == n->m_State) {
      n->m_State = 3;
    }

    //LOGV("reformat_start_array exit %d\n", n->m_State);

    return 1;
}


static int reformat_end_array(void * ctx) {
    MazeNetwork *n = (MazeNetwork *)ctx;

    //LOGV("reformat_end_array enter %d\n", n->m_State);

    if (4 == n->m_State) {
      n->m_Delegate->RequestRegistration((int)n->m_Arg0);
      n->m_State = 3;
    }

    if (11 == n->m_State) {
      n->m_Delegate->UpdatePlayerAtIndex((int)n->m_Arg0, n->m_Arg1, n->m_Arg2, n->m_Arg3, n->m_Arg4);
      n->m_State = 3;
    }

    //LOGV("reformat_end_array enter %d\n", n->m_State);

    return 1;
}


static yajl_callbacks callbacks = {
    reformat_null,
    reformat_boolean,
    NULL,
    NULL,
    reformat_number,
    reformat_string,
    reformat_start_map,
    reformat_map_key,
    reformat_end_map,
    reformat_start_array,
    reformat_end_array
};


int MazeNetwork::Tick(bool write, float x, float y, float a, float b) {

  // attempt to establish connection, check existing connection
  int network_connected_error = ConnectNetwork();

  // if the connection state is invalid, return the invalid state
  if (network_connected_error > 0) {
    return network_connected_error;
  }

  // if we are still waiting for a connection, don't bother with anything else
  // but still return an OK status so we don't freak out the upper levels
  if (m_ConnectionState < 2) {
    return 0;
  }

  if (write) {
    // we need to try and send on every tick to make sure the connection
    // is still active, if it fails, restart networking
    char payload[2048];
    int out = snprintf(payload, 2048 - 1, "{\"update_player\":[%f, %f, %f, %f]}\n", x, y, a, b);

    ssize_t sent = send(m_Socket, payload, out, 0); //MSG_DONTWAIT
    if (sent > 0) {
      //LOGV("fd: %d sent: %d %s\n", m_Socket, sent, payload);
    } else {
      LOGV("send failed\n");
      return StopNetwork();
    }
  }

  int bytesAvailableThisTick = -1;
  int bytesReadThisTick = -1;

  //read the socket to see how many bytes are there
  if (ioctl(m_Socket, FIONREAD, &bytesAvailableThisTick)) {
    LOGV("ioctl FIONREAD failed\n");
    return 2;
  }

  if (bytesAvailableThisTick == 0) {
    return 0;
  }

  bytesReadThisTick = recv(m_Socket, (void *)m_InputBuffer, bytesAvailableThisTick, 0);

  if (bytesReadThisTick < 1) {
    perror("recv m_InputBuffer failed\n");
    return 4;
  }

  m_InputBuffer[bytesReadThisTick] = '\0';
  LOGV("read: %s\n", m_InputBuffer);
  yajl_status stat = yajl_parse(hand, m_InputBuffer, bytesReadThisTick * sizeof(unsigned char));
  if (stat == yajl_status_ok) {
    return 0;
  } else {
    unsigned char *str = yajl_get_error(hand, 1, m_InputBuffer, 1023);
    fprintf(stderr, "%s", (const char *) str);
    yajl_free_error(hand, str);
    return 5;
  }
}


int MazeNetwork::ConnectNetwork(void) {

  // if we have a socket created
  if (m_Socket > 0) {
    // and this connection is connected and ready to write to
    if (m_ConnectionState > 1) {
      // return OK status
      return 0;
    }
  }

  // if this is the initial state
  if (m_ConnectionState == 0) {
    // create socket
    m_Socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_Socket < 1) {
      // reset everything if we cannot create a socket, and return not-OK
      return StopNetwork();
    }

    // Get the file access mode and the file status flags, and set to non-blocking
    int x;
    x = fcntl(m_Socket, F_GETFL, 0);
    fcntl(m_Socket, F_SETFL, x | O_NONBLOCK);

    // attempt to connect, best case scenario, it finishes right away with 0 status code
    if (-1 == connect(m_Socket, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr))) {
      // next best case, we are still in the progress of connecting, upgrade the connection state
      if (EINPROGRESS == errno) {
        // upgrade the connection state to waiting-for-write
        m_ConnectionState = 1;
      } else {
        // the connection totally failed early, return not-OK state
        return StopNetwork();
      }
    } else {
      // yay, we connected right away, upgrade connection state to waiting-for-write
      m_ConnectionState = 1;
    }
  }

  // setup arguments to poll for writability on connection thats in-progress to connect
  fd_set wset;
  struct timeval tval;
  int retVal;
  FD_ZERO(&wset);
  FD_SET(m_Socket, &wset);
  tval.tv_sec = 0;
  tval.tv_usec = 0;

  // poll the socket, testing if we can write to it
  #ifdef EMSCRIPTEN
  retVal = select(m_Socket + 1, &wset, NULL, NULL, &tval);
  #else
  retVal = select(m_Socket + 1, NULL, &wset, NULL, &tval);
  #endif

  // if select returns 0, it means we timed out, we should retry on the next tick
  if (0 == retVal) {
    // but if we timeout too many times, just reset the whole network
    if (m_ConnectionSelectsAttempted++ > 1024) {
      LOGV("shit\n");
      return StopNetwork();
    } else {
      LOGV("stall\n");
      // return OK state, but don't increment the connection state
      return 0;
    }
  } else if (retVal > 0) {
    // we might be connected at this point in time, so lets check for an error
    int valopt = 0;
    socklen_t lon;
    lon = sizeof(int); 
    // check for socket errors, this check may occur
    if (getsockopt(m_Socket, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
      // if it does fail, the sockets totally fucked, return not-OK so we can retry on next tick
      return StopNetwork();
    } else {
      // the checking was successful, but do we have an error or not?
      if (valopt > 0) {
        // yes we do have an error, the socket is fucked, retry on next tick
        return StopNetwork();
      } else {
        // awesome, the socket is in the writable set, lets proceed to send magic cookie
        if (FD_ISSET(m_Socket, &wset)) {
          // lets not block when we try to send data, it will get there when it gets there
          int set = 1;
          setsockopt(m_Socket, SOL_SOCKET, MSG_NOSIGNAL, (void *)&set, sizeof(int));

          // setup the magic cookie, which begins the outbound json stream the server
          // uses to maintain client state for each player
          //char magic[] = "{\"stream\":[";
          char magic[] = "{\"-1\":[]}\n";
          ssize_t sent = send(m_Socket, magic, strlen(magic), 0); //MSG_DONTWAIT
          if (sent == 0) {
            // if we cant send any bytes... the socket is fucked, retry on next tick
            return StopNetwork();
          } else {
            // we are fully connected, and the magic cookie is sent, lets
            // increment the connection state so we dont have to do all of this
            // on the next tick
            m_ConnectionState++;
            return 0;
          }
        } else {
          LOGV("wtf this shouldnt happen\n");
          return StopNetwork();
        }
      }
    }
  } else {
    // select failed, the socket is probably fucked, retry on next tick
    return StopNetwork();
  }
}


int MazeNetwork::StopNetwork() {
  shutdown(m_Socket, SHUT_RDWR);
  close(m_Socket);
  m_Socket = -1;
  m_State = 0;
  m_ConnectionState = 0;
  m_ConnectionSelectsAttempted = 0;
  return 1;
}


MazeNetwork::~MazeNetwork() {
  LOGV("MazeNetwork::dealloc\n");
  StopNetwork();
}


MazeNetwork::MazeNetwork(MazeNetworkDelegate *theDelegate, size_t theBpt) {
  bpt = theBpt;
  m_InputBufferSize = bpt + 1;
  m_Delegate = theDelegate;
  m_Socket = -1;
  m_State = 0;
  m_ConnectionState = 0;
  m_ConnectionSelectsAttempted = 0;

  m_MessageIndex = 0;

  m_Arg0 = 0;
  m_Arg1 = 0;
  m_Arg2 = 0;
  m_Arg3 = 0;
  m_Arg4 = 0;
  m_Arg5 = 0;

  int addressResolution = 0;

  // setup the json stream parser
  m_InputBuffer = (unsigned char *) malloc(sizeof(unsigned char) * m_InputBufferSize);
  hand = yajl_alloc(&callbacks, NULL, (void *)this);
  yajl_config(hand, yajl_allow_comments, 1); // allow json comments
  yajl_config(hand, yajl_dont_validate_strings, 1); // dont validate strings

  //struct sockaddr_in stSockAddr;
  memset(&stSockAddr, 0, sizeof(stSockAddr));
  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(7001);

  struct hostent *host = gethostbyname("emscripten.risingcode.com");
  if (NULL == host) {
    LOGV("error: gethostbyname failed\n");
    StopNetwork();
  } else {

    char **addr_list = host->h_addr_list;
    int *addr = (int*)*addr_list;
    char name[INET_ADDRSTRLEN];

    if (!inet_ntop(AF_INET, addr, name, sizeof(name))) {
      LOGV("gethostbyname failed\n");
      StopNetwork();
    }

    addressResolution = inet_pton(AF_INET, name, &stSockAddr.sin_addr);

    if (0 > addressResolution) {
      LOGV("error: first parameter is not a valid address family");
      StopNetwork();
    } else if (0 == addressResolution) {
      LOGV("char string (second parameter does not contain valid ipaddress)");
      StopNetwork();
    } else {
    }
  }
}
