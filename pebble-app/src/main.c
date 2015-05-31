#include <pebble.h>

#include "pge.h"
#include "pge_sprite.h"
#include "pge_collision.h"
#include "pge_title.h"

#include "shooter.h"
#include "invader.h"
#include "bullet.h"
#include "shieldblock.h"

static Window *game_window;
static Shooter *shooter;
static Invader *invaders[MAX_NUM_INVADERS] = {NULL};
static Bullet *shooter_bullets[NUM_BULLETS] = {NULL};
static Bullet *invader_bullets[NUM_INVADER_BULLETS] = {NULL};
static ShieldBlock *blocks[NUM_SHIELD_BLOCKS] = {NULL};

static GFont *score_font = NULL;
static GFont *gameover_font = NULL;
static GFont *final_scores_font = NULL;

/* holds a 2 letter string for 2 digit level */
static char level_str[16] = "";
static int level = 1;

/* holds a 4 letter string for a score up to 4 digits */
static char score_str[32] = "";
static int score = 0;

static int invader_interval = 0;

static GPoint shooter_initial_position;
static GSize screen_size;

static int num_invaders_left = MAX_NUM_INVADERS;
static direction invaders_direction = RIGHT;
static direction powerup_invader_direction = RIGHT;
static bool powerup = false;
static bool gameover = false;
static int previous_highscore = 0;
static bool click_enabled = false;
static bool once_token = false;
   
static AppTimer *shooter_timer; 
static AppTimer *invader_bullet_timer;
static AppTimer *invader_move_timer;
static AppTimer *powerup_timer;

static Invader *powerup_invader = NULL;

static void game_start();

static void create_shooter_bullet(direction shooter_direction, GPoint position) {
  for (int i = 0; i < NUM_BULLETS; i++) {
    if (!shooter_bullets[i]) {
      shooter_bullets[i] = bullet_create(GRect(position.x + SHOOTER_WIDTH/2-2, 
                                         position.y, 
                                         SHOOTER_BULLET_WIDTH, 
                                         SHOOTER_BULLET_HEIGHT), 
                                         true,
                                         shooter_direction,
                                         powerup);
      break;
    }
  }
}

/* timer automatically fire shooter bullets */
static void shooter_fire_bullet() {
  GPoint shooter_position = pge_sprite_get_position(shooter->sprite);
  
  /* if in powerup mode, shoot 3 bullets at once */
  if (powerup) {
    create_shooter_bullet(LEFT, shooter_position);
    create_shooter_bullet(RIGHT, shooter_position);
  }

  create_shooter_bullet(UP, shooter_position);
  
  shooter_timer = app_timer_register(SHOOTER_FIRE_INTERVAL, shooter_fire_bullet, NULL);
}

/* timer randomly fire invader bullets */
static void invader_fire_bullet() {
  
  /* randomly generate which invader to fire from */
  int rand_invader = rand() % num_invaders_left;
  
  GPoint invader_position = pge_sprite_get_position(invaders[rand_invader]->sprite);
  
  for (int i = 0; i < NUM_INVADER_BULLETS; i++) {
      if (!invader_bullets[i]) {
        invader_bullets[i] = bullet_create(GRect(invader_position.x + INVADER_WIDTH/2, 
                                                  invader_position.y + INVADER_HEIGHT, 
                                                  INVADER_BULLET_WIDTH,
                                                  INVADER_BULLET_HEIGHT),
                                                  false,
                                                  DOWN,
                                                  false); 
        break;
      }
  }
  
  /* randomly generate time until next fire, depending on the level */
  /* the higher the level, the smaller the randomly generated speed's range */
  int min;
  int max;
  if (level < INVADER_BULLET_LEVEL_CAP) {
    min = INVADER_BULLET_MIN_SPEED - level * INVADER_BULLET_MIN_DECREASE;
    max = INVADER_BULLET_MAX_SPEED - level * INVADER_BULLET_MAX_DECREASE; 
  } else {
    min = INVADER_BULLET_MIN_SPEED - INVADER_BULLET_LEVEL_CAP * INVADER_BULLET_MIN_DECREASE;
    max = INVADER_BULLET_MAX_SPEED - INVADER_BULLET_LEVEL_CAP * INVADER_BULLET_MAX_DECREASE;
  }
  int rand_interval = (rand() % (max + 1 -min)) + min;
  
  invader_bullet_timer = app_timer_register(rand_interval, invader_fire_bullet, NULL);
}

/* timer to move the invaders */
static void invader_move_timer_callback() {
  bool changed_direction = false;
  
  for (int i = 0; i < num_invaders_left; i++) {    
    int invader_x_position = pge_sprite_get_position(invaders[i]->sprite).x;
      
    /* determine if invaders should change directions */
    if (invader_x_position + INVADER_WIDTH >= screen_size.w) {
      invaders_direction = LEFT;
      changed_direction = true;
      break;
    } else if (invader_x_position <= 0) {
      invaders_direction = RIGHT;
      changed_direction = true;
      break;
    }
  }
  
  /* moving the invaders left/right */ 
  int speed = level + (MAX_NUM_INVADERS - num_invaders_left)/5;
  for (int i = 0; i < num_invaders_left; i++) {
    if (changed_direction == true) {
      invader_move(invaders[i], invaders_direction, true, speed);
    } else {
      invader_move(invaders[i], invaders_direction, false, speed);
    }
  }
  
  invader_move_timer = app_timer_register(invader_interval, invader_move_timer_callback, NULL);
}

/* disable powerup once time is up */
void powerup_over_callback() {
  powerup = false;
}

static void reset() {  
  num_invaders_left = MAX_NUM_INVADERS;
  invaders_direction = RIGHT;
  powerup = false;
  once_token = false;

  /* destroy shooter bullets still deployed */
  for (int i = 0; i < NUM_BULLETS; i++) { 
    if (shooter_bullets[i]) {
      bullet_destroy(shooter_bullets[i]);
			shooter_bullets[i] = NULL;
    }
  }
  
  /* destroy the invader bullets still deployed */
  for (int i = 0; i < NUM_INVADER_BULLETS; i++) {
    if (invader_bullets[i]) {
      bullet_destroy(invader_bullets[i]);
			invader_bullets[i] = NULL;
    }
  }
  
  /* create the first, third , and fifth rows of invaders */
  int x_coord;
  for (int i = 0; i < NUM_ROW1_INVADERS; i++) {
      x_coord = COLUMN_PADDING*(i + 1) + INVADER_WIDTH*i;
      invaders[i] = invader_create(GPoint(x_coord, TOP_PADDING + INVADER_HEIGHT + ROW_PADDING), INVADER_RESOURCE_HAPPY);
      invaders[i+13] = invader_create(GPoint(x_coord, TOP_PADDING + INVADER_HEIGHT*3 + ROW_PADDING*3), INVADER_RESOURCE_HAPPY);
      invaders[i+26] = invader_create(GPoint(x_coord, TOP_PADDING + INVADER_HEIGHT*5 + ROW_PADDING*5), INVADER_RESOURCE_HAPPY);
  }
  
  /* create the second and fourth rows of invaders */
  for (int i = 0; i < NUM_ROW2_INVADERS; i++) {
    x_coord = COLUMN_PADDING + INVADER_WIDTH/2 + COLUMN_PADDING*(i + 1) + INVADER_WIDTH*i;
    invaders[i + 7] = invader_create(GPoint(x_coord, TOP_PADDING + INVADER_HEIGHT*2 + ROW_PADDING*2), INVADER_RESOURCE_SQUID);
    invaders[i + 20] = invader_create(GPoint(x_coord, TOP_PADDING + INVADER_HEIGHT*4 + ROW_PADDING*4), INVADER_RESOURCE_SQUID);
  }

  /* create the powerup invader */
  if (!powerup_invader) {
    x_coord = COLUMN_PADDING;
    powerup_invader = invader_create(GPoint(x_coord, TOP_PADDING), INVADER_RESOURCE_POWERUP);
  }
}

/* determine the speed of invaders by level */
void calculate_invader_interval() {
  if (level < INVADER_SPEED_LEVEL_CAP) {
    invader_interval = INVADER_MAX_INTERVAL - level*INVADER_INTERVAL_DECREASE_PER_LEVEL;
  } else {
    invader_interval = INVADER_MIN_INTERVAL;
  }
}

static void next_level() {
  shooter_set_position(shooter, shooter_initial_position);
  reset();
  
  /* unhide all shield blocks */
  for (int i = 0; i < NUM_SHIELD_BLOCKS; i++) {
    if (blocks[i]->hidden == true) {
      blocks[i]->hidden = false;
    }
  }
  
  level++;

  calculate_invader_interval();
}

static void shield_logic() {
  /* collision check with the shield blocks */
  for (int i = 0; i < NUM_SHIELD_BLOCKS; i++) {
    if (blocks[i]->hidden == false) {
      GRect block_rect = blocks[i]->rect;

      /* shield and shooter bullet collision check */
      for (int j = 0; j < NUM_BULLETS; j++) {
        if (shooter_bullets[j]) {
          GRect bullet_rect = shooter_bullets[j]->rect;
          if (pge_collision_rectangle_rectangle(&bullet_rect, &block_rect)) {
            blocks[i]->hidden = true;

            bullet_destroy(shooter_bullets[j]);
            shooter_bullets[j] = NULL;

            break;
          }
        }
      }

      /* shield and invader bullet collision check */
      for (int j = 0; j < NUM_INVADER_BULLETS; j++) {
        if (invader_bullets[j]) {
          GRect invader_bullet_rect = invader_bullets[j]->rect;
          if (pge_collision_rectangle_rectangle(&invader_bullet_rect, &block_rect)) {
            blocks[i]->hidden = true;

            bullet_destroy(invader_bullets[j]);
            invader_bullets[j] = NULL;

            break;
          }
        }
      }

      /* shield and invader collision check */
      for (int j = 0; j < num_invaders_left; j++) {
        GRect invader_rect = pge_sprite_get_bounds(invaders[j]->sprite);
        if (pge_collision_rectangle_rectangle(&invader_rect, &block_rect)) {
          blocks[i]->hidden = true;
          break;
        }
      }
    }
  }
}

static void shooter_bullet_logic() {
  for (int i = 0; i < NUM_BULLETS; i++) {
    if (shooter_bullets[i]) {
      if (shooter_bullets[i]->bullet_direction == LEFT) {
        bullet_move(shooter_bullets[i], -1, -SHOOTER_BULLET_SPEED);
      } else if (shooter_bullets[i]->bullet_direction == RIGHT) {
        bullet_move(shooter_bullets[i], 1, -SHOOTER_BULLET_SPEED);
      } else {
        bullet_move(shooter_bullets[i], 0, -SHOOTER_BULLET_SPEED);
      }
      GRect bullet_rect = shooter_bullets[i]->rect;

      /* destroy the bullets when they go off screen */
      if (shooter_bullets[i]->rect.origin.y <= 0 ) {
        bullet_destroy(shooter_bullets[i]);
        shooter_bullets[i] = NULL;
        continue;
      }

      /* shooter bullet and invaders collision check */
      for (int j = 0; j < num_invaders_left; j++) {
        GRect invader_rect = pge_sprite_get_bounds(invaders[j]->sprite);
        if (pge_collision_rectangle_rectangle(&bullet_rect, &invader_rect)) {
          invader_destroy(invaders[j]);

          /* remove invader from array by shifting the rest of the array down. 
             This makes random invader selection easier */
          for (int k = j; k < num_invaders_left-1; k++) {
            invaders[k] = invaders[k+1];
          }
          invaders[num_invaders_left-1] = NULL;
          num_invaders_left--;

          bullet_destroy(shooter_bullets[i]);
          shooter_bullets[i] = NULL;

          if (!gameover) {
            score++;
          }

          break;
        }
      }

      /* shooter bullet and invader bullet collision check */
      for (int j = 0; j < NUM_INVADER_BULLETS; j++) {
        if (invader_bullets[j] && shooter_bullets[i]) {
          GRect invader_bullet_rect = invader_bullets[j]->rect;

          if (pge_collision_rectangle_rectangle(&bullet_rect, &invader_bullet_rect)) {
            bullet_destroy(invader_bullets[j]);
            invader_bullets[j] = NULL;

            bullet_destroy(shooter_bullets[i]);
            shooter_bullets[i] = NULL;

            if (!gameover) {
              score++;
            }

            break;
          }
        }
      }

      /* shooter bullet and powerup invader collision check */
      if (powerup_invader) {
        GRect powerup_invader_rect = pge_sprite_get_bounds(powerup_invader->sprite);
        if (pge_collision_rectangle_rectangle(&bullet_rect, &powerup_invader_rect)) {
          invader_destroy(powerup_invader);
          powerup_invader = NULL;
          powerup = true;
          
          /* disables powerup after a certain amount of time */
          powerup_timer = app_timer_register(POWERUP_TIME, powerup_over_callback, NULL);
        }
      }
    }
  }
}

static void invader_bullet_logic() {
  GRect shooter_rect = pge_sprite_get_bounds(shooter->sprite);
    
  for (int i = 0; i < NUM_INVADER_BULLETS; i++) {
    if (invader_bullets[i]) {
      bullet_move(invader_bullets[i], 0, level/2 + 2);

      /* invader bullet and shooter collision check */
      GRect invader_bullet_rect = invader_bullets[i]->rect;
      if (pge_collision_rectangle_rectangle(&shooter_rect, &invader_bullet_rect)) {
        gameover = true;
      }
  
      /* destroy invader bullets that go off the screen */
      if (invader_bullets[i]->rect.origin.y > 168) {
        bullet_destroy(invader_bullets[i]);
        invader_bullets[i] = NULL;
      }
    }
  }
}

static void powerup_invader_logic() {
  if (powerup_invader){
    int invader_x_position = pge_sprite_get_position(powerup_invader->sprite).x;

    /* if invader hits the side, change directions */
    if (invader_x_position + INVADER_WIDTH >= screen_size.w) {
      powerup_invader_direction = LEFT;
    } else if (invader_x_position <= 0) {
      powerup_invader_direction = RIGHT;
    }
    
    invader_move(powerup_invader, powerup_invader_direction, false, (level + 1)/2);
  }
}

static void start_timers() {
  shooter_fire_bullet();
  invader_fire_bullet();
  invader_move_timer_callback();
}

static void cancel_timers() {
  if (shooter_timer) {
    app_timer_cancel(shooter_timer);
    app_timer_cancel(invader_bullet_timer);
    app_timer_cancel(invader_move_timer);

    /* used to check later so we don't cancel timers at every loop */
    shooter_timer = NULL;
  }
}

static void server_update_highscore(uint8_t highscore) {
  APP_LOG(APP_LOG_LEVEL_INFO, "updating score");
  DictionaryIterator *iterator;
  app_message_outbox_begin(&iterator);
  dict_write_uint8(iterator, KEY_SET_HIGHSCORE, highscore);
  dict_write_end(iterator);
  app_message_outbox_send();
}

static void game_logic() {
  if (gameover && once_token == false) {
    level = 1;

    //server_update_highscore(score); //todo remove
    
    cancel_timers();
    
    int highscore = pge_title_get_highscore();
    if (score > highscore) {
      pge_title_set_highscore(score);
    }

    uint8_t global_highscore = 0;
    if (persist_exists(KEY_HIGHSCORE)) {
      global_highscore = persist_read_int(KEY_HIGHSCORE);
    }

    if (highscore > global_highscore) {
      //APP_LOG(APP_LOG_LEVEL_INFO, "udpating highscore: %d", highscore);
      //server_update_highscore(global_highscore);
    }
    
    app_focus_service_unsubscribe();

    once_token = true;
  } else {
    shooter_bullet_logic();
    invader_bullet_logic();
    shield_logic();
    powerup_invader_logic();
    
    if (num_invaders_left == 0) {
      next_level();
    }
    
    /* moving the shooter due to acceleration */
    AccelData accel_data = {0};
    accel_service_peek(&accel_data);
    
    if (accel_data.x > MIN_ACCELERATION) {
      shooter_move(shooter, RIGHT, screen_size.w);
    } else if (accel_data.x < -MIN_ACCELERATION) {
      shooter_move(shooter, LEFT, screen_size.w);
    }
  }
}

static void draw(GContext *context) {
	graphics_context_set_compositing_mode(context, GCompOpSet);
  
  if (gameover) {
    graphics_context_set_text_color(context, COLOR_FALLBACK(GColorRed, GColorWhite));
    graphics_draw_text(context, 
                       "GAME OVER", 
                       gameover_font, 
                       (GRect) {.origin = {0,40}, .size = {screen_size.w, screen_size.h}}, 
                       GTextOverflowModeWordWrap,
                       GTextAlignmentCenter,
                       NULL);
    
    graphics_context_set_text_color(context, GColorWhite);
    snprintf(score_str, sizeof(score_str), "Your score %d", score);
    graphics_draw_text(context, 
                       score_str, 
                       final_scores_font, 
                       (GRect) {.origin = {0,70}, .size = {screen_size.w, 50}}, 
                       GTextOverflowModeWordWrap,
                       GTextAlignmentCenter,
                       NULL);
    
     snprintf(score_str, sizeof(score_str), "High score %d", previous_highscore);
    graphics_draw_text(context, 
                       score_str, 
                       final_scores_font, 
                       (GRect) {.origin = {0,90}, .size = {screen_size.w, 50}}, 
                       GTextOverflowModeWordWrap,
                       GTextAlignmentCenter,
                       NULL);
  } else {
    graphics_context_set_text_color(context, GColorWhite);
    
    for (int i = 0; i < NUM_SHIELD_BLOCKS; i++) {
      block_draw(blocks[i], context);
    }

  	for (int i = 0; i < num_invaders_left; i++) {
    	invader_draw(invaders[i], context);
  	}
    
    for (int i = 0; i < NUM_BULLETS; i++) {
      if (shooter_bullets[i]) {
        bullet_draw(shooter_bullets[i], context);
      }
    }
    
    for (int i = 0; i < NUM_INVADER_BULLETS; i++) {
      if (invader_bullets[i]) {
        bullet_draw(invader_bullets[i], context);
      }
    }

    if (powerup_invader) {
      invader_draw(powerup_invader, context);
    }

    shooter_draw(shooter, context);
    
    /* draw level text */
    snprintf(level_str, sizeof(level_str), "Level %d", level);
    graphics_draw_text(context, 
                       level_str, 
                       score_font, 
                       (GRect) {.origin = {5,2}, .size = {60,20}}, 
                       GTextOverflowModeWordWrap,
                       GTextAlignmentLeft,
                       NULL);
    
    /* draw score text */
    snprintf(score_str, sizeof(score_str), "Score %d", score);
    graphics_draw_text(context, 
                       score_str, 
                       score_font, 
                       (GRect) {.origin = {70,2}, .size = {70,20}}, 
                       GTextOverflowModeWordWrap,
                       GTextAlignmentRight,
                       NULL);
  }
}

/* button click handler */
static void click(int button_id) {
  server_update_highscore(score);
  if (gameover) {
    cancel_timers();
    window_stack_pop(false);
    server_update_highscore(score);
    
  /* shoot bullet on click, if this mode is enabled (disabled for now) */
  } else if (click_enabled && button_id == BUTTON_ID_UP) {
    GPoint shooter_position = pge_sprite_get_position(shooter->sprite);
      
    create_shooter_bullet(UP, shooter_position);
  }
}

/* ensure timers are stopped when notification appears */
void app_focus_changed(bool in_focus) {
  if (in_focus) {
    start_timers();
  } else {
    cancel_timers();
  }
}

static void game_start() {

  /* creates 2 shields */
  int index = 0;
  for (int i = 0; i < NUM_SHIELD_COLUMNS; i++) {
    for (int j = 0; j < NUM_SHIELD_ROWS; j++) {
      /* don't create the bottom middle blocks */
      if (!(i > 2 && i < 6 && j > 2)) {
        /* first shield */
        if (blocks[index]) {
          block_destroy(blocks[index]);
        }
        blocks[index] = block_create(GRect(27 + i*BLOCK_SIZE, 125 + j*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE));
        
        /* second shield */
        if (blocks[index + 1]) {
          block_destroy(blocks[index + 1]);
        }
        blocks[index+1] = block_create(GRect(90 + i*BLOCK_SIZE, 125 + j*BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE));
        index += 2;
      }
    }
  }
  
  for (int i = 0; i < num_invaders_left; i++) {
		if (invaders[i]) {
			invader_destroy(invaders[i]);
			invaders[i] = NULL;
		}
	}
    
  reset();
  
  game_window = pge_begin(game_logic, draw, click);
	window_set_background_color(game_window, GColorBlack);
	pge_set_framerate(FPS);

  screen_size = get_screen_size();
  shooter_initial_position = GPoint(screen_size.w/2 - SHOOTER_WIDTH/2, 
                                    screen_size.h - SHOOTER_HEIGHT/2 - SHOOTER_BOTTOM_PADDING);

  if (!shooter) {  
    shooter = shooter_create(shooter_initial_position);
  }
  shooter_set_position(shooter, shooter_initial_position);
  
  accel_data_service_subscribe(0, NULL);
  app_focus_service_subscribe(app_focus_changed);
    
  score = 0;
  gameover = false;
  
  calculate_invader_interval();

  cancel_timers();
  start_timers();
  
  score_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  gameover_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  final_scores_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  
  previous_highscore = pge_title_get_highscore();
}

static void title_click_handler(int button_id) {
  game_start();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message received");
  Tuple *t = dict_read_first(iterator);

  while (t != NULL) {
    APP_LOG(APP_LOG_LEVEL_INFO, "KEY: %d VLUE: %d", (int)t->key, (int)t->value->int8);
    if (t->key == KEY_JS_READY) {
      APP_LOG(APP_LOG_LEVEL_INFO, "KEY_JS_READY");
      /* get current global highscore */
      DictionaryIterator *iterator;
      app_message_outbox_begin(&iterator);
      dict_write_uint8(iterator, KEY_GET_HIGHSCORE, 1);
      dict_write_end(iterator);
      app_message_outbox_send();
    } else if (t->key == KEY_GET_HIGHSCORE) {
      /* set the received highscore on screen */
      int global_highscore = t->value->int8;
       APP_LOG(APP_LOG_LEVEL_INFO, "global highscore: %d", global_highscore);
      /* store global highscore in persistent memory */
      persist_write_int(KEY_HIGHSCORE, global_highscore);
    }

    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success");
}

void pge_init(void) {
  /* show title page */
  pge_title_show_highscore(true);
  pge_title_push("", "", "", "", COLOR_FALLBACK(GColorMagenta, GColorWhite), 
                 RESOURCE_ID_TITLE, title_click_handler);
    
  /* make backlight stay on when in game */
  light_enable(true);

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_register_outbox_failed(outbox_failed_callback);

  app_message_open(
    app_message_inbox_size_maximum(),
    app_message_outbox_size_maximum()
  );

  /* show persistent data's global highscore */
  int global_highscore = 0;
  if (persist_exists(KEY_HIGHSCORE)) {
    global_highscore = persist_read_int(KEY_HIGHSCORE);
  }
  //todo: SHOWWWWW dis
  APP_LOG(APP_LOG_LEVEL_INFO, "init global highscore: %d", global_highscore);
}

void pge_deinit(void) {
  accel_data_service_unsubscribe();
  app_focus_service_unsubscribe();
  
  cancel_timers();
  
  pge_title_set_highscore(score);
  light_enable(false);
  
  /* free all allocated memory */
  for (int i = 0; i < NUM_BULLETS; i++) { 
    if (shooter_bullets[i]) {
      bullet_destroy(shooter_bullets[i]);
    }
  }
  
  for (int i = 0; i < NUM_INVADER_BULLETS; i++) {
    if (invader_bullets[i]) {
      bullet_destroy(invader_bullets[i]);
    }
  }
  
  for (int i = 0; i < num_invaders_left; i++) {
    invader_destroy(invaders[i]);
  }
  
  for (int i = 0; i < NUM_SHIELD_BLOCKS; i++) {
    block_destroy(blocks[i]);
  }
  
  shooter_destroy(shooter);

  if (powerup_invader) {
    invader_destroy(powerup_invader);
  }
}