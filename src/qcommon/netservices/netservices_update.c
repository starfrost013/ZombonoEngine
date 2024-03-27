#include "netservices.h"


#if defined(_DEBUG) || !defined(NDEBUG) // gcc
game_update_channel current_channel = update_channel_debug;
#elif PLAYTEST
game_update_channel current_channel = update_channel_playtest;
#else
game_update_channel current_channel = update_channel_release;
#endif
