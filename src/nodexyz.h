// Jon Bardin GPL


class nodexyz {

public:

  int node_index;
  int x;
  int y;
  int z;


  bool operator==(const nodexyz &other) const {
    return (this->x == other.x && this->y == other.y);
  }


  bool operator!=(const nodexyz &other) const {
    return !(*this == other);
  }


};
