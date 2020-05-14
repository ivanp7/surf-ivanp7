/* modifier 0 means no modifier */
static int surfuseragent    = 1;  /* Append Surf version to default WebKit user agent */
static char *fulluseragent  = ""; /* Or override the whole user agent string */
static char *scriptfile     = "~/.config/surf/script.js";
static char *styledir       = "~/.config/surf/styles/";
static char *cachedir       = "~/.config/surf/cache/";
static char *cookiefile     = "~/.config/surf/cookies.txt";
static char *historyfile    = "~/.config/surf/history.txt";
static char *wmclass        = "Surf";
static int scroll_multiplier = 5;

/* Webkit default features */
static Parameter defconfig[ParameterLast] = {
    SETB(AcceleratedCanvas,  1),
    SETB(CaretBrowsing,      0),
    SETV(CookiePolicies,     "@Aa"),
    SETB(DiskCache,          1),
    SETB(DNSPrefetch,        0),
    SETI(FontSize,           14),
    SETB(FrameFlattening,    0),
    SETB(Geolocation,        0),
    SETB(HideBackground,     0),
    SETB(Inspector,          1),
    SETB(JavaScript,         1),
    SETB(KioskMode,          0),
    SETB(LoadImages,         1),
    SETB(MediaManualPlay,    0),
    SETB(Plugins,            1),
    SETV(PreferredLanguages, ((char *[]){ NULL })),
    SETB(RunInFullscreen,    0),
    SETB(ScrollBars,         1),
    SETB(ShowIndicators,     1),
    SETB(SiteQuirks,         1),
    SETB(SpellChecking,      0),
    SETV(SpellLanguages,     ((char *[]){ "en_US", NULL })),
    SETB(StrictSSL,          0),
    SETB(Style,              1),
    SETF(ZoomLevel,          1.0),
};

static UriParameters uriparams[] = {
    { "(://|\\.)suckless\\.org(/|$)", {
      FSETB(JavaScript, 0),
      FSETB(Plugins,    0),
    }, },
};

static WebKitFindOptions findopts = WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE |
                                    WEBKIT_FIND_OPTIONS_WRAP_AROUND;

/* styles */
/*
 * The iteration will stop at the first match, beginning at the beginning of
 * the list.
 */
static SiteStyle styles[] = {
    /* regexp               file in $styledir */
    { ".*",                 "default.css" },
};

#define SETPROP(p, q) { \
        .v = (const char *[]){ "/bin/sh", "-c", \
             "prop=\"`xprop -id $2 $0 " \
             "| sed \"s/^$0(STRING) = \\(\\\\\"\\?\\)\\(.*\\)\\1$/\\2/\" " \
             "| xargs -0 printf %b | dmenu -fn \"$DEFAULT_FONT\" -w $2`\" &&" \
             "xprop -id $2 -f $1 8s -set $1 \"$prop\"", \
             p, q, winid, NULL \
        } \
}

/* DOWNLOAD(URI, referer) */
#define DOWNLOAD(d, r) { \
        .v = (const char *[]){ "/bin/sh", "-c", \
             "$TERMINAL -e /bin/sh -c \"curl -g -L -J -O --user-agent '$1'" \
             " --referer '$2' -b $3 -c $3 '$0';" \
             " sleep 5;\"", \
             d, useragent, r, cookiefile, NULL \
        } \
}

/* PLUMB(URI) */
/* This called when some URI which does not begin with "about:",
 * "http://" or "https://" should be opened.
 */
#define PLUMB(u) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
             "xdg-open \"$0\"", u, NULL \
        } \
}

/* VIDEOPLAY(URI) */
#define VIDEOPLAY(u) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
             "$PLAYER --really-quiet \"$0\"", u, NULL \
        } \
}

#define SCRIPTS_DIR "~/.scripts/xdf/surf/"

/* bookmarks */
static char *bookmarkadd_curwin [] = { "/bin/sh", "-c",
    SCRIPTS_DIR "surf_add_bookmark.sh \
        \"$(xprop -id $0 _SURF_URI | cut -d '\"' -f2 | \
                sed -E 's@.*https?://(www\\.)?@@')        $(xprop -id $0 WM_NAME | \
                cut -d '\"' -f2 | sed 's/[^|]*| *//')\" $0",
    winid, NULL
};
static char *bookmarkselect_curwin [] = { "/bin/sh", "-c",
    SCRIPTS_DIR "surf_bookmarks_dmenu.sh $0 | xargs -r xprop -id $0 -f _SURF_GO 8s -set _SURF_GO",
    winid, NULL
};
/* history */
static char *historyselect_curwin [] = { "/bin/sh", "-c",
    SCRIPTS_DIR "surf_history_dmenu.sh $0 | xargs -r xprop -id $0 -f _SURF_GO 8s -set _SURF_GO",
    winid, NULL
};
/* external pipe */
static char *linkselect_curwin [] = { "/bin/sh", "-c",
    SCRIPTS_DIR "surf_linkselect.sh $0 'Link' | xargs -r xprop -id $0 -f _SURF_GO 8s -set _SURF_GO",
    winid, NULL
};
static char *linkselect_newwin [] = { "/bin/sh", "-c",
    SCRIPTS_DIR "surf_linkselect.sh $0 'Link (new window)' | " SCRIPTS_DIR "surf_new_window.sh $(xdotool getactivewindow)",
    winid, NULL
};
static char *editscreen[] = { "/bin/sh", "-c", SCRIPTS_DIR "edit_screen.sh", NULL };
static char *externalpipe_sigusr1[] = {"/bin/sh", "-c", SCRIPTS_DIR "externalpipe_buffer.sh surf_strings_read"};

/* hotkeys */

#define MODKEY GDK_CONTROL_MASK

/*
 * If you use anything else but MODKEY and GDK_SHIFT_MASK, don't forget to
 * edit the CLEANMASK() macro.
 */
static Key keys[] = {
    /* modifier              keyval          function    arg */
    { MODKEY,                36 /*GDK_KEY_Return*/, spawn,      SETPROP("_SURF_URI", "_SURF_GO") },
    { MODKEY,                39 /*GDK_KEY_s*/,      spawn,      { .v = historyselect_curwin } },
    { MODKEY,                42 /*GDK_KEY_g*/,      spawn,      { .v = bookmarkselect_curwin } },
    { MODKEY,                58 /*GDK_KEY_m*/,      spawn,      { .v = bookmarkadd_curwin } },

    { MODKEY,                40 /*GDK_KEY_d*/,     externalpipe, { .v = linkselect_curwin } },
    { GDK_SHIFT_MASK|MODKEY, 40 /*GDK_KEY_d*/,     externalpipe, { .v = linkselect_newwin } },
    { MODKEY,                30 /*GDK_KEY_u*/,     externalpipe, { .v = editscreen        } },

    { MODKEY,                25 /*GDK_KEY_w*/,      playexternal, { 0 } },

    { 0,                     9 /*GDK_KEY_Escape*/, stop,       { 0 } },
    { MODKEY,                54 /*GDK_KEY_c*/,      stop,       { 0 } },

    { MODKEY|GDK_SHIFT_MASK, 27 /*GDK_KEY_r*/,      reload,     { .b = 1 } },
    { MODKEY,                27 /*GDK_KEY_r*/,      reload,     { .b = 0 } },

    { MODKEY,                31 /*GDK_KEY_i*/,      navigate,   { .i = +1 } },
    { MODKEY,                32 /*GDK_KEY_o*/,      navigate,   { .i = -1 } },

    /* Currently we have to use scrolling steps that WebKit2GTK+ gives us
     * d: step down, u: step up, r: step right, l:step left
     * D: page down, U: page up */
    { MODKEY,                43 /*GDK_KEY_h*/,      scroll,     { .i = 'l' } },
    { MODKEY,                44 /*GDK_KEY_j*/,      scroll,     { .i = 'd' } },
    { MODKEY,                45 /*GDK_KEY_k*/,      scroll,     { .i = 'u' } },
    { MODKEY,                46 /*GDK_KEY_l*/,      scroll,     { .i = 'r' } },
    { MODKEY,                41 /*GDK_KEY_f*/,      scroll,     { .i = 'D' } },
    { MODKEY,                56 /*GDK_KEY_b*/,      scroll,     { .i = 'U' } },

    { MODKEY,                20 /*GDK_KEY_minus*/,  zoom,       { .i = -1 } },
    { MODKEY,                51 /*GDK_KEY_backslash*/,  zoom,       { .i = 0  } },
    { MODKEY,                21 /*GDK_KEY_equal*/,  zoom,       { .i = +1 } },

    { MODKEY,                33 /*GDK_KEY_p*/,      clipboard,  { .b = 1 } },
    { MODKEY,                29 /*GDK_KEY_y*/,      clipboard,  { .b = 0 } },

    { MODKEY,                61 /*GDK_KEY_slash*/,  spawn,      SETPROP("_SURF_FIND", "_SURF_FIND") },
    { MODKEY,                57 /*GDK_KEY_n*/,      find,       { .i = +1 } },
    { MODKEY|GDK_SHIFT_MASK, 57 /*GDK_KEY_n*/,      find,       { .i = -1 } },

    { MODKEY|GDK_SHIFT_MASK, 33 /*GDK_KEY_p*/,      print,      { 0 } },

    { MODKEY|GDK_SHIFT_MASK, 38 /*GDK_KEY_a*/,      togglecookiepolicy, { 0 } },
    { 0,                     95 /*GDK_KEY_F11*/,    togglefullscreen, { 0 } },
    { MODKEY|GDK_SHIFT_MASK, 30 /*GDK_KEY_u*/,      toggleinspector, { 0 } },

    { MODKEY|GDK_SHIFT_MASK, 54 /*GDK_KEY_c*/,      toggle,     { .i = CaretBrowsing } },
    { MODKEY|GDK_SHIFT_MASK, 41 /*GDK_KEY_f*/,      toggle,     { .i = FrameFlattening } },
    { MODKEY|GDK_SHIFT_MASK, 42 /*GDK_KEY_g*/,      toggle,     { .i = Geolocation } },
    { MODKEY|GDK_SHIFT_MASK, 39 /*GDK_KEY_s*/,      toggle,     { .i = JavaScript } },
    { MODKEY|GDK_SHIFT_MASK, 31 /*GDK_KEY_i*/,      toggle,     { .i = LoadImages } },
    { MODKEY|GDK_SHIFT_MASK, 55 /*GDK_KEY_v*/,      toggle,     { .i = Plugins } },
    { MODKEY|GDK_SHIFT_MASK, 56 /*GDK_KEY_b*/,      toggle,     { .i = ScrollBars } },
    { MODKEY|GDK_SHIFT_MASK, 58 /*GDK_KEY_m*/,      toggle,     { .i = Style } },
};

/* button definitions */
/* target can be OnDoc, OnLink, OnImg, OnMedia, OnEdit, OnBar, OnSel, OnAny */
static Button buttons[] = {
    /* target       event mask      button  function        argument        stop event */
    { OnLink,       0,              2,      clicknewwindow, { .b = 0 },     1 },
    { OnLink,       MODKEY,         2,      clicknewwindow, { .b = 1 },     1 },
    { OnLink,       MODKEY,         1,      clicknewwindow, { .b = 1 },     1 },
    { OnAny,        0,              8,      clicknavigate,  { .i = -1 },    1 },
    { OnAny,        0,              9,      clicknavigate,  { .i = +1 },    1 },
    { OnMedia,      MODKEY,         1,      clickexternplayer, { 0 },       1 },
};

static char *searchengine = "https://duckduckgo.com/?q=";

