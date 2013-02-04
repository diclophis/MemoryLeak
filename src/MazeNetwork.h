//
//  Network.h
//  MemoryLeak
//
//  Created by Jon Bardin on 2012.
//

class MazeNetworkDelegate {
public:
  virtual bool UpdatePlayerAtIndex(int i, float x, float y, float a, float b) = 0;
  virtual bool RequestRegistration(int i) = 0;
};

class MazeNetwork {


public:


  MazeNetwork(MazeNetworkDelegate *theDelegate, size_t bpt);
  virtual ~MazeNetwork();
  int Tick(float, float, float, float);

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
  float m_Arg3;
  float m_Arg4;
  float m_Arg5;

  int m_ConnectionState;
  int m_ConnectionSelectsAttempted;

  int m_MessageIndex;


};
