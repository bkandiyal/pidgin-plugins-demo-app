#!/bin/bash
gcc pidgin-plugins.c -o pidgin-plugins $(pkg-config --cflags --libs gtk+-3.0 gmodule-2.0 webkitgtk-3.0)
