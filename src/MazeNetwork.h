//
//  Network.h
//  MemoryLeak
//
//  Created by Jon Bardin on 2012.
//


class MazeNetwork {


public:


  MazeNetwork();
  virtual ~MazeNetwork();


  //network stuff
  int done;
  int SocketFD;
  unsigned int get_all_buf(int sock, const unsigned char* output, unsigned int maxsize);
  void iter(void *arg);
  int ConnectNetwork(void);
  void StopNetwork();

  yajl_handle hand;
  unsigned char *out;


};
