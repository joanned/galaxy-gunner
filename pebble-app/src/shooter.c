#include "shooter.h"
#include "constants.h"

#define SHOOTER_SPEED 3
  
void shooter_draw(Shooter *shooter, GContext *context) {
	pge_sprite_draw(shooter->sprite, context);
}

Shooter* shooter_create(GPoint position) {
	Shooter *shooter = malloc(sizeof(Shooter));

	shooter->sprite = pge_sprite_create(position, RESOURCE_ID_SHOOTER);

	return shooter;
}

void shooter_destroy(Shooter *shooter) {
	pge_sprite_destroy(shooter->sprite);
  free(shooter);
}

void shooter_move(Shooter *shooter, direction shooter_direction, int screen_width) {
  GPoint position = pge_sprite_get_position(shooter->sprite);
  
  if (shooter_direction == RIGHT && position.x < screen_width - SHOOTER_WIDTH) {
    position.x += SHOOTER_SPEED;
  } else if (shooter_direction == LEFT && position.x > 0) {
    position.x -= SHOOTER_SPEED;
  }
  
  pge_sprite_set_position(shooter->sprite, position);
}

void shooter_set_position(Shooter *shooter, GPoint position) {
  pge_sprite_set_position(shooter->sprite, position);
}

