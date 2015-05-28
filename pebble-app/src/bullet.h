#pragma once

#include "pge.h"
#include "pge_sprite.h"
#include "constants.h"

typedef struct {
  GRect rect;
  bool is_shooter;
  direction bullet_direction;
  bool powerup;
} Bullet;

Bullet *bullet_create(GRect rect, bool is_shooter, direction bullet_direction, bool powerup);

void bullet_draw(Bullet *bullet, GContext *context);

void bullet_destroy(Bullet *bullet);

void bullet_move(Bullet *bullet, int x, int y);


