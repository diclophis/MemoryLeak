// Jon Bardin


class Audio {
	
public:
	
	Audio();
	~Audio();

  void buzz();

private:

  static int saw(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, void *userData);

};
