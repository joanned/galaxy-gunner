#pragma once
  
#include "pge.h"
#include "pge_sprite.h"
#include "constants.h"

typedef struct {
  PGESprite *sprite;
} Invader;

Invader *invader_create(GPoint position, invader_type type);

void invader_destroy(Invader *invader);

void invader_draw(Invader *invader, GContext *context);

void invader_move(Invader *invader, direction invader_direction, bool move_down, int speed);