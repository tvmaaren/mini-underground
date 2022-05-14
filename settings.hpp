
#define framerate 40

#define min_seperation_stations 30

#define chance_of_a_new_passenger (int)(0.1/framerate*INT_MAX)
#define chance_of_a_new_station (int)(0.05/framerate*INT_MAX)

#define station_max_passengers 10

#define randomly_add_stations true
#define manually_add_stations false
//#define all_lines_available_from_beginning

#ifdef __ANDROID__
#define font_file "fonts/dejavusanscondensed-bold.ttf"
#else
#define font_file "DejaVuSansCondensed-Bold.ttf"
#endif

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

#ifdef all_lines_available_from_beginning
const int line_unlocked[]={
0,
0,
0,
0,
0,
0};
#else
const int line_unlocked[]={
0,
0,
20,
100,
200,
300};
#endif

