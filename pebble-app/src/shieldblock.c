#include <pebble.h>
#include "shieldblock.h"

ShieldBlock *block_create(GRect rect) {
  ShieldBlock *block = (ShieldBlock *) malloc (sizeof (ShieldBlock));
  block->rect = rect;
  block->hidden = false;
  
  return block;
}

void block_draw(ShieldBlock *block, GContext *context) {
  if (block->hidden == true) {
    graphics_context_set_fill_color(context, GColorBlack);
  } else {
    graphics_context_set_fill_color(context, COLOR_FALLBACK(GColorBrightGreen, GColorWhite));
  }
  graphics_fill_rect(context, block->rect, 0, GCornerNone);
}

void block_destroy(ShieldBlock *block) {
  free(block);
}
