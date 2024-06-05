// Copyright (C) 2023 zhuvoy
// 
// This file is part of ClassicImageViewer.
// 
// ClassicImageViewer is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
// 
// ClassicImageViewer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with ClassicImageViewer.
// If not, see <https://www.gnu.org/licenses/>.


#ifndef ACTIONS_H
#define ACTIONS_H

#define ACTDISABLE_UNLOADED	(1 << 0)
#define ACTDISABLE_FULLSCREEN	(1 << 1)
#define ACTDISABLE_CLIPBOARD	(1 << 2)

enum Action
{
	ACT_INVALID = 0,
	ACT_OPEN,
	ACT_REOPEN,
	ACT_RECENT_FILE_0,
	ACT_RECENT_FILE_1,
	ACT_RECENT_FILE_2,
	ACT_RECENT_FILE_3,
	ACT_RECENT_FILE_4,
	ACT_RECENT_FILE_5,
	ACT_RECENT_FILE_6,
	ACT_RECENT_FILE_7,
	ACT_CLEAR_RECENT_FILES,
	ACT_THUMBNAILS,
	ACT_SLIDESHOW,
	ACT_BATCH,
	ACT_SELECT_TARGET_DIR,
	ACT_FILE_RENAME,
	ACT_FILE_MOVE,
	ACT_FILE_COPY,
	ACT_FILE_DELETE,
	ACT_SAVE,
	ACT_SAVE_AS,
	ACT_PRINT,
	ACT_EXIT,
	
	ACT_UNDO,
	ACT_CUT_SELECTION,
	ACT_CROP_SELECTION,
	ACT_COPY,
	ACT_PASTE,
	ACT_PASTE_TO_SIDE,
	ACT_UNLOAD,
	ACT_CLEAR_CLIPBOARD,
	
	ACT_INFO,
ACTGROUP_FILTER_BEGIN,
	ACT_ROTATE_L,
	ACT_ROTATE_R,
	ACT_FLIP_V,
	ACT_FLIP_H,
	ACT_GRAYSCALE,
	ACT_NEGATIVE,
	ACT_AUTO_COLOR,
	ACT_SHARPEN,
ACTGROUP_FILTER_END,
	ACT_ROTATE_C,
	ACT_RESIZE,
	ACT_ADD_BORDER,
	ACT_PAD_TO_SIZE,
	ACT_COLOR_ADJUST,
	ACT_EFFECTS,
	
	ACT_SETTINGS,
	ACT_MINIMIZE,
	
	ACT_TOGGLE_STATUSBAR,
	ACT_TOGGLE_TOOLBAR,
	ACT_TOGGLE_MENUBAR,
	ACT_TOGGLE_FULLSCREEN,
ACTGROUP_NAVIGATION_BEGIN,
	ACT_NEXT_FILE,
	ACT_PREV_FILE,
	ACT_FIRST_FILE,
	ACT_LAST_FILE,
ACTGROUP_NAVIGATION_END,
	ACT_ZOOM_IN,
	ACT_ZOOM_OUT,
	ACT_ZOOM_1,		// 1:1
	ACT_ZOOM_5,		// percents
	ACT_ZOOM_10,
	ACT_ZOOM_15,
	ACT_ZOOM_20,
	ACT_ZOOM_25,
	ACT_ZOOM_33,
	ACT_ZOOM_50,
	ACT_ZOOM_66,
	ACT_ZOOM_100,
	ACT_ZOOM_125,
	ACT_ZOOM_150,
	ACT_ZOOM_175,
	ACT_ZOOM_200,
	ACT_ZOOM_300,
	ACT_ZOOM_400,
	
	ACT_LICENSE,
	ACT_SHORTCUTS,
	ACT_ABOUT,
	
	ACT_LAST_ENUM
};


#endif //ACTIONS_H
