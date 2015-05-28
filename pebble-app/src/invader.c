#include <pebble.h>
#include "invader.h"
  
void invader_draw(Invader *invader, GContext *context) {
	pge_sprite_draw(invader->sprite, context);
}

Invader* invader_create(GPoint position, invader_type type) {
	Invader *invader = malloc(sizeof(Invader));

  if (type == INVADER_RESOURCE_HAPPY) { 
    invader->sprite = pge_sprite_create(position, RESOURCE_ID_INVADER1);  
  } else if (type == INVADER_RESOURCE_SQUID) {
    invader->sprite = pge_sprite_create(position, RESOURCE_ID_INVADER2);
  } else {
    invader->sprite = pge_sprite_create(position, RESOURCE_ID_INVADER3);
  }
	
  return invader;
}

void invader_destroy(Invader *invader) {
	pge_sprite_destroy(invader->sprite);
  free(invader);
}

void invader_move(Invader *invader, direction invader_direction, bool move_down, int speed) {
  GPoint position = pge_sprite_get_position(invader->sprite);
  
  if (invader_direction == RIGHT) {
    position.x += speed;
  } else if (invader_direction == LEFT) {
    position.x -= speed;
  }
  
  if (move_down) {
    position.y += 2;
  }
  
  pge_sprite_set_position(invader->sprite, position);
}
