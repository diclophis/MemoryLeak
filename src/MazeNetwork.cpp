// Jon Bardin GPL 2012


#include "MemoryLeak.h"


static int reformat_null(void * ctx) {
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_null(g);
    LOGV("reformat_null\n");
    return 1;
}


static int reformat_boolean(void * ctx, int boolean) {
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_bool(g, boolean);
    LOGV("reformat_boolean %d\n", boolean);
    return 1;
}


static int reformat_number(void * ctx, const char * s, size_t l) {
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_number(g, s, l);
    //LOGV("reformat_number %f\n", strtof(s, (char **)s+l));
    //LOGV("reformat_number\n");
    return 1;
}


static int reformat_string(void * ctx, const unsigned char * stringVal, size_t stringLen) {
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_string(g, stringVal, stringLen);
    //LOGV("reformat_string %s %d\n", stringVal, stringLen);
    LOGV("reformat_string\n");
    return 1;
}


static int reformat_map_key(void * ctx, const unsigned char * stringVal, size_t stringLen) {
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_string(g, stringVal, stringLen);
    //LOGV("reformat_map_key %s\n", stringVal);
    LOGV("reformat_map_key\n");
    return 1;
}


static int reformat_start_map(void * ctx) {
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_map_open(g);
    LOGV("reformat_start_map\n");
    return 1;
}


static int reformat_end_map(void * ctx) {
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_map_close(g);
    LOGV("reformat_end_map\n");
    return 1;
}


static int reformat_start_array(void * ctx) {
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_array_open(g);
    LOGV("reformat_start_array\n");
    return 1;
}


static int reformat_end_array(void * ctx) {
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_array_close(g);
    LOGV("reformat_end_array\n");
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


int MazeNetwork::Tick() {

  int network_connected_error = ConnectNetwork();

  if (network_connected_error > 0) {
    LOGV("not connected, will try again next tick\n");
    return network_connected_error;
  }

  char payload[4] = "[1]";
  ssize_t sent = send(m_Socket, payload, 3, 0); //MSG_DONTWAIT
  if (sent > 0) {
    //LOGV("wtf11111 %d payload-sent: %d\n", SocketFD, sent);
  } else {
    LOGV("send failed\n");
    StopNetwork();
    return 1;
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

  bytesReadThisTick = recv(m_Socket, (void *)m_InputBuffer, bpt, 0);

  if (bytesReadThisTick < 1) {
    perror("recv m_InputBuffer failed\n");
    return 4;
  }

  m_InputBuffer[bytesReadThisTick] = '\0';
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

  if (m_Socket > 0) {
    return 0;
  }

  int addressResolution = 0;

  struct sockaddr_in stSockAddr;
  memset(&stSockAddr, 0, sizeof(stSockAddr));
  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(7001);

  struct hostent *host = gethostbyname("emscripten.risingcode.com");
  char **addr_list = host->h_addr_list;
  int *addr = (int*)*addr_list;
  char name[INET_ADDRSTRLEN];

  if (!inet_ntop(AF_INET, addr, name, sizeof(name))) {
    LOGV("gethostbyname failed\n");
    StopNetwork();
    return 1;
  }

  addressResolution = inet_pton(AF_INET, name, &stSockAddr.sin_addr);

  if (0 > addressResolution) {
    LOGV("error: first parameter is not a valid address family");
    StopNetwork();
    return 2;
  } else if (0 == addressResolution) {
    LOGV("char string (second parameter does not contain valid ipaddress)");
    StopNetwork();
    return 3;
  }

  m_Socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (m_Socket < 1) {
    LOGV("cannot create socket");
    StopNetwork();
    return 4;
  }

  if (-1 == connect(m_Socket, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr))) {
    LOGV("connect failed\n");
    StopNetwork();
    return 5;
  }

  int set = 1;
  setsockopt(m_Socket, SOL_SOCKET, MSG_NOSIGNAL, (void *)&set, sizeof(int));

  char magic[1];
  magic[0] = '{';
  ssize_t sent = send(m_Socket, magic, 1, 0); //MSG_DONTWAIT
  if (sent == 0) {
    StopNetwork();
    return 6;
  }

  m_InputBuffer = (unsigned char *) malloc(sizeof(unsigned char) * m_InputBufferSize);
  hand = yajl_alloc(&callbacks, NULL, (void *)m_Delegate);
  yajl_config(hand, yajl_allow_comments, 1); // allow json comments
  yajl_config(hand, yajl_dont_validate_strings, 1); // dont validate strings

  return 0;
}


void MazeNetwork::StopNetwork() {
  shutdown(m_Socket, SHUT_RDWR);
  close(m_Socket);
  m_Socket = -1;
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
}
