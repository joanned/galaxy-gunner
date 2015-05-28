#include <pebble.h>
#include "bullet.h"

Bullet *bullet_create(GRect rect, bool is_shooter, direction bullet_direction, bool powerup) {
  Bullet *bullet = (Bullet *) malloc (sizeof (Bullet));
  bullet->rect = rect;
  bullet->is_shooter = is_shooter;
  bullet->bullet_direction = bullet_direction;
  bullet->powerup = powerup;
  
  return bullet;
}

void bullet_draw(Bullet *bullet, GContext *context) {
  if (bullet->is_shooter) {
    int x_coord = bullet->rect.origin.x;
    int y_coord = bullet->rect.origin.y;

    /* draws the shooter bullet as a triangle */
    if (bullet->powerup) {
      graphics_context_set_fill_color(context, COLOR_FALLBACK(GColorRed, GColorWhite));
    } else {
      graphics_context_set_fill_color(context, COLOR_FALLBACK(GColorBrightGreen, GColorWhite));
    }

    const GPathInfo path_info = {
    .num_points = 3,
    .points = (GPoint []) {{x_coord + SHOOTER_BULLET_WIDTH/2, y_coord},
                           {x_coord, y_coord + SHOOTER_BULLET_HEIGHT},
                           {x_coord + SHOOTER_BULLET_WIDTH, y_coord + SHOOTER_BULLET_HEIGHT}}
    };
    
    GPath *triangle_path = gpath_create(&path_info);

    if (bullet->bullet_direction == LEFT) {
      gpath_rotate_to(triangle_path, -POWERUP_BULLET_ANGLE);
    } else if (bullet->bullet_direction == RIGHT) {
      gpath_rotate_to(triangle_path, POWERUP_BULLET_ANGLE);
    }

    gpath_draw_filled(context, triangle_path);
    gpath_destroy(triangle_path);

  } else {
    graphics_context_set_fill_color(context, GColorWhite);
    graphics_fill_rect(context, bullet->rect, 2, GCornersAll);
  }
}

void bullet_destroy(Bullet *bullet) {
  free(bullet);
}

void bullet_move(Bullet *bullet, int x, int y) {
  bullet->rect.origin.x += x;
  bullet->rect.origin.y += y;
}
