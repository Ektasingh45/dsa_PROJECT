/*
 * railway.h
 * ---------
 * Central RAILWAY SYSTEM controller.
 *
 * Owns the global state (BST root, undo stack, PNR counter) and
 * implements the Admin and Passenger menus. It is the only module that
 * knows about all seven data structures at once; each data-structure
 * module only knows about itself.
 */
#ifndef RAILWAY_H
#define RAILWAY_H

#include "train_bst.h"
#include "undo_stack.h"

/* Preloads trains 12951, 12952 and 12431 with routes, coaches and seats. */
void loadInitialData(void);

/* Main menu loop (Admin Login / Passenger Login / Exit). */
void runRailwaySystem(void);

/* Frees every train, every list/queue node and the undo stack. Safe to call twice. */
void shutdownRailwaySystem(void);

#endif
