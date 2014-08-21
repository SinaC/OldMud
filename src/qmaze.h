#ifndef _QMAZE_H_         /* fix multi includes */
#define _QMAZE_H_         /* fix multi includes */


struct MazeRoom {
  bool open[4]; // tells if the exits are open
  bool used;    // if the room is already visited
  int x, y;     // position of the room

  MazeRoom(){
    for (int i = 0; i<4; i++)
      open[i] = false;
    used = false;
  }

};

struct MazeQ {
  MazeRoom* room;
  int dir;         // where are we coming from in the visit ?
  MazeQ* next;

  MazeQ(MazeRoom* r, int d) {room=r; dir=d; next=NULL;};

};

struct MazeInfo {

  MazeQ* head;
  MazeQ* tail;

  MazeRoom* data;
  int w,h;

  MazeInfo(int w, int h) {
    this->w = w;
    this->h = h;

    data = new MazeRoom[w*h];    

    for (int x = 0; x<w; x++)
      for (int y = 0; y<h; y++) {
	MazeRoom& r= get(x,y);
	r.x = x;
	r.y = y;
      }
  }

  ~MazeInfo() {
    delete data;
  }

  MazeRoom & get(int i, int j) {
    return data[i+j*w];
  }
  MazeRoom & get( int off ){
    return data[off];
  }

  void print( char *maze_map);
  void visit();
};

// Added by SinaC 2000
/* qmaze.C */
void    reset_maze      args( ( int from, int width, int height, char *maze_map ) );


#endif                  /* end of fix multi includes */
