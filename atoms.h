/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#ifndef _ATOMS_H_
#define _ATOMS_H_

typedef enum _AtomType {
    ATOM_MEEGO_STACKING_LAYER,
    ATOM_MEEGOTOUCH_SKIP_ANIMATIONS,
    ATOM_NET_WM_STATE_SKIP_TASKBAR,
    ATOM_NET_WM_STATE,
    ATOM_MEEGO_INHIBIT_SCREENSAVER,
    ATOM_COUNT
} AtomType;

void initAtoms ();
Atom getAtom (AtomType type);

#endif
