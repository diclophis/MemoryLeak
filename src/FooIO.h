//magic FooIO sauce

class FooStream : public Assimp::IOStream {
  public:
    FooStream(foo &a) : m_Foo(&a) {
	  };

	  ~FooStream() {
    };
	
	  size_t Read(void* buffer, size_t size, size_t count) {
		  fseek(m_Foo->fp, m_Foo->off, SEEK_SET);
		  size_t r = fread(buffer, size, count, m_Foo->fp);
		  return r;
	  }
	
	  size_t Write( const void* buffer, size_t size, size_t count) {
		  return 0;
	  }
	
	  aiReturn Seek( size_t offset, aiOrigin origin)
	  {
		  int seeked;
	    switch (origin)
	    {
	    case aiOrigin_SET:
				seeked = fseek(m_Foo->fp, offset, SEEK_SET);

	      break;
	    case aiOrigin_CUR:
				seeked = fseek(m_Foo->fp, offset, SEEK_CUR);

	      break;
	    case aiOrigin_END:
				seeked = fseek(m_Foo->fp, offset, SEEK_END);

	      break;
	    default:
        throw 1;
	    }

		  if (seeked == 0) {
			  return aiReturn_SUCCESS;
		  } else {
			  return aiReturn_FAILURE;
		  }

	  }
	
	  size_t Tell() const
	  {
		  return ftell(m_Foo->fp);
	  }
	
	  size_t FileSize() const
	  {
		  return m_Foo->len;
	  }
	
	  void Flush() {
      fflush(m_Foo->fp);
	  }
	
	private:
	
		foo *m_Foo;

	};
	
class FooSystem : public Assimp::IOSystem {

	public:
		
		FooSystem(std::vector<GLuint> &a,std::vector<foo*> &b) : m_Textures(&a), m_Models(&b) {
		};
	
	  ~FooSystem() {
	  };
	
	  // Check whether a specific file exists
	  bool Exists(const char* file) const
	  {
	    return true;
	  }
	
	  // Get the path delimiter character we'd like to see
	  char getOsSeparator() const
	  {
	    return '/';
	  }
	
	  // ... and finally a method to open a custom stream
	  Assimp::IOStream* Open(const char* file, const char* mode)
	  {
      int index = atoi(file);
		  return new FooStream(*m_Models->at(index));
	  }
	
	  void Close(Assimp::IOStream* stream) {
		  delete stream;
	  }
	
	private:
		std::vector<GLuint> *m_Textures;
		std::vector<foo*> *m_Models;
};
