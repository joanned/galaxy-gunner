/**
 * Pebble Game Engine - A simple game engine for Pebble
 *
 * This version includes modifications by Matthew Hungerford
 *
 * Author: 
 * Chris Lewis
 *
 * Source and example:
 * https://github.com/C-D-Lewis/pge
 *
 * Features:
 * - 30 frames per second
 * - Automatic game loop using PGELogicHandler and PGERenderHandler
 * - Customizable framerate
 * - Easy to use button events, as well as query functions
 *
 * Abstracted Pebble APIs (DO NOT REIMPLEMENT!):
 * - Clicks using a PGEClickHandler
 * - Window is managed for you
 * - main function is managed for you. Implement pge_init() and pge_deinit to use.
 */

#pragma once

#include <pebble.h>

/********************************** Engine ***********************************/

// Function for user to place their per-frame game logic
typedef void (PGELogicHandler)();

// Function for user to use the GContext to draw their game items
typedef void (PGERenderHandler)(GContext *ctx);

// Function for user to implement button clicks
typedef void (PGEClickHandler)(int button_id);

// Implement app setup here
void pge_init();

// Implement app teardown here
void pge_deinit();

/**
 * Create a full-screen Window and Layer to use as a rendering canvas
 *
 * Note: The Click handler can be NULL to not implement
 */
Window* pge_begin(PGELogicHandler *logic_handler, PGERenderHandler *render_handler, PGEClickHandler *click_handler);

/**
 * Query the current state of a button
 */
bool pge_get_button_state(ButtonId button);

/**
 * Set the desired framerate (0 - 30)
 */
void pge_set_framerate(int new_rate);

/**
 * Set the fullscreen background image
 */
void pge_set_background(int bg_resource_id);

GSize get_screen_size();
