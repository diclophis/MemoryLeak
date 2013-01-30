//
//  Network.h
//  MemoryLeak
//
//  Created by Jon Bardin on 2012.
//

class MazeNetworkDelegate {
public:
//  virtual bool UpdatePlayerAtIndex(int i, float x, float y);
  virtual bool RequestRegistration(int i) = 0;
};

class MazeNetwork {


public:


  MazeNetwork(MazeNetworkDelegate *theDelegate, size_t bpt);
  virtual ~MazeNetwork();
  int Tick();

  MazeNetworkDelegate *m_Delegate;

  //network stuff
  int m_Socket;
  int ConnectNetwork(void);
  int StopNetwork();

  yajl_handle hand;
  unsigned char *m_InputBuffer;
  int m_InputBufferSize;
  size_t bpt;

  struct sockaddr_in stSockAddr;

  int m_State;


  float m_Arg0;
  float m_Arg1;
  float m_Arg2;

  int m_ConnectionState;
  int m_ConnectionSelectsAttempted;


};
