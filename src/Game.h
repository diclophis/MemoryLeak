// Jon Bardin GPL

class Game {
    public:
        virtual ~Game(){}
        virtual void* allocate(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s)const=0;
        virtual void* cast(void* obj)const=0;
};

template<typename T> class GameImpl : public Game {
      public:
         void* allocate(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s)const{ return new T(w, h, t, m, l, s); }
         void* cast(void* obj)const{ return static_cast<T*>(obj); }
};
