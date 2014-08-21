//
//
//  Random generated maze coded by SinaC & JyP (aka Oxtal)
// 
//

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "qmaze.h"
//#include "olc.h"

// Added by SinaC 2001
#include "handler.h"
#include "db.h"
#include "mem.h"


int myRange( int a, int b ) {
  if( b == 0 || a < 0 ) {
    return 0;
  }
  else {
    return (( rand() % (b-a) ) + a + 1);
  }
}

//             N  E  S  W
int dirx[] = { 0, 1, 0,-1};
int diry[] = {-1, 0, 1, 0};
int opposite_dir[] = {2, 3, 0, 1};

void MazeInfo::visit() {
  int qsize=0;

  // start at a random poiunt.
  MazeRoom *start = &get(myRange(0, w-1), myRange(0, h-1));
  tail = head = new MazeQ(start, -1);    
  qsize = 1;
  
  while (head != NULL /*qsize > 0*/) {

    //get some random room we have access to.
    int n = myRange(0, qsize-1);
    MazeQ* q = head;
    MazeQ* p = NULL;

    for (int i=0; i<n; i++) {
      p = q;
      q = q->next;
    }

    MazeRoom& room = *q->room;
  
    if (!room.used) {
      if (q->dir != -1) {
	int dir = q->dir;
	//open doors.
	room.open[opposite_dir[dir]] = true;
	get(room.x - dirx[dir], room.y - diry[dir]).open[dir] = true;
	//ouf!
      }     
      room.used = true; // now, there is a path to this room

      // for all adjacent rooms, add them in the queue
      for (int dir = 0; dir<4; dir++) {
	int ax = room.x+dirx[dir];
	int ay = room.y+diry[dir];
      
	if (ax < 0 || ax >= w || ay < 0 || ay >= h)
	  continue;

	MazeRoom &next = get(ax,ay);

	if (next.used) // use less to put an already visited room
	  continue;
      
	tail->next = new MazeQ(&next, dir); // put it.
	tail = tail->next;
	qsize++;
      }
    }

    // done with it.
    if (p!=NULL)
      p->next = q->next;
    else
      head = q->next;
    if (q->next == NULL)
      tail = p;
    delete q;
    qsize--;
  }
}

void MazeInfo::print( char *maze_map) {
  char buf[100];

  maze_map[0]='\0';

  for ( int j = 0; j < h; j++ ){
    for ( int i = 0; i < w; i++ ){
      sprintf(buf," %s ",
	      get(i,j).open[0]?"|":" " );
      strcat(maze_map,buf);
    }
    strcat(maze_map,"\n\r");
    for ( int i = 0; i < w; i++ )    {
      sprintf(buf,"%s*%s", 
	      get(i,j).open[3]?"-":" ",
	      get(i,j).open[1]?"-":" "
	      );
      strcat(maze_map,buf);
    }
    strcat(maze_map,"\n\r");
    //we can remove that because the maze is coherent, SinaC 2000
    // if there is a path to south, the south room will have a north path
    /*   
    for ( int i = 0; i < w; i++ ){
      sprintf(buf," %s ",
	      get(i,j).open[2]?"|":" " );
      strcat(maze_map,buf);
    }
    strcat(maze_map,"\n\r");
    */
  }
  strcat(maze_map,"\n\r"
	 "  *    represents rooms.\n\r"
	 "  - |  represents passages.\n\r");
}

// used in db.C
void reset_maze( int startvnum, int width, int height, char *maze_map )
{
  // Create an empty maze
  MazeInfo maze( width, height );
  
  // Make paths in the maze
  maze.visit();
  
  // Create the map
  maze.print( maze_map );
  
  int vnum, off; 
  
  // offset in maze data
  off = 0;
  for ( vnum = startvnum; vnum < startvnum + width * height; vnum++ ) {
    int x = (vnum-startvnum) % maze.w; // position of the room
    int y = (vnum-startvnum) / maze.w;
    
    ROOM_INDEX_DATA *pRoomIndex;
    
    pRoomIndex = get_room_index( vnum );
    if ( pRoomIndex == NULL ) {
      bug("UpdateMaze: room %d doesnt exist!!",vnum);
      return;
    }
    // destroy every door and put new one, ignore Up, Down, Northeast, Northwest, Southeast and Southwest
    // N E S W U D
    for ( int door = 0; door < 4; door++ ) {	 
      int ax = x+dirx[door]; // position of potential exit
      int ay = y+diry[door];
      
      // if the room is outside the maze, leave exit as is
      if (ax<0 || ay<0 || ax>=maze.w || ay>=maze.h)
	continue;
      
      ROOM_INDEX_DATA *toroom;
      
      // if there is an exit, delete it
      if ( pRoomIndex->exit[door] ) {
	pRoomIndex->exit[door] = NULL;
      }
      
      // create the new exit
      if ( maze.get(vnum-startvnum).open[door]) {
	toroom = get_room_index( vnum+dirx[door]+diry[door]*maze.w );
	pRoomIndex->exit[door] = new_exit();
	pRoomIndex->exit[door]->u1.to_room = toroom;
	pRoomIndex->exit[door]->orig_door = door;
      }
    }
    // next room of the maze
    off++;
  }
}
