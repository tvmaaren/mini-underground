
#define framerate 40

#define min_seperation_stations 30

#define chance_of_a_new_passenger (int)(0.1/framerate*INT_MAX)
#define chance_of_a_new_station (int)(0.05/framerate*INT_MAX)

#define station_max_passengers 10

#define font_file "DejaVuSansCondensed-Bold.ttf"

#define am_lines 6

/*const COLOUR line_colours[]={
{255,0,255},
{255,0,0},
{0,255,0},
{255,255,0},
{0,0,255},
{0,0,0},
{0,255,255}};*/

const Uint32 line_colours[]={
0xFFFF00FF,
0xFFFF0000,
0xFF00FF00,
0xFFFFFF00,
0xFF0000FF,
0xFF00FFFF};
