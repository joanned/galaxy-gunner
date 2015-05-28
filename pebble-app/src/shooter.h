#pragma once

#include "pge.h"
#include "pge_sprite.h"
#include "constants.h"

typedef struct {
  PGESprite *sprite;
} Shooter;

Shooter *shooter_create(GPoint position);

void shooter_destroy(Shooter *shooter);

void shooter_draw(Shooter *shooter, GContext *context);

void shooter_move(Shooter *shooter, direction shooter_direction, int screen_width);

void shooter_set_position(Shooter *shooter, GPoint position);