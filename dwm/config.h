/* See LICENSE file for copyright and license details. */

#include "movestack.c"
/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int minwsz    = 20;       /* Minimal heigt of a client for smfact */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const int user_bh            = 3;        /* 2 is the default spacing around the bar's font */
static const char *fonts[]          = {"unscii:style=16-full:pixelsize=20:antialias=true:autohint=true"};
static const char dmenufont[]       = "unscii:style=16-full:pixelsize=20:antialias=true:autohint=true";
static const char col_black[]       = "#000000";
static const char col_white[]       = "#ffffff";
static const char col_orange[]      = "#ffa500";
static const char col_gray[]      = "#bbbbbb";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
    /*               bat text   bar          inactive client*/
	[SchemeNorm] = { col_white, col_black,   col_gray },
    /*               tag number active tag   active client*/                            
	[SchemeSel]  = { col_white, col_orange,  col_orange },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ NULL,       NULL,       NULL,       0,            False,       -1 },
};

/* layout(s) */
static const float mfact     = 0.5; /* factor of master area size [0.05..0.95] */
static const float smfact     = 0.00; /* factor of tiled clients [0.00..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 0; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[T]",      tile },    /* first entry is default */
	{ "[F]",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
// #define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char* dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_black, "-nf", col_gray, "-sb", col_orange, "-sf", col_white, NULL };
static const char *termcmd[]  = { "st", NULL };

static const char* flameshot[] = { "/usr/bin/flameshot", "gui", NULL};
static const char* printScreen[] = { "/usr/bin/flameshot", "full", "-c", NULL};
static const char* volumeUp[] = { "/usr/bin/pactl", "set-sink-volume", "0", "+1%", NULL};
static const char* volumeDown[] = { "/usr/bin/pactl", "set-sink-volume", "0", "-1%", NULL};
static const char* muteSpeakers[] = { "/usr/bin/pactl", "set-sink-mute", "0", "toggle", NULL};
static const char* muteMicro[] = { "/usr/bin/pactl", "set-source-mute", "1", "toggle", NULL};
static const char* brightUp[] = { "/usr/bin/brightnessctl", "set", "1%+", NULL};
static const char* brightDown[] = { "/usr/bin/brightnessctl", "set", "1%-", NULL};
static const char* lockScreen[] = { "/usr/local/bin/slock", NULL};
// static const char* turnOffScreen[] = {"/usr/bin/xset", "dpms", "force", "off", NULL}; 

static const Key keys[] = {
	/* modifier                     key        function        argument */
    { MODKEY,                       XK_w,         togglecanfocusfloating,   {0} },      /* enable / disable focus on floating winfows, usefull for Picture in Picture mode */
	{ MODKEY,                       XK_p,         spawn,          {.v = dmenucmd } },   /* launche dmenu */
	{ MODKEY|ShiftMask,             XK_Return,    spawn,          {.v = termcmd } },    /* launch st */

    /*                          Command combinations                                */
	{ MODKEY,			            XK_s,	      spawn,	   {.v = flameshot } },
	{ MODKEY,			            0xff61,       spawn,	   {.v = printScreen } },
	{ 0,				            0x1008ff13,   spawn,	   {.v = volumeUp } },
	{ 0,				            0x1008ff11,   spawn, 	   {.v = volumeDown } },
	{ 0,				            0x1008ff12,   spawn,	   {.v = muteSpeakers } }, 
	{ 0,				            0x1008ff02,   spawn,	   {.v = brightUp } },
	{ 0,				            0x1008ff03,   spawn,	   {.v = brightDown } },
	// { 0, 				            0x1008ff59,   spawn,	   {.v = turnOffScreen } },
    { 0,                            0x1008ffb2,   spawn,       {.v = muteMicro } },
    { MODKEY,			            XK_l,	      spawn,	   {.v = lockScreen } },

	{ MODKEY,                       XK_b,      togglebar,      {0} },                   /* show/hide bar */

	{ MODKEY,                       0xff53,    focusstack,     {.i = +1 } },            /* select next or prev client */
	{ MODKEY,                       0xff51,    focusstack,     {.i = -1 } },
    { MODKEY,                       0xff54,    focusstack,     {.i = +1 } },
	{ MODKEY,                       0xff52,    focusstack,     {.i = -1 } },

	{ MODKEY|ShiftMask,             XK_i,      incnmaster,     {.i = +1 } },            /* increase / decrease number of windows in master area */
	{ MODKEY|ShiftMask,             XK_d,      incnmaster,     {.i = -1 } },

	{ MODKEY|ControlMask,           XK_Left,   setmfact,       {.f = -0.01} },          /* resize master window */
	{ MODKEY|ControlMask,           XK_Right,  setmfact,       {.f = +0.01} },

	{ MODKEY|ControlMask,           0xff52,    setsmfact,      {.f = +0.01} },          /* resize client in stack */
	{ MODKEY|ControlMask,           0xff54,    setsmfact,      {.f = -0.01} },

    { MODKEY|ShiftMask,             0xff54,    movestack,      {.i = +1 } },            /* move Up in stack */
	{ MODKEY|ShiftMask,             0xff52,    movestack,      {.i = -1 } },            /* move Down in stack */

	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },

	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },

	// { MODKEY,                       XK_space,  setlayout,      {0} }, /* switch current / previous layout */
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} }, /* restore client to current layout */

	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },

	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)

	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

