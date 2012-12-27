// Jon Bardin GPL 2012


#include "MemoryLeak.h"


#include <stdlib.h>
#define EXPECTED_BYTES 5


static int reformat_null(void * ctx)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_null(g);
    LOGV("reformat_null\n");
    return 1;
}

static int reformat_boolean(void * ctx, int boolean)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_bool(g, boolean);
    LOGV("reformat_boolean %d\n", boolean);
    return 1;
}

static int reformat_number(void * ctx, const char * s, size_t l)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_number(g, s, l);
    //LOGV("reformat_number %f\n", strtof(s, (char **)s+l));
    LOGV("reformat_number\n");
    return 1;
}

static int reformat_string(void * ctx, const unsigned char * stringVal,
                           size_t stringLen)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_string(g, stringVal, stringLen);
    //LOGV("reformat_string %s %d\n", stringVal, stringLen);
    LOGV("reformat_string\n");
    return 1;
}

static int reformat_map_key(void * ctx, const unsigned char * stringVal,
                            size_t stringLen)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_string(g, stringVal, stringLen);
    //LOGV("reformat_map_key %s\n", stringVal);
    LOGV("reformat_map_key\n");
    return 1;
}

static int reformat_start_map(void * ctx)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_map_open(g);
    LOGV("reformat_start_map\n");
    return 1;
}


static int reformat_end_map(void * ctx)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_map_close(g);
    LOGV("reformat_end_map\n");
    return 1;
}

static int reformat_start_array(void * ctx)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_array_open(g);
    LOGV("reformat_start_array\n");
    return 1;
}

static int reformat_end_array(void * ctx)
{
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

unsigned int MazeNetwork::get_all_buf(int sock, const unsigned char* output, unsigned int maxsize)
{
  int bytes;

  //read the socket to see how many bytes are there
  if (ioctl(sock, FIONREAD, &bytes)) {
    LOGV("wtf1\n");
    perror("foo\n");
    return 0;
  }

  // how many bytes are available
  if (bytes == 0) {
    //LOGV("wtf\n");
    return 0;
  }

  LOGV("reading: %d of %d\n", maxsize, bytes);

  int n;
  errno = 0;
  n = recv(sock, (void *)output, maxsize, 0);

  if (n>0) {
  } else {
    LOGV("error in get_all_buf!");
  }

  return n;
}


void MazeNetwork::StopNetwork() {
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
}


void MazeNetwork::iter(void *arg) {

  yajl_status stat;

  int n = get_all_buf(SocketFD, out, 10);

  if (n > 0) {
    out[n] = '\0';
    LOGV("read! n=%d out=%s\n", n, out);
    stat = yajl_parse(hand, out, n * sizeof(unsigned char));
    if (stat == yajl_status_ok) {
    } else {
      unsigned char *str = yajl_get_error(hand, 1, out, 1023);
      fprintf(stderr, "%s", (const char *) str);
      yajl_free_error(hand, str);
    }
  }

  char payload[4] = "[1]";
  ssize_t sent = send(SocketFD, payload, 3, 0); //MSG_DONTWAIT
  if (sent > 0) {
    //LOGV("wtf11111 %d payload-sent: %d\n", SocketFD, sent);
  } else {
    //LOGV("send failed\n");
  }
}

int MazeNetwork::ConnectNetwork(void) {
  struct sockaddr_in stSockAddr;
  int Res = 0;
  done = 0;
  SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (-1 == SocketFD)
  {
    LOGV("cannot create socket");
    return 1;
  }

  memset(&stSockAddr, 0, sizeof(stSockAddr));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(7001);

  //struct hostent *host0 = gethostbyname("test.com"); // increment hostname counter to check for possible but at 0,0 not differentiating low/high
  //struct hostent *host = gethostbyname("localhost");
  struct hostent *host = gethostbyname("emscripten.risingcode.com");
  char **addr_list = host->h_addr_list;
  int *addr = (int*)*addr_list;
  LOGV("raw addr: %d\n", *addr);
  char name[INET_ADDRSTRLEN];
  if (!inet_ntop(AF_INET, addr, name, sizeof(name))) {
    LOGV("could not figure out name\n");
    return 1;
  }
  LOGV("localhost has 'ip' of %s\n", name);

  Res = inet_pton(AF_INET, name, &stSockAddr.sin_addr);

  if (0 > Res) {
    LOGV("error: first parameter is not a valid address family");
    close(SocketFD);
    return 1;
  } else if (0 == Res) {
    LOGV("char string (second parameter does not contain valid ipaddress)");
    close(SocketFD);
    return 1;
  }

  if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr))) {
    LOGV("connect failed");
    close(SocketFD);
    return 1;
  }

  int set = 1;
  setsockopt(SocketFD, SOL_SOCKET, MSG_NOSIGNAL, (void *)&set, sizeof(int));

  char magic[1];
  magic[0] = '{';
  ssize_t sent = send(SocketFD, magic, 1, 0); //MSG_DONTWAIT
  //LOGV("wtf11111 %d magic-sent: %d\n", SocketFD, sent);

  out = (unsigned char *) malloc(sizeof(unsigned char) * 1024);

  void *g = NULL;

  hand = yajl_alloc(&callbacks, NULL, (void *) g);
  yajl_config(hand, yajl_allow_comments, 1); // allow json comments
  yajl_config(hand, yajl_dont_validate_strings, 1); // dont validate strings

  return 0;
}


MazeNetwork::~MazeNetwork() {
  LOGV("MazeNetwork::dealloc\n");
}


MazeNetwork::MazeNetwork() {
  ConnectNetwork();
}
