#pragma once

#include "pge.h"
#include "pge_sprite.h"

typedef struct {
  GRect rect;
  bool hidden;
} ShieldBlock;

ShieldBlock *block_create(GRect rect);

void block_draw(ShieldBlock *block, GContext *context);

void block_destroy(ShieldBlock *block);
