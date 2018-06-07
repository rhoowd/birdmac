/*
 * topology.h
 *
 *  Created on: Nov 24, 2015
 *      Author: user
 */

#ifndef LANADA_APP_TOPOLOGY_H_
#define LANADA_APP_TOPOLOGY_H_

//#include "bird.h"
#include "param.h"

enum tree {SINK, MIDDLE, LEAF};

/* state, level, child_num, parent[0], parent[1],
 * num_content, decent, slot_num, nodeid */

#if TOPOLOGY==1
#define NODE_NUM 2
#define MAX_LEVEL 1
#define MAX_CHILD 1
const unsigned char topology[NODE_NUM][9]
								  =
{
		{SINK,0,1,0xff,0xff,0,1,0,1},
		{LEAF,1,0,1,0,1,0,1,2}
};
#elif TOPOLOGY==2
#define NODE_NUM 2
#define MAX_LEVEL 1
#define MAX_CHILD 1
const unsigned char topology[NODE_NUM][9]
								  =
{
		{SINK,0,1,0xff,0xff,0,1,0,1},
		{LEAF,1,0,1,0,1,0,1,18}
};
#elif TOPOLOGY==101
#define NODE_NUM 2
#define MAX_LEVEL 1
#define MAX_CHILD 1
const unsigned char topology[NODE_NUM][9]
								  =
{
		{SINK,0,1,0xff,0xff,0,1,0,1},
		{LEAF,1,0,1,0,1,0,1,5}
};
#elif TOPOLOGY==3
#define NODE_NUM 4
#define MAX_LEVEL 1
#define MAX_CHILD 3
const unsigned char topology[NODE_NUM][9]
								  =
{
		{SINK,0,3,0xff,0xff,0,3,0,1},
		{LEAF,1,0,1,0,3,0,1,2},
		{LEAF,1,0,1,0,3,0,2,3},
		{LEAF,1,0,1,0,3,0,3,4}
};


#elif TOPOLOGY==11
#define NODE_NUM 5
#define MAX_LEVEL 2
#define MAX_CHILD 3
const unsigned char topology[NODE_NUM][9]
								  =
{
		{SINK  ,0,2,0xff,0xff,0,4,0,1},
		{MIDDLE,1,2,1,0,2,2,1,18},
		{LEAF  ,1,0,1,0,2,0,2,3},
		{LEAF  ,2,0,18,0,2,0,1,19},
		{LEAF  ,2,0,18,0,2,0,2,22}
};
#elif TOPOLOGY==1111
#define NODE_NUM 11
#define MAX_LEVEL 3
#define MAX_CHILD 3
const unsigned char topology[NODE_NUM][9]
								  =
{
		{SINK, 0, 3, 0xff, 0xff, 0, 10, 0, 1},
		{MIDDLE, 1, 1, 1, 0, 3, 1, 1, 2},
		{LEAF, 2, 0, 2, 0, 1, 0, 1, 3},
		{MIDDLE, 1, 3, 1, 0, 3, 3, 2, 4},
		{LEAF, 2, 0, 4, 0, 3, 0, 1, 5},
		{LEAF, 2, 0, 4, 0, 3, 0, 2, 6},
		{LEAF, 2, 0, 4, 0, 3, 0, 3, 7},
		{MIDDLE, 1, 1, 1, 0, 3, 3, 3, 8},
		{MIDDLE, 2, 2, 8, 0, 1, 2, 1, 9},
		{LEAF, 3, 0, 9, 0, 2, 0, 1, 10},
		{LEAF, 3, 0, 9, 0, 2, 0, 2, 11}

};

#elif TOPOLOGY==28
#define NODE_NUM 28
#define MAX_LEVEL 6
#define MAX_CHILD 3
#if BRIDGE_L == 0
const unsigned char topology[NODE_NUM][9]
								  =
{
		{SINK,0,2,0xff,0xff,0,27,0,1},
		{MIDDLE,1,2,1,0,2,15,1,2},
		{MIDDLE,2,3,2,0,2,10,1,3},
		{MIDDLE,3,1,3,0,3,1,1,4},
		{LEAF,4,0,4,0,1,0,1,5},
		{MIDDLE,2,1,2,0,2,3,2,6},
		{MIDDLE,3,1,3,0,3,5,2,7},
		{MIDDLE,3,1,3,0,3,1,3,8},
		{LEAF,4,0,8,0,1,0,1,9},
		{MIDDLE,3,2,6,0,1,2,1,10},
		{MIDDLE,4,1,7,0,1,4,1,11},
		{MIDDLE,5,3,11,0,1,3,1,12},
		{LEAF,6,0,12,0,3,0,1,13},
		{LEAF,4,0,10,0,2,0,1,14},
		{LEAF,4,0,10,0,2,0,2,15},
		{LEAF,6,0,12,0,3,0,2,16},
		{LEAF,6,0,12,0,3,0,3,17},
		{MIDDLE,1,2,1,0,2,10,2,18},
		{MIDDLE,2,1,18,0,2,2,1,19},
		{MIDDLE,3,1,19,0,1,1,1,20},
		{LEAF,4,0,20,0,1,0,1,21},
		{MIDDLE,2,2,18,0,2,6,2,22},
		{MIDDLE,3,2,22,0,2,4,1,23},
		{MIDDLE,4,2,23,0,2,2,1,24},
		{LEAF,5,0,24,0,2,0,1,25},
		{LEAF,3,0,22,0,2,0,2,26},
		{LEAF,4,0,23,0,2,0,2,27},
		{LEAF,5,0,24,0,2,0,2,28}

};
#else
const unsigned char topology[4][9]
								  =
{
		{SINK,0,2,0xff,0xff,0,27,0,1},
		{MIDDLE,1,2,1,0,2,10,2,18},
		{MIDDLE,2,1,18,0,2,2,1,19},
		{MIDDLE,2,2,18,0,2,6,2,22}
};
#endif

#elif TOPOLOGY==100
#if PLATFORM_L != MICAZ_L
#define NODE_NUM 16
#define MAX_LEVEL 5
#define MAX_CHILD 3
const unsigned char topology[NODE_NUM][9]
								  =
{
	{SINK,0,2,0xff,0xff,0,15,0,1},
	{MIDDLE,1,3,1,0,2,10,1,2},
	{MIDDLE,2,1,2,0,1,1,1,3},
	{LEAF,3,0,3,0,1,0,1,4},
	{MIDDLE,1,1,1,0,2,3,2,5},
	{MIDDLE,2,1,2,0,3,5,2,6},
	{MIDDLE,2,1,2,0,3,1,3,7},
	{LEAF,3,0,7,0,1,0,1,8},
	{MIDDLE,2,2,5,0,1,2,1,10},
	{MIDDLE,3,1,6,0,1,4,1,11},
	{MIDDLE,4,3,11,0,1,3,1,12},
	{LEAF,5,0,12,0,3,0,1,13},
	{LEAF,3,0,10,0,2,0,1,14},
	{LEAF,3,0,10,0,2,0,2,15},
	{LEAF,5,0,12,0,3,0,2,16},
	{LEAF,5,0,12,0,3,0,3,17}

};
#elif PLATFORM_L == MICAZ_L
#define NODE_NUM 10
#define MAX_LEVEL 5
#define MAX_CHILD 2
const unsigned char topology[NODE_NUM][9]
								  =
{
	{SINK,0,1,0xff,0xff,0,10,0,18},
	{MIDDLE,1,2,18,0,1,9,1,19},
	{MIDDLE,2,1,19,0,2,1,1,20},
	{LEAF,3,0,20,0,1,0,1,21},
	{MIDDLE,2,2,19,0,2,5,2,23},
	{MIDDLE,3,2,23,0,2,3,1,24},
	{MIDDLE,4,1,24,0,2,1,1,25},
	{LEAF,3,0,23,0,2,0,2,26},
	{LEAF,4,0,24,0,2,0,2,27},
	{LEAF,5,0,25,0,1,0,1,28}

};
#endif

/*#elif TOPOLOGY==111 // For simulation
#define NODE_NUM 16
#define MAX_LEVEL 5
#define MAX_CHILD 3
const unsigned char topology[NODE_NUM][9]
								  =
{
	{SINK,0,2,0xff,0xff,0,15,0,1},
	{MIDDLE,1,3,1,0,2,10,1,2},
	{MIDDLE,2,1,2,0,1,1,1,3},
	{LEAF,3,0,3,0,1,0,1,4},
	{MIDDLE,1,1,1,0,2,3,2,5},
	{MIDDLE,2,1,2,0,3,5,2,6},
	{MIDDLE,2,1,2,0,3,1,3,7},
	{LEAF,3,0,7,0,1,0,1,8},
	{MIDDLE,2,2,5,0,1,2,1,9},
	{MIDDLE,3,1,6,0,1,4,1,10},
	{MIDDLE,4,3,10,0,1,3,1,11},
	{LEAF,5,0,11,0,3,0,1,12},
	{LEAF,3,0,9,0,2,0,1,13},
	{LEAF,3,0,9,0,2,0,2,14},
	{LEAF,5,0,11,0,3,0,2,15},
	{LEAF,5,0,11,0,3,0,3,16}

};*/

#elif TOPOLOGY==222 // For simulation
#define NODE_NUM 7
#define MAX_LEVEL 2
#define MAX_CHILD 2
const unsigned char topology[NODE_NUM][9]
								  =
{
		{SINK,0,2,0xff,0xff,0,6,0,1},
		{MIDDLE,1,2,1,0,2,2,1,2},
		{MIDDLE,1,2,1,0,2,2,2,3},
		{LEAF,2,0,2,0,2,2,1,4},
		{LEAF,2,0,2,0,2,2,2,5},
		{LEAF,2,0,3,0,2,2,1,6},
		{LEAF,2,0,3,0,2,2,2,7}
};
#elif TOPOLOGY==333 // For simulation
#define NODE_NUM 13
#define MAX_LEVEL 2
#define MAX_CHILD 3
const unsigned char topology[NODE_NUM][9]
								  =
{
		{SINK,0,3,0xff,0xff,0,12,0,1},
		{MIDDLE,1,3,1,0,3,3,1,2},
		{MIDDLE,1,3,1,0,3,3,2,3},
		{MIDDLE,1,3,1,0,3,3,3,4},
		{LEAF,2,0,2,0,3,0,1,5},
		{LEAF,2,0,2,0,3,0,2,6},
		{LEAF,2,0,2,0,3,0,3,7},
		{LEAF,2,0,3,0,3,0,1,8},
		{LEAF,2,0,3,0,3,0,2,9},
		{LEAF,2,0,3,0,3,0,3,10},
		{LEAF,2,0,4,0,3,0,1,11},
		{LEAF,2,0,4,0,3,0,2,12},
		{LEAF,2,0,4,0,3,0,3,13}
};
#elif TOPOLOGY==444 // For simulation
#define NODE_NUM 21
#define MAX_LEVEL 2
#define MAX_CHILD 4
const unsigned char topology[NODE_NUM][9]
								  =
{
		{SINK,0,4,0xff,0xff,0,20,0,1},
		{MIDDLE,1,4,1,0,4,4,1,2},
		{MIDDLE,1,4,1,0,4,4,2,3},
		{MIDDLE,1,4,1,0,4,4,3,4},
		{MIDDLE,1,4,1,0,4,4,4,5},
		{LEAF,2,0,2,0,4,0,1,6},
		{LEAF,2,0,2,0,4,0,2,7},
		{LEAF,2,0,2,0,4,0,3,8},
		{LEAF,2,0,2,0,4,0,4,9},
		{LEAF,2,0,3,0,4,0,1,10},
		{LEAF,2,0,3,0,4,0,2,11},
		{LEAF,2,0,3,0,4,0,3,12},
		{LEAF,2,0,3,0,4,0,4,13},
		{LEAF,2,0,4,0,4,0,1,14},
		{LEAF,2,0,4,0,4,0,2,15},
		{LEAF,2,0,4,0,4,0,3,16},
		{LEAF,2,0,4,0,4,0,4,17},
		{LEAF,2,0,5,0,4,0,1,18},
		{LEAF,2,0,5,0,4,0,2,19},
		{LEAF,2,0,5,0,4,0,3,20},
		{LEAF,2,0,5,0,4,0,4,21}
};
#elif TOPOLOGY==555 // For simulation
#define NODE_NUM 31
#define MAX_LEVEL 2
#define MAX_CHILD 5
const unsigned char topology[NODE_NUM][9]
								  =
{
		{SINK,0,5,0xff,0xff,0,30,0,1},
		{MIDDLE,1,5,1,0,5,5,1,2},
		{MIDDLE,1,5,1,0,5,5,2,3},
		{MIDDLE,1,5,1,0,5,5,3,4},
		{MIDDLE,1,5,1,0,5,5,4,5},
		{MIDDLE,1,5,1,0,5,5,5,6},
		{LEAF,2,0,2,0,5,0,1,7},
		{LEAF,2,0,2,0,5,0,2,8},
		{LEAF,2,0,2,0,5,0,3,9},
		{LEAF,2,0,2,0,5,0,4,10},
		{LEAF,2,0,2,0,5,0,5,11},
		{LEAF,2,0,3,0,5,0,1,12},
		{LEAF,2,0,3,0,5,0,2,13},
		{LEAF,2,0,3,0,5,0,3,14},
		{LEAF,2,0,3,0,5,0,4,15},
		{LEAF,2,0,3,0,5,0,5,16},
		{LEAF,2,0,4,0,5,0,1,17},
		{LEAF,2,0,4,0,5,0,2,18},
		{LEAF,2,0,4,0,5,0,3,19},
		{LEAF,2,0,4,0,5,0,4,20},
		{LEAF,2,0,4,0,5,0,5,21},
		{LEAF,2,0,5,0,5,0,1,22},
		{LEAF,2,0,5,0,5,0,2,23},
		{LEAF,2,0,5,0,5,0,3,24},
		{LEAF,2,0,5,0,5,0,4,25},
		{LEAF,2,0,5,0,5,0,5,26},
		{LEAF,2,0,6,0,5,0,1,27},
		{LEAF,2,0,6,0,5,0,2,28},
		{LEAF,2,0,6,0,5,0,3,29},
		{LEAF,2,0,6,0,5,0,4,30},
		{LEAF,2,0,6,0,5,0,5,31}
};
#elif TOPOLOGY==1000 // For simulation
#define NODE_NUM 100
#define MAX_LEVEL 16
#define MAX_CHILD 3
const unsigned char topology[NODE_NUM][9]
								  =
{
		{SINK,  0, 3, 0xff, 0xff, 0, 99, 0, 1},
		{MIDDLE,6, 1, 56, 0, 2, 1, 1, 2},
		{MIDDLE,6, 2, 70, 0, 3, 13, 2, 3},
		{LEAF,  9, 0, 39, 0, 2, 0, 2, 4},
		{LEAF,  6, 0, 70, 0, 3, 0, 3, 5},
		{MIDDLE,8, 1, 49, 0, 2, 1, 2, 6},
		{MIDDLE,6, 1, 56, 0, 2, 3, 2, 7},
		{MIDDLE,9, 2, 65, 0, 2, 2, 2, 8},
		{MIDDLE,4, 1, 35, 0, 1, 30, 1, 9},
		{MIDDLE,5, 3, 79, 0, 1, 10, 1, 10},
		{MIDDLE,3, 1, 91, 0, 2, 6, 1, 11},
		{LEAF,  11, 0, 21, 0, 2, 0, 1, 12},
		{MIDDLE,4, 1, 40, 0, 2, 6, 2, 13},
		{MIDDLE,1, 1, 1, 0, 3, 47, 1, 14},
		{MIDDLE,5, 1, 17, 0, 2, 1, 2, 15},
		{LEAF,  9, 0, 34, 0, 1, 0, 1, 16},
		{MIDDLE,4, 2, 53, 0, 2, 4, 2, 17},
		{MIDDLE,9, 2, 69, 0, 1, 9, 1, 18},
		{MIDDLE,11, 1, 93, 0, 1, 6, 1, 19},
		{MIDDLE,12, 1, 19, 0, 1, 5, 1, 20},
		{MIDDLE,10, 2, 25, 0, 1, 2, 1, 21},
		{MIDDLE,7, 1, 74, 0, 2, 1, 2, 22},
		{LEAF,  8, 0, 85, 0, 1, 0, 1, 23},
		{LEAF,  9, 0, 43, 0, 1, 0, 1, 24},
		{MIDDLE,9, 1, 65, 0, 2, 3, 1, 25},
		{LEAF,  8, 0, 84, 0, 2, 0, 1, 26},
		{MIDDLE,2, 2, 14, 0, 1, 46, 1, 27},
		{LEAF,  4, 0, 99, 0, 2, 0, 1, 28},
		{MIDDLE,7, 1, 7, 0, 1, 2, 1, 29},
		{MIDDLE,2, 2, 82, 0, 2, 16, 1, 30},
		{MIDDLE,6, 3, 10, 0, 3, 3, 1, 31},
		{LEAF,  15, 0, 50, 0, 2, 0, 1, 32},
		{LEAF,  5, 0, 87, 0, 3, 0, 1, 33},
		{MIDDLE,8, 1, 29, 0, 1, 1, 1, 34},
		{MIDDLE,3, 1, 27, 0, 2, 31, 2, 35},
		{MIDDLE,2, 1, 82, 0, 2, 14, 2, 36},
		{MIDDLE,5, 1, 92, 0, 1, 6, 1, 37},
		{LEAF,  8, 0, 78, 0, 1, 0, 1, 38},
		{MIDDLE,8, 2, 84, 0, 2, 2, 2, 39},
		{MIDDLE,3, 2, 30, 0, 2, 14, 1, 40},
		{LEAF,  8, 0, 22, 0, 1, 0, 1, 41},
		{LEAF,  16, 0, 66, 0, 1, 0, 1, 42},
		{MIDDLE,8, 1, 95, 0, 1, 1, 1, 43},
		{MIDDLE,7, 2, 77, 0, 1, 12, 1, 44},
		{LEAF,  7, 0, 98, 0, 1, 0, 1, 45},
		{LEAF,  7, 0, 31, 0, 3, 0, 2, 46},
		{LEAF,  7, 0, 2, 0, 1, 0, 1, 47},
		{LEAF,  10, 0, 18, 0, 2, 0, 2, 48},
		{MIDDLE,7, 2, 3, 0, 2, 10, 1, 49},
		{MIDDLE,14, 2, 90, 0, 1, 3, 1, 50},
		{LEAF,  7, 0, 31, 0, 3, 0, 3, 51},
		{LEAF,  10, 0, 8, 0, 2, 0, 2, 52},
		{MIDDLE,3, 2, 36, 0, 1, 13, 1, 53},
		{MIDDLE,6, 1, 10, 0, 3, 2, 2, 54},
		{LEAF,  7, 0, 31, 0, 3, 0, 1, 55},
		{MIDDLE,5, 2, 89, 0, 1, 6, 1, 56},
		{MIDDLE,5, 1, 13, 0, 1, 5, 1, 57},
		{LEAF,  10, 0, 8, 0, 2, 0, 1, 58},
		{MIDDLE,7, 1, 74, 0, 2, 1, 1, 59},
		{MIDDLE,3, 1, 91, 0, 2, 8, 2, 60},
		{MIDDLE,5, 1, 87, 0, 3, 2, 2, 61},
		{LEAF,  8, 0, 64, 0, 1, 0, 1, 62},
		{LEAF,  9, 0, 6, 0, 1, 0, 1, 63},
		{MIDDLE,7, 1, 54, 0, 1, 1, 1, 64},
		{MIDDLE,8, 2, 49, 0, 2, 7, 1, 65},
		{MIDDLE,15, 1, 50, 0, 2, 1, 2, 66},
		{LEAF,  5, 0, 87, 0, 3, 0, 3, 67},
		{MIDDLE,6, 1, 10, 0, 3, 2, 3, 68},
		{MIDDLE,8, 1, 44, 0, 2, 10, 2, 69},
		{MIDDLE,5, 3, 9, 0, 1, 29, 1, 70},
		{MIDDLE,6, 1, 100, 0, 2, 3, 2, 71},
		{LEAF,  9, 0, 39, 0, 2, 0, 1, 72},
		{LEAF,  11, 0, 21, 0, 2, 0, 2, 73},
		{MIDDLE,6, 2, 57, 0, 1, 4, 1, 74},
		{LEAF,  3, 0, 30, 0, 2, 0, 2, 75},
		{LEAF,  6, 0, 15, 0, 1, 0, 1, 76},
		{MIDDLE,6, 1, 70, 0, 3, 13, 1, 77},
		{MIDDLE,7, 1, 68, 0, 1, 1, 1, 78},
		{MIDDLE,4, 1, 99, 0, 2, 11, 2, 79},
		{LEAF,  6, 0, 100, 0, 2, 0, 1, 80},
		{LEAF,  8, 0, 44, 0, 2, 0, 1, 81},
		{MIDDLE,1, 2, 1, 0, 3, 32, 2, 82},
		{MIDDLE,5, 1, 17, 0, 2, 1, 1, 83},
		{MIDDLE,7, 2, 97, 0, 1, 4, 1, 84},
		{MIDDLE,7, 1, 3, 0, 2, 1, 2, 85},
		{MIDDLE,4, 1, 40, 0, 2, 6, 1, 86},
		{MIDDLE,4, 3, 11, 0, 1, 5, 1, 87},
		{MIDDLE,1, 1, 1, 0, 3, 17, 3, 88},
		{MIDDLE,4, 1, 53, 0, 2, 7, 1, 89},
		{MIDDLE,13, 1, 20, 0, 1, 4, 1, 90},
		{MIDDLE,2, 2, 88, 0, 1, 16, 1, 91},
		{MIDDLE,4, 1, 60, 0, 1, 7, 1, 92},
		{MIDDLE,10, 1, 18, 0, 2, 7, 1, 93},
		{LEAF,  8, 0, 59, 0, 1, 0, 1, 94},
		{MIDDLE,7, 1, 71, 0, 1, 2, 1, 95},
		{LEAF,  6, 0, 83, 0, 1, 0, 1, 96},
		{MIDDLE,6, 1, 37, 0, 1, 5, 1, 97},
		{MIDDLE,6, 1, 61, 0, 1, 1, 1, 98},
		{MIDDLE,3, 2, 27, 0, 2, 13, 1, 99},
		{MIDDLE,5, 2, 86, 0, 1, 5, 1, 100}
};




#endif
#endif /* __TOPOLOGY_H__ */
