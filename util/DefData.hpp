#include "Image.hpp"

typedef struct _TIMER {
    unsigned int expire;
    void* routine;
    void* arg;
    short timer_id;
    bool last_check = 0;
} TIMER;

typedef enum _err {
    ERR_NONE = 0,


};

typedef struct _PARAM {
    int dim;
    int* region; //half size 
    int ground;
};

typedef enum groundtype
{
    BaseballHome,
    BaseballGround,
    BasketballHalf,
    BasketballGround,
    Boxing,
    IceLinkHalf,
    IceLink,
    SoccerHalf,
    Soccer,
    Taekwondo,
    TennisHalf,
    Tennis,
    Ufc,
    VolleyballHalf,
    VolleyballGround,
    Football
};