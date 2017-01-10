"use strict";

/* Edit these values to suit your taste */

var audiofiles = [
    "AirHorn.mp3", // tatami 1
    "CarAlarm.mp3", // tatami 2 etc.
    "Doorbell.mp3",
    "IndutrialAlarm.mp3",
    "IntruderAlarm.mp3",
    "RedAlert.mp3",
    "TrainHorn.mp3",
    "BikeHorn.mp3",
    "TwoToneDoorbell.mp3",
    "AirHorn.mp3"
];

var use_2017_rules = true;
var selected_name_layout = 7;
var show_shido_cards = true;
var shido_cards = [
    "shido-none.png",
    "shido-yellow1.png",
    "shido-yellow2.png",
    "shido-red.png",
];

var no_big_text = true;

/* End of user defined values */

var menuspace = 40;
var webSocket;
var svgtemplate;
var hide_zero_osaekomi_points = 0;

var NUM_TATAMIS = 10;
var COMM_VERSION = 3;

var APPLICATION_TYPE_UNKNOWN = 0;
var APPLICATION_TYPE_SHIAI   = 1;
var APPLICATION_TYPE_TIMER   = 2;
var APPLICATION_TYPE_INFO    = 3;
var APPLICATION_TYPE_WEIGHT  = 4;
var APPLICATION_TYPE_JUDOGI  = 5;
var APPLICATION_TYPE_PROXY   = 6;
var NUM_APPLICATION_TYPES    = 7;

var MSG_NEXT_MATCH = 1;
var MSG_RESULT     = 2;
var MSG_ACK        = 3;
var MSG_SET_COMMENT = 4;
var MSG_SET_POINTS = 5;
var MSG_HELLO      = 6;
var MSG_DUMMY      = 7;
var MSG_MATCH_INFO = 8;
var MSG_NAME_INFO  = 9;
var MSG_NAME_REQ   = 10;
var MSG_ALL_REQ    = 11;
var MSG_CANCEL_REST_TIME = 12;
var MSG_UPDATE_LABEL = 13;
var MSG_EDIT_COMPETITOR = 14;
var MSG_SCALE      = 15;
var MSG_11_MATCH_INFO = 16;
var MSG_EVENT      = 17;
var MSG_WEB        = 18;
var MSG_LANG       = 19;
var MSG_LOOKUP_COMP = 20;
var NUM_MESSAGE    = 21;

var EDIT_OP_GET        = 0;
var EDIT_OP_SET        = 1;
var EDIT_OP_SET_WEIGHT = 2;
var EDIT_OP_SET_FLAGS  = 4;
var EDIT_OP_GET_BY_ID  = 8;
var EDIT_OP_SET_JUDOGI = 16;
var EDIT_OP_CONFIRM    = 32;

var DELETED       = 0x01;
var HANSOKUMAKE   = 0x02;
var JUDOGI_OK     = 0x20;
var JUDOGI_NOK    = 0x40;
var GENDER_MALE   = 0x80;
var GENDER_FEMALE = 0x100;

var MATCH_FLAG_BLUE_DELAYED  = 0x0001;
var MATCH_FLAG_WHITE_DELAYED = 0x0002;
var MATCH_FLAG_REST_TIME     = 0x0004;
var MATCH_FLAG_BLUE_REST     = 0x0008;
var MATCH_FLAG_WHITE_REST    = 0x0010;
var MATCH_FLAG_SEMIFINAL_A   = 0x0020;
var MATCH_FLAG_SEMIFINAL_B   = 0x0040;
var MATCH_FLAG_BRONZE_A      = 0x0080;
var MATCH_FLAG_BRONZE_B      = 0x0100;
var MATCH_FLAG_GOLD          = 0x0200;
var MATCH_FLAG_SILVER        = 0x0400;
var MATCH_FLAG_JUDOGI1_OK    = 0x0800;
var MATCH_FLAG_JUDOGI1_NOK   = 0x1000;
var MATCH_FLAG_JUDOGI2_OK    = 0x2000;
var MATCH_FLAG_JUDOGI2_NOK   = 0x4000;
var MATCH_FLAG_JUDOGI_MASK   = (MATCH_FLAG_JUDOGI1_OK | MATCH_FLAG_JUDOGI1_NOK |
                                MATCH_FLAG_JUDOGI2_OK | MATCH_FLAG_JUDOGI2_NOK);
var MATCH_FLAG_REPECHAGE     = 0x8000;
var MATCH_FLAG_TEAM_EVENT    = 0x10000;

var ROUND_MASK           = 0x00ff;
var ROUND_TYPE_MASK      = 0x0f00;
var ROUND_UP_DOWN_MASK   = 0xf000;
var ROUND_UPPER          = 0x1000;
var ROUND_LOWER          = 0x2000;
var ROUND_ROBIN          = (1<<8);
var ROUND_REPECHAGE      = (2<<8);
var ROUND_REPECHAGE_1    = (ROUND_REPECHAGE | ROUND_UPPER);
var ROUND_REPECHAGE_2    = (ROUND_REPECHAGE | ROUND_LOWER);
var ROUND_SEMIFINAL      = (3<<8);
var ROUND_SEMIFINAL_1    = (ROUND_SEMIFINAL | ROUND_UPPER);
var ROUND_SEMIFINAL_2    = (ROUND_SEMIFINAL | ROUND_LOWER);
var ROUND_BRONZE         = (4<<8);
var ROUND_BRONZE_1       = (ROUND_BRONZE | ROUND_UPPER);
var ROUND_BRONZE_2       = (ROUND_BRONZE | ROUND_LOWER);
var ROUND_SILVER         = (5<<8);
var ROUND_FINAL          = (6<<8);

var INC = false, DEC = true;

var OSAEKOMI_DSP_NO     = 0;
var OSAEKOMI_DSP_YES    = 1;
var OSAEKOMI_DSP_YES2   = 2;
var OSAEKOMI_DSP_BLUE   = 3;
var OSAEKOMI_DSP_WHITE  = 4;
var OSAEKOMI_DSP_UNKNOWN= 5;

var GDK_0 = 48, GDK_1 = 49, GDK_2 = 50, GDK_3 = 51, GDK_4 = 52, GDK_5 = 53,
    GDK_6 = 54, GDK_7 = 55, GDK_8 = 56, GDK_9 = 57;
var GDK_a = 65, GDK_b = 66, GDK_c = 67, GDK_d = 68, GDK_e = 69, GDK_f = 70,
    GDK_g = 71, GDK_h = 72, GDK_i = 73, GDK_j = 74, GDK_k = 75, GDK_l = 76,
    GDK_m = 77, GDK_n = 78, GDK_o = 79, GDK_p = 80, GDK_q = 81, GDK_r = 82,
    GDK_s = 83, GDK_t = 84, GDK_u = 85, GDK_v = 86, GDK_w = 87, GDK_x = 88,
    GDK_y = 89, GDK_z = 90;
var GDK_F1 = 112, GDK_F2 = 113, GDK_F3 = 114, GDK_F4 = 115, GDK_F5 = 116,
    GDK_F6 = 117, GDK_F7 = 118, GDK_F8 = 119, GDK_F9 = 120, GDK_F10 = 121;
var GDK_space = 32, GDK_Return = 13, GDK_KP_Enter = 13, GDK_Up = 38, GDK_Down = 40;

var CLEAR_SELECTION   = 0;
var HANTEI_BLUE       = 1;
var HANTEI_WHITE      = 2;
var HANSOKUMAKE_BLUE  = 3;
var HANSOKUMAKE_WHITE = 4;
var HIKIWAKE          = 5;

function ROUND_IS_FRENCH(_n) {
    return ((_n & ROUND_TYPE_MASK) == 0 ||
	    (_n & ROUND_TYPE_MASK) == ROUND_REPECHAGE);
}
function ROUND_TYPE(_n) { return (_n & ROUND_TYPE_MASK); }
function ROUND_NUMBER(_n) { return (_n & ROUND_MASK); }
function ROUND_TYPE_NUMBER(_n) { return (ROUND_TYPE(_n) | ROUND_NUMBER(_n)); }

var commver = 3;
var apptype = APPLICATION_TYPE_TIMER;
var myaddr = 104;
var show_competitor_names = true;

var screenwidth = window.innerWidth
    || document.documentElement.clientWidth
    || document.body.clientWidth;

var screenheight = window.innerHeight
    || document.documentElement.clientHeight
    || document.body.clientHeight;

var fontsize = 16;//screenheight/32;

var canvas_width = 100;
var canvas_height = 100;

var labels = [];

var SMALL_H = 0.032;
var BIG_H = ((1.0 - 4*SMALL_H)/3.0);
var BIG_H2 = ((1.0 - 4*SMALL_H)/2.0);
var BIG_START = (4.0*SMALL_H);
var BIG_W = 0.125;
var BIG_W2 = 0.25;
var TXTW = 0.12;

var blue_name_1 = label_new("blue_name_1", "",  TXTW,     0.0,     0.5-TXTW, SMALL_H);
var white_name_1 = label_new("white_name_1", "", TXTW,     SMALL_H, 0.5-TXTW, SMALL_H);
var blue_name_2 = label_new("blue_name_2", "",  0.5+TXTW, 0.0,     0.5-TXTW, SMALL_H);
var white_name_2 = label_new("white_name_2", "", 0.5+TXTW, SMALL_H, 0.5-TXTW, SMALL_H);
var blue_club = label_new("blue_club", "", 0,0,0,0);
var white_club = label_new("white_club", "", 0,0,0,0);

var match1 = label_new("match1", "Match:", 0.0, 0.0, TXTW, SMALL_H);
var match2 = label_new("match2", "Next:",  0.5, 0.0, TXTW, SMALL_H);

var wazaari = label_new("wazaari", "W", 0.0,       BIG_START, BIG_W, BIG_H);
var yuko = label_new("yuko", "Y",    1.0*BIG_W, BIG_START, BIG_W, BIG_H);
var koka = label_new("koka", "K",    2.0*BIG_W, BIG_START, BIG_W, BIG_H);
var shido = label_new("shido", "S",   3.0*BIG_W, BIG_START, BIG_W, BIG_H);
var padding = label_new("padding", "",   4.0*BIG_W, BIG_START, 3.0*BIG_W, BIG_H);
var sonomama = label_new("sonomama", "SONO", 7.0*BIG_W, BIG_START, BIG_W, BIG_H);

var bw = label_new("bw", "0", 0.0,       BIG_START+BIG_H, BIG_W, BIG_H);
var by = label_new("by", "0", 1.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
var bk = label_new("bk", "0", 2.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
var bs = label_new("bs", "0", 3.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
var ww = label_new("ww", "0", 0.0,       BIG_START+2*BIG_H, BIG_W, BIG_H);
var wy = label_new("wy", "0", 1.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);
var wk = label_new("wk", "0", 2.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);
var ws = label_new("ws", "0", 3.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);

var t_min = label_new("t_min", "0",  4.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
var colon = label_new("colon", ":",  5.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
var t_tsec = label_new("t_tsec", "0", 6.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
var t_sec = label_new("t_sec", "0",  7.0*BIG_W, BIG_START+BIG_H, BIG_W, BIG_H);
var o_tsec = label_new("o_tsec", "0", 4.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);
var o_sec = label_new("o_sec", "0",  5.0*BIG_W, BIG_START+2*BIG_H, BIG_W, BIG_H);

var points = label_new("points", "-", 6.5*BIG_W, BIG_START+2*BIG_H, 1.5*BIG_W, BIG_H);
var pts_to_blue = label_new("pts_to_blue", "?", 6.0*BIG_W, BIG_START+2*BIG_H, 0.5*BIG_W, BIG_H/2.0);
var pts_to_white = label_new("pts_to_white", "?", 6.0*BIG_W, BIG_START+2.5*BIG_H, 0.5*BIG_W, BIG_H/2.0);
var comment = label_new("comment", "-", 0.0, 2.0*SMALL_H, 1.0, SMALL_H);

var cat1 = label_new("cat1", "-", 0.0, SMALL_H, TXTW, SMALL_H);
var cat2 = label_new("cat2", "-", 0.5, SMALL_H, TXTW, SMALL_H);
var gs = label_new("gs", "", 0.0, 0.0, 0.0, 0.0);
var flag_blue = label_new("flag_blue", "", 0.0, 0.0, 0.0, 0.0);
var flag_white = label_new("flag_white", "", 0.0, 0.0, 0.0, 0.0);

var roundnum = label_new("roundnum", "Round", 0.0, 0.0, 0.0, 0.0);
var comp1_country = label_new("comp1_country", "", 0.0, 0.0, 0.0, 0.0);
var comp2_country = label_new("comp2_country", "", 0.0, 0.0, 0.0, 0.0);

var comp1_leg_grab = label_new("comp1_leg_grab", "LG", 0.0, 0.0, 0.0, 0.0);
var comp2_leg_grab = label_new("comp2_leg_grab", "LG", 0.0, 0.0, 0.0, 0.0);

var padding1 = label_new("padding1", "",  0.0, 0.0, 0.0, 0.0);
var padding2 = label_new("padding2", "",  0.0, 0.0, 0.0, 0.0);
var padding3 = label_new("padding3", "",  0.0, 0.0, 0.0, 0.0);

var bgcolor = "#00f";
var bgcolor_pts = "#000";
var bgcolor_points = "#000";
var clock_run = "#fff", clock_stop = "yellow", clock_bg = "#000";
var oclock_run = "green", oclock_stop = "#777", oclock_bg = "#000";

var hide_clock_if_osaekomi = 0, no_frames = 0,
    hide_zero_osaekomi_points = 0, slave_mode = 0,
    hide_scores_if_zero = 0;

var current_osaekomi_state = 0;

var num2str = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "+"];
var pts2str = ["-", "K", "Y", "W", "I", "S"];
var blue_wins_voting = false, white_wins_voting = false;
var hansokumake_to_blue = false, hansokumake_to_white = false;
var result_hikiwake = false;

var big_dialog = false, big_text, big_end;

var rules_leave_score = true;
var rules_stop_ippon_2 = false;
var rules_confirm_match = true;
var menu_hidden = false;

var saved_first1, saved_first2, saved_last1, saved_last2, saved_cat;
var saved_country1, saved_country2, saved_round = 0;

// translations

var words = [];
var wordix = 0;
var tmo;
var clocktmo;

var I = 0, W = 1, Y = 2, S = 3, L = 4;

var osaekomi_winner;
var action_on = false;
var running = false, oRunning = false;
var elap = 0, oElap = 0, startTime = 0, oStartTime = 0;
var total = 240000;
var rest_time = false;
var score = 0;
var BLUE = 1, WHITE = 2, CMASK = 3, GIVE_POINTS = 4;
var osaekomi_winner = 0;
var bluepts = [0, 0, 0, 0, 0];
var whitepts = [0, 0, 0, 0, 0];
var ippon = 20000;
var show_soremade = false;
var flash = 0;
var last_m_otime = 0, last_m_run = 0;
var big_displayed = false;
var match_time;
var stackval = [{who: 0, points: 0}, {who: 0, points: 0},
		{who: 0, points: 0}, {who: 0, points: 0}];
var stackdepth = 0;

var last_m_time;
var last_m_run;
var last_m_otime;
var last_m_orun;
var last_score;
var osaekomi = ' ';

var koka = 0;
var yuko = 0;
var wazaari = 10000;
var ippon = 20000;
if (!use_2017_rules) {
    yuko = 10000;
    wazaari = 15000;
    ippon = 20000;
}

var gs_time = 0;
var tatami = 1;
var automatic = true;
var short_pin_times = false;
var golden_score = false;
var rest_time = false;
var rest_flags = 0;
var gs_cat = 0, gs_num = 0;
var require_judogi_ok = false;
var jcontrol;
var jcontrol_flags;

var asking = false;
var key_pending = 0;
var ASK_OK  = 0x7301;
var ASK_NOK = 0x7302;
var legend = 0;

var current_category = 0, current_match = 0;
var traffic_last_rec_time;
var msgout;
var result_send_time;

var lastkey;
var lasttime, now;
var comp_window = null, show_window = null;
var sound = 0;
var last_shido_to = 0;

function _(t) {
    return t;
}

function approve_osaekomi_score(who) {
    if (stackdepth == 0)
        return false;

    stackdepth--;

    if (who == 0)
        who = stackval[stackdepth].who;

    switch (stackval[stackdepth].points) {
    case 1:
        break;
    case 2:
	incdecpts(who, Y, 0);
        break;
    case 4:
	incdecpts(who, I, 0);
    case 3:
	incdecpts(who, W, 0);
        break;
    }

    check_ippon();

    return true;
}

function array2int(pts)
{
    var x = 0;
    x = (pts[0] << 16) | (pts[1] << 12) | (pts[2] << 8) | pts[3];
    return x;
}

function ask_ok() {
    asking = false;
    reset(ASK_OK);
}

function ask_nok() {
    asking = false;
    reset(ASK_NOK);
}

function askone() {
    if (wordix >= words.length) {
	window.clearTimeout(tmo);
	return;
    }
    xlate(words[wordix]);
    wordix++;
}

function beep(txt) {
    if (big_displayed)
        return;

    display_big(txt, 4);
    big_displayed = true;

    var audio = new Audio(audiofiles[sound > 0 ? sound-1 : tatami-1]);
    audio.play();
}

function cancel_rest_time(comp1, comp2) {
    var msgout = [commver,MSG_CANCEL_REST_TIME,0,myaddr,
		  current_category, current_match, comp1, comp2];
    sendmsg(msgout);
}

function check_ippon() {
    if (bluepts[I] > 1)
        bluepts[I] = 1;
    if (whitepts[I] > 1)
        whitepts[I] = 1;

    if (!use_2017_rules) {
	if (bluepts[W] >= 2) {
	    bluepts[I] = 1;
	    bluepts[W] = 0;
	}
	if (whitepts[W] >= 2) {
	    whitepts[I] = 1;
	    whitepts[W] = 0;
	}
    }

    set_points(bluepts, whitepts);

    if (whitepts[I] || bluepts[I]) {
        if (rules_stop_ippon_2 == true) {
            if (oRunning)
		oToggle();
            if (running)
		toggle();
        }
        beep("IPPON");
    } else if (golden_score && get_winner(false)) {
        beep("Golden Score");
    } else if ((whitepts[S] >= SHIDOMAX() || bluepts[S] >= SHIDOMAX()) &&
               rules_stop_ippon_2) {
    	if (oRunning)
    		oToggle();
    	if (running)
    		toggle();
        beep("SHIDO, Hansokumake");
    }
}

function clock_key(key, event_state) {
    var i;
    var shift = event_state & 1;
    var ctl = event_state & 4;

    //console.log("clock key="+key);
    now = Date.now();
    if (now < lasttime + 100 && key == lastkey)
        return;

    lastkey = key;
    lasttime = now;

    if (key == GDK_c && ctl) {
    	//manipulate_time(main_window, (gpointer)4 );
    	return;
    }
    if (key == GDK_b && ctl) {
    	if (white_first) {
    		voting_result(HANTEI_WHITE);
    	} else {
           	voting_result(HANTEI_BLUE);
    	}
    	return;
    }
    if (key == GDK_w && ctl) {
    	if(white_first){
    		voting_result(HANTEI_BLUE);
    	}else{
           	voting_result(HANTEI_WHITE);
    	}
    	return;
    }
    if (key == GDK_e && ctl) {
       	voting_result(CLEAR_SELECTION);
    	return;
    }

    if (key == GDK_t) {
        display_comp_window(saved_cat, saved_last1, saved_last2, 
                            saved_first1, saved_first2, saved_country1, saved_country2, saved_round);
        return;
    }

    if (key >= GDK_Return && key <= GDK_F10)
	action_on = true;

    switch (key) {
    case GDK_0:
        automatic = true;
        reset(GDK_7, null);
        break;
    case GDK_1:
    case GDK_2:
    case GDK_3:
    case GDK_4:
    case GDK_5:
    case GDK_6:
    case GDK_9:
        automatic = false;
        reset(key);
        break;
    case GDK_space:
        toggle();
        break;
    case GDK_k:
	// comp1 leg grab
	bluepts[L] = !bluepts[L];
        check_ippon();
	break;
    case GDK_l:
	// comp2 leg grab
	whitepts[L] = !whitepts[L];
        check_ippon();
	break;
    case GDK_s:
        break;
    case GDK_Up:
        set_osaekomi_winner(BLUE);
        break;
    case GDK_Down:
        set_osaekomi_winner(WHITE);
        break;
    case GDK_Return:
    case GDK_KP_Enter:
        oToggle();
        if (!oRunning) {
            give_osaekomi_score();
        }
        break;
    case GDK_F5:
	if (whitepts[I] && !shift)
	    break;
        incdecpts(WHITE, I, shift);
        check_ippon();
        break;
    case GDK_F6:
        incdecpts(WHITE, W, shift);
        check_ippon();
        break;
    case GDK_F7:
        incdecpts(WHITE, Y, shift);
        break;
    case GDK_F8:
        if (shift) {
            if (whitepts[S] >= SHIDOMAX()) bluepts[I] = 0;
            incdecpts(WHITE, S, DEC);
	    last_shido_to = 0;
        } else {
            incdecpts(WHITE, S, INC);
            if (whitepts[S] >= SHIDOMAX()) {
		bluepts[I] = 1;
		whitepts[S] = SHIDOMAX();
	    }
	    last_shido_to = WHITE;
        }
	check_ippon();
        break;
    case GDK_F1:
	if (bluepts[I] && !shift)
	    break;
        incdecpts(BLUE, I, shift);
        check_ippon();
        break;
    case GDK_F2:
        incdecpts(BLUE, W, shift);
        check_ippon();
        break;
    case GDK_F3:
        incdecpts(BLUE, Y, shift);
        break;
    case GDK_F4:
        if (shift) {
            if (bluepts[S] >= SHIDOMAX()) whitepts[I] = 0;
            incdecpts(BLUE, S, DEC);
	    last_shido_to = 0;
        } else {
            incdecpts(BLUE, S, INC);
            if (bluepts[S] >= SHIDOMAX()) {
		whitepts[I] = 1;
		bluepts[S] = SHIDOMAX();
	    }
	    last_shido_to = BLUE;
        }
	check_ippon();
        break;
    default:
        ;
    }

    /* check for shido amount of points */
    if (bluepts[S] > SHIDOMAX())
        bluepts[S] = SHIDOMAX();

    if (whitepts[S] > SHIDOMAX())
        whitepts[S] = SHIDOMAX();

    //check_ippon();
}

function clock_running() {
    return action_on;
}

function connect(){
    try {
	var url = window.location.href;
	var aa = url.lastIndexOf(':');
	var host = url.slice(7, aa);

	$("body").css("background-color", "red");

	webSocket = new ReconnectingWebSocket('ws://' + host + ':2315/');

        webSocket.onopen = function() {
	    sendmsg([commver,MSG_DUMMY,0,myaddr,apptype,0]);
	    sendmsg([commver,MSG_ALL_REQ,0,myaddr]);
	    $("body").css("background-color", "white");
	    get_translations();
        }

        webSocket.onmessage = function(msg){
	    parse_msg(msg.data);
        }

        webSocket.onclose = function(){
	    $("body").css("background-color", "red");
        }
    } catch(exception) {
   	message('<p>Error: '+exception);
    }
}

function create_ask_window() {
    var width, height, winner = get_winner(true);
    var last_wname, first_wname, wtext = _("WINNER");

    if (!show_competitor_names || !winner) {
	var yes = confirm("Start New Match?");
	if (yes) ask_ok();
	else ask_nok();
	return;
    }

    if (winner == BLUE) {
        last_wname = saved_last1;
        first_wname = saved_first1;
    } else {
        last_wname = saved_last2;
        first_wname = saved_first2;
    }

    var win = window.open(winner == WHITE ? "ask_new_match2.html" : "ask_new_match.html",
			  "_blank",
			  "height="+canvas_height+",width="+canvas_width+
			  ",status=yes,toolbar=no,menubar=no,location=no,addressbar=no");
    win.focus();
    win.onload = function() {
	var fsize = getpxver(0.2);
	win.document.getElementById("askwin").innerHTML =
	    "<span style='font-size:"+fsize+"px'>"+wtext+"</span>";
	win.document.getElementById("askcat").innerHTML =
	    "<span style='font-size:"+fsize+"px'>"+saved_cat+"</span>";
	win.document.getElementById("asklast").innerHTML =
	    "<span style='font-size:"+fsize+"px'>"+last_wname+"</span>";
	win.document.getElementById("askfirst").innerHTML =
	    "<span style='font-size:"+fsize+"px'>"+first_wname+"</span>";
	win.setcolor();
	win.onkeydown = function(e) {
	    var k = e.which || e.keyCode;
	    win.close();
	    if (k == GDK_y || k == GDK_space || k == GDK_Return)
		ask_ok();
	    else
		ask_nok();
	}
    }
}

function delete_big(data) {
    big_dialog = false;
    if (show_window) show_window.close();
    show_window = null;
    init_display();
}

function display_comp_window() {
    if (comp_window) return;

    comp_window = window.open("show_comp.html", "_blank",
			      "height="+canvas_height+",width="+canvas_width+
			      ",status=yes,toolbar=no,menubar=no,location=no,addressbar=no");
    comp_window.onload = function() {
	var fsize = getpxver(0.2);
	comp_window.document.getElementById("compcat").innerHTML =
	    "<span style='font-size:"+fsize+"px'>"+saved_cat+"</span>";
	comp_window.document.getElementById("comp1").innerHTML =
	    "<span style='font-size:"+fsize+"px'>"+saved_last1+"</span>";
	comp_window.document.getElementById("comp2").innerHTML =
	    "<span style='font-size:"+fsize+"px'>"+saved_last2+"</span>";
	comp_window.onkeydown = function(e) {
	    var k = e.which || e.keyCode;
	    if (k == GDK_space || k == GDK_Return)
		comp_window.close();
	}
	comp_window.onunload = function() {
	    comp_window = null;
	}
    }
}

function display_big(txt, tmo_sec) {
    big_text = txt;
    big_end = Date.now()/1000 + tmo_sec;
    big_dialog = true;
    if (no_big_text == false)
	show_big();
}

function expose() {
    var i;

    $('#timercanvas').clearCanvas();
    $('#timercanvas').drawRect({
	fillStyle: '#000',
	opacity: 1,
	x: 0, y: 0,
	width: canvas_width,
	height: canvas_height,
	fromCenter: false,
	/*
	click: function(layer) {
	    console.log("Click");
	}
	*/
    });

    /*console.log("canvas size w="+$('#timercanvas').width()+
	       " h="+$('#timercanvas').height());*/

    expose_label(padding);
    expose_label(padding1);
    expose_label(padding2);
    expose_label(padding3);
    
    for (i = 0; i < labels.length; i++) {
	if (i != padding && i != padding1 &&
	    i != padding2 && i != padding3)
	    expose_label(i);
    }
}

function expose_label(w) {
    if (labels[w].w == 0.0)
        return;

    if (labels[w].text == undefined) labels[w].text = "";

    $('#timercanvas').drawRect({
	fillStyle: labels[w].bg,
	x: getpxhor(labels[w].x),
	y: getpxver(labels[w].y),
	width: getpxhor(labels[w].w),
	height: getpxver(labels[w].h),
	fromCenter: false
    });

    var s = labels[w].size > 0 ? labels[w].size : 0.8;

    if ((w != flag_blue && w != flag_white &&
	 w != bs && w != ws) ||
	((w == bs || w == ws) && !show_shido_cards &&
	 (hide_zero_osaekomi_points == 0 ||
	  labels[w].text != "0"))) {
	if (labels[w].xalign < 0)
	    $('#timercanvas').drawText({
		fillStyle: labels[w].fg,
		x: getpxhor(Number(labels[w].x)),
		y: getpxver(Number(labels[w].y) + Number(labels[w].h)/2),
		fontSize: getpxver(s*labels[w].h),
		fontFamily: 'Verdana, sans-serif',
		text: labels[w].text,
		align: 'left',
		respectAlign: true,
	    });
	else
	    $('#timercanvas').drawText({
		fillStyle: labels[w].fg,
		x: getpxhor(Number(labels[w].x) + Number(labels[w].w)/2),
		y: getpxver(Number(labels[w].y) + Number(labels[w].h)/2),
		fontSize: getpxver(s*labels[w].h),
		fontFamily: 'Verdana, sans-serif',
		text: labels[w].text
	    });
    }

    if ((w == flag_blue || w == flag_white) &&
	labels[w].text.length == 3) {
	var img = document.createElement('img');
	img.src = "flags-ioc/" + labels[w].text + ".png";
	img.onload = function() {
	    var c = document.getElementById('timercanvas');
	    var ctx = c.getContext('2d');
	    ctx.drawImage(img,
			  getpxhor(Number(labels[w].x)),
			  getpxver(Number(labels[w].y)),
			  getpxhor(Number(labels[w].w)),
			  getpxver(Number(labels[w].h)));
	}
    } else if ((w == bs || w == ws) && show_shido_cards) {
	var img = document.createElement('img');
	if (labels[w].text == "0") img.src = shido_cards[0];
	else if (labels[w].text == "1") img.src = shido_cards[1];
	else if (labels[w].text == "2") img.src = shido_cards[2];
	else if (labels[w].text == "3") img.src = shido_cards[3];
	img.onload = function() {
	    var c = document.getElementById('timercanvas');
	    var ctx = c.getContext('2d');
	    ctx.drawImage(img,
			  getpxhor(Number(labels[w].x)),
			  getpxver(Number(labels[w].y)),
			  getpxhor(Number(labels[w].w)),
			  getpxver(Number(labels[w].h)));
	}
    }
}

function get_color(r, g, b) {
    return "rgb(" + Math.floor(r*255) + "," +
	Math.floor(g*255) + "," + Math.floor(b*255) + ")";
}

function get_match_time() {
    return elap + match_time;
}

function getpxhor(val) {
    return Math.floor(canvas_width*val);
}

function getpxver(val) {
    return Math.floor(canvas_height*val);
}

function get_translations() {
    $(".lang").each(function() {
	words.push($(this).text());
	//xlate($(this).text());
    });
    tmo = window.setInterval(askone, 100);
}

function get_winner(final_result) {
    var winner = 0;
    if (bluepts[I] > whitepts[I]) winner = BLUE;
    else if (bluepts[I] < whitepts[I]) winner = WHITE;
    else if (bluepts[W] > whitepts[W]) winner = BLUE;
    else if (bluepts[W] < whitepts[W]) winner = WHITE;
    else if (bluepts[Y] > whitepts[Y]) winner = BLUE;
    else if (bluepts[Y] < whitepts[Y]) winner = WHITE;
    else if (blue_wins_voting) winner = BLUE;
    else if (white_wins_voting) winner = WHITE;
    else if (hansokumake_to_white) winner = BLUE;
    else if (hansokumake_to_blue) winner = WHITE;
    else if (use_2017_rules && !final_result) {
	if (golden_score) {
	    if (last_shido_to == BLUE &&
		(bluepts[S] > whitepts[S])) winner = WHITE;
	    else if (last_shido_to == WHITE &&
		     (whitepts[S] > bluepts[S])) winner = BLUE;
	}
    } else if (bluepts[S] || whitepts[S]) {
        if (bluepts[S] > whitepts[S]) winner = WHITE;
        else if (bluepts[S] < whitepts[S]) winner = BLUE;
    }

    return winner;
}

function give_osaekomi_score() {
    if (score) {
        stackval[stackdepth].who = osaekomi_winner;
        stackval[stackdepth].points = score;
        if (stackdepth < 3)
            stackdepth++;
    }

    score = 0;
    osaekomi_winner = 0;
}

function hajime_inc_func() {
    if (running)
        return;
    if (total == 0) {
        elap += 1000;
    } else if (elap >= 1000)
        elap -= 1000;
    else
        elap = 0;
    update_display();
}

function hajime_dec_func() {
    if (running)
        return;
    if (total == 0) {
        if (elap >= 1000) elap -= 1000;
    } else if (elap <= (total - 1000))
        elap += 1000;
    else
        elap = total;
    update_display();
}

function handle_click(w, shift) {
    switch (w) {
    case match2:
	clock_key(GDK_0, shift ? 1 : 0);
	break;
    case wazaari:
    case yuko:
    case koka:
    case shido:
    case padding:
	if (big_dialog) delete_big();
	break;
    case sonomama:
	if (big_dialog) delete_big();
	clock_key(GDK_s, 0);
	break;
    case bw: case by: case bk: case bs:
	if (set_osaekomi_winner(BLUE | GIVE_POINTS))
            return;
	clock_key(GDK_F1+w-bw, shift ? 1 : 0);
	break;
    case ww: case wy: case wk: case ws:
	if (set_osaekomi_winner(WHITE | GIVE_POINTS))
            return;
	clock_key(GDK_F5+w-ww, shift ? 1 : 0);
	break;
    case t_min: case colon: case t_tsec: case t_sec:
	clock_key(GDK_space, shift ? 1 : 0);
	break;
    case o_tsec: case o_sec:
	clock_key(GDK_Return, shift ? 1 : 0);
	break;
    case points:
	set_osaekomi_winner(0);
	break;
    case pts_to_blue:
	set_osaekomi_winner(BLUE);
	break;
    case pts_to_white:
	set_osaekomi_winner(WHITE);
	break;
    case comp1_leg_grab:
	clock_key(GDK_k, shift ? 1 : 0);
	break;
    case comp2_leg_grab:
	clock_key(GDK_l, shift ? 1 : 0);
	break;
    }
}

function incdecpts(who, pos, dec) {
    if (dec) big_displayed = false;

    if (who == BLUE) {
	if (dec) {
	    if (bluepts[pos] > 0) bluepts[pos]--;
	} else {
	    if (bluepts[pos] < 9) bluepts[pos]++;
	}
    } else {
	if (dec) {
	    if (whitepts[pos] > 0) whitepts[pos]--;
	} else {
	    if (whitepts[pos] < 9) whitepts[pos]++;
	}
    }
}

function init_display() {
    expose();
}

function label_new(id1, txt1, x1, y1, w1, h1) {
    var ret = labels.length;
    var a = {
	id: id1,
	x: Number(x1),
	y: Number(y1),
	w: Number(w1),
	h: Number(h1),
	text: txt1,
	size: Number(0),
	xalign: 0,
	fg: '#FFF',
	bg: '#000',
	status: 0
    };
    labels.push(a);
    return ret;
}

function message(m) {
    console.log("Message:" + m);
}

function get_name_by_layout(first, last, club, country) {
    switch (selected_name_layout) {
    case 0:
        if (!country || country == "")
            return first+" "+last+", "+club;
        else if (!club || club == "")
            return first+" "+last+", "+country;
        return first+" "+last+", "+country+"/"+club;
    case 1:
        if (!country || country == "")
            return last+", "+first+", "+club;
        else if (!club || club == "")
            return last+", "+first+", "+country;
        return last+", "+first+", "+country+"/"+club;
    case 2:
        if (!country || country == "")
            return club+"  "+last+", "+first;
        else if (!club || club == "")
            return country+"  "+last+", "+first;
        return country+"/"+club+"  "+last+", "+first;
    case 3:
        return country+"  "+last+", "+first;
    case 4:
        return club+"  "+last+", "+first;
    case 5:
        return country+"  "+last;
    case 6:
        return club+"  "+last;
    case 7:
        return last+", "+first;
    case 8:
        return last;
    case 9:
        return country;
    case 10:
        return club;
    }

    return NULL;
}

function num_to_str(num) {
    return String(num);
}

function osaekomi_dec_func() {
    if (running)
        return;
    if (oElap >= 1000)
        oElap -= 1000;
    else
        oElap = 0;
    update_display();
}

function osaekomi_inc_func() {
    if (running)
        return;
    if (oElap <= (ippon - 1000))
        oElap += 1000;
    else
        oElap = ippon;
    update_display();
}

function osaekomi_running() {
    return oRunning;
}

function oToggle() {
    /*
    if (running == false && (elap < total || total == 0)) {
        if (oRunning) // yoshi
            toggle();
        return;
    }
    */
    if (oRunning) {
        oRunning = false;
        oElap = 0;
        set_comment_text("");
        update_display();
        if (total > 0 && elap >= total)
            beep("SOREMADE");
    } else {
	if (!running) toggle();
        //running = true;
        osaekomi = ' ';
        oRunning = true;
        oStartTime = Date.now();
        update_display();
    }
}

function parse_msg(data) {
    var message;
    try {
	message = JSON.parse(data);
    } catch(exception) {
   	console.log('Error: '+exception+" data="+data);
	return;
    }
    var msg = message.msg;
    if (!msg) return;
    //console.log(JSON.stringify(message, null));

    if (msg[1] == MSG_LANG) {
	$(".lang").each(function() {
	    if ($(this).text() == msg[4])
		$(this).html(msg[5]);
	});

	//$(".lang:contains("+msg[4]+")").html(msg[5]);
    } else if (msg[1] == MSG_NEXT_MATCH) {
	if (msg[4] != tatami) return;
	if (clock_running()) return;

	show_message(msg[16], msg[17], msg[18], msg[19], msg[20], msg[21],
		     msg[22], msg[23]);
	if (msg[7] > 0 && automatic)
            reset(GDK_0, msg);
        if (golden_score)
            set_comment_text(_("Golden Score"));
        if (current_category != msg[5] ||
            current_match != msg[6]) {
	    display_comp_window(saved_cat, saved_last1, saved_last2,
				saved_first1, saved_first2,
				saved_country1, saved_country2, saved_round);
            current_category = msg[5];
            current_match = msg[6];
	}
    }
}

function pts_to_str(num) {
    if (num < 6)
        return pts2str[num];
    return ".";
}

function reset(key, msg) {
    var i;
    var asked = false;

    console.log("reset key="+key);
    if (key == ASK_OK) {
        key = key_pending;
        key_pending = 0;
        asking = false;
        asked = true;
    } else if (key == ASK_NOK) {
        key_pending = 0;
        asking = false;
        return;
    }

    if ((running && rest_time == false) || oRunning || asking)
        return;

    labels[gs].text = "";

    if (key != GDK_0) {
        golden_score = false;
    }

    var bp;
    var wp;
    bp = array2int(bluepts);
    wp = array2int(whitepts);

    if (use_2017_rules) {
	// ignore shidos
	bp &= ~0xf;
	wp &= ~0xf;
    }

    //console.log("asked="+asked+" rules_confirm_match="+rules_confirm_match+" key="+key);
    if (key == GDK_9 ||
        (bp == wp && result_hikiwake == false &&
         blue_wins_voting == white_wins_voting &&
         total > 0 && elap >= total && key != GDK_0)) {

        //ask_for_golden_score();
	var yes = confirm("Start Golden Score?");
	if (yes) {
	    last_shido_to = 0;
            golden_score = true;
            gs_cat = current_category;
            gs_num = current_match;
            automatic = false;
	} else {
            gs_cat = 0;
            gs_num = 0;
            golden_score = false;
	}

        if (key == GDK_9 && golden_score == false)
            return;
        key = GDK_9;
        match_time = elap;
	labels[gs].text = "GOLDEN SCORE";
    } else if (asked == false &&
               ((bluepts[I] == 0 && whitepts[I] == 0 &&
                 elap > 1 && elap < total - 1) ||
                (running && rest_time) ||
                rules_confirm_match) &&
               key != GDK_0) {
        key_pending = key;
        asking = true;
        create_ask_window();
        return;
    }

    if (key != GDK_0 && rest_time) {
        if (rest_flags & MATCH_FLAG_BLUE_REST)
            cancel_rest_time(true, false);
        else if (rest_flags & MATCH_FLAG_WHITE_REST)
            cancel_rest_time(false, true);
    }

    rest_time = false;

    if (key != GDK_0 && golden_score == false) {
        // set bit if golden score
        if (gs_cat == current_category && gs_num == current_match)
            legend |= 0x100;

        send_result(bluepts, whitepts,
                    blue_wins_voting, white_wins_voting,
                    hansokumake_to_blue, hansokumake_to_white, legend, result_hikiwake);
        match_time = 0;
        gs_cat = gs_num = 0;
    }

    if (golden_score == false || rules_leave_score == false) {
	bluepts = [0, 0, 0, 0, 0];
	whitepts = [0, 0, 0, 0, 0];
    }

    blue_wins_voting = 0;
    white_wins_voting = 0;
    hansokumake_to_blue = 0;
    hansokumake_to_white = 0;
    if (key != GDK_0) result_hikiwake = 0;
    osaekomi_winner = 0;
    big_displayed = false;

    running = false;
    elap = 0;
    oRunning = false;
    oElap = 0;
    stackdepth = 0;
    koka    = 5000;
    yuko    = 10000;
    wazaari = 15000;
    ippon   = 20000;

    switch (key) {
    case GDK_0:
        if (msg) {
            /*g_print("is-gs=%d match=%d gs=%d rep=%d flags=0x%x\n", 
              golden_score, msg->match_time, msg->gs_time, msg->rep_time, msg->flags);*/
            if ((msg[22] & MATCH_FLAG_REPECHAGE) && msg[10]) {
                total = msg[10]*1000;
                golden_score = true;
            } else if (golden_score)
                total = msg[9]*1000; //gs_time;
            else
                total = msg[8]*1000; //match_time;

            koka    = msg[15]*1000; //->pin_time_koka;
            yuko    = msg[14]*1000; //->pin_time_yuko;
            wazaari = msg[13]*1000; //->pin_time_wazaari;
            ippon   = msg[12]*1000; //->pin_time_ippon;

            jcontrol = false;
            if (require_judogi_ok &&
                (!(msg[22] & MATCH_FLAG_JUDOGI1_OK) ||
                 !(msg[22] & MATCH_FLAG_JUDOGI2_OK))) {
                var buf = "CONTROL: " +
                    (!(msg[22] & MATCH_FLAG_JUDOGI1_OK) ?
		     labels[blue_name_1].text : labels[white_name_1].text);
		display_big(buf, 2);
                jcontrol = true;
                jcontrol_flags = msg[22];//->flags;
            }
	    if (msg[11]/*rest_time*/) {
                var buf = "REST TIME: " +
                    (msg[7] & MATCH_FLAG_BLUE_REST ?
		     labels[blue_name_1].text : labels[white_name_1].text);
                rest_time = true;
                rest_flags = msg[7]; //->minutes;
                total = msg[11]*1000; //->rest_time;
                startTime = Date.now();
                running = true;
                display_big(buf, msg[11]);
                return;
            }
        }
        break;
    case GDK_1:
        total   = golden_score ? gs_time : 120000;
        koka    = 0;
        yuko    = 5000;
        wazaari = 10000;
        ippon   = 15000;
        break;
    case GDK_2:
        total   = golden_score ? gs_time : 120000;
        break;
    case GDK_3:
        total   = golden_score ? gs_time : 180000;
        break;
    case GDK_4:
        total   = golden_score ? gs_time : 240000;
        break;
    case GDK_5:
        total   = golden_score ? gs_time : 300000;
        break;
    case GDK_6:
        total   = golden_score ? gs_time : 150000;
        break;
    case GDK_7:
        total   = 10000000;
        break;
    case GDK_9:
        if (golden_score)
            total = gs_time;
        break;
    }

    if (short_pin_times) {
        koka    = 0;
        yuko    = 5000;
        wazaari = 10000;
        ippon   = 15000;
    }

    action_on = false;

    if (key != GDK_0) {
        set_comment_text("");
        delete_big();
    }

    reset_display(key);
    update_display();

    if (key != GDK_0) {
        //display_ad_window();
    }
}

function reset_display(key) {
    var pts = [0,0,0,0,0];

    set_timer_run_color(false, false);
    set_timer_osaekomi_color(OSAEKOMI_DSP_NO, 0);
    set_osaekomi_value(0, 0);

    if (golden_score == false)
        set_points(pts, pts);
}

function round_to_str(round) {
    if (round == 0)
	return "";

    switch (round & ROUND_TYPE_MASK) {
    case 0:
	return "Round " + (round & ROUND_MASK);
    case ROUND_ROBIN:
	return "Round Robin";
    case ROUND_REPECHAGE:
	return "Repechage";
    case ROUND_SEMIFINAL:
	if ((round & ROUND_UP_DOWN_MASK) == ROUND_UPPER)
            return "Semifinal 1";
	else if ((round & ROUND_UP_DOWN_MASK) == ROUND_LOWER)
            return "Semifinal 2";
	else
            return "Semifinal";
    case ROUND_BRONZE:
	if ((round & ROUND_UP_DOWN_MASK) == ROUND_UPPER)
            return "Bronze 1";
	else if ((round & ROUND_UP_DOWN_MASK) == ROUND_LOWER)
            return "Bronze 2";
	else
            return "Bronze";
    case ROUND_SILVER:
	return "Silver medal match";
    case ROUND_FINAL:
	return "Final";
    }
    return "";
}

function sendmsg(msg) {
    var data = JSON.stringify({"msg":msg});
    webSocket.send(data);
}

function send_result(bluepts, whitepts, blue_vote, white_vote,
		     blue_hansokumake, white_hansokumake, legend, hikiwake) {
    if (current_category < 10000 || current_match >= 1000) {
        return;
    }
    var msgout = [commver,MSG_RESULT,0,myaddr,
		  tatami, current_category, current_match, get_match_time(),
		  array2int(bluepts), array2int(whitepts),
		  hikiwake ? 1 : blue_vote, hikiwake ? 1 : white_vote,
		  blue_hansokumake, white_hansokumake, legend];

    if (msgout[8] != msgout[9] ||
        msgout[10] != msgout[11] || hikiwake ||
        blue_hansokumake || white_hansokumake) {
	sendmsg(msgout);
        result_send_time = Date.now()/1000;
    }
}

function set_clocks(clock, osaekomi) {
    if (running)
        return;

    if (clock >= 0 && total >= clock)
    	elap = total - clock;
    else if (clock >= 0)
        elap = clock;

    if (osaekomi) {
        oElap = osaekomi;
        oRunning = true;
    }

    update_display();
}

function set_comment_text(txt)
{
    if (golden_score && !txt)
        txt = _("Golden Score");

    labels[comment].text = txt;
    expose_label(comment);

    if (big_dialog)
	show_big();
}

/*
function set_competitor_window_judogi_control(control, flags) {
    var isize = getpxver(0.25);
    if (!comp_window) return;

    if (!(flags & MATCH_FLAG_JUDOGI1_OK))
	comp_window.document.getElementById("comp1rest").innerHTML =
	"<img src='white-ctl.png' width='"+isize+"' height='"+isize+"'>";
    else
	comp_window.document.getElementById("comp1rest").innerHTML = "";

    if (!(flags & MATCH_FLAG_JUDOGI2_OK))
	comp_window.document.getElementById("comp2rest").innerHTML =
	"<img src='blue-ctl.png' width='"+isize+"' height='"+isize+"'>";
    else
	comp_window.document.getElementById("comp2rest").innerHTML = "";
}
*/

function set_competitor_window_rest_time(min, sec, rest, flags, ctl, ctlflags) {
    if (!comp_window) return;

    var fsize = getpxver(0.15);
    var isize = getpxver(0.25);

    if (ctl && !(ctlflags & MATCH_FLAG_JUDOGI1_OK))
	comp_window.document.getElementById("comp1rest").innerHTML =
	"<img src='white-ctl.png' width='"+isize+"' height='"+isize+"'>";
    else if (rest && sec > 2 && (flags & MATCH_FLAG_BLUE_REST))
	comp_window.document.getElementById("comp1rest").innerHTML =
	"<span style='font-size:"+fsize+"px;color:red;'>"+
	min+":"+(Math.floor(sec/10))+(sec%10)+"</span>";
    else
	comp_window.document.getElementById("comp1rest").innerHTML = "";

    if (ctl && !(ctlflags & MATCH_FLAG_JUDOGI2_OK))
	comp_window.document.getElementById("comp2rest").innerHTML =
	"<img src='blue-ctl.png' width='"+isize+"' height='"+isize+"'>";
    else if (rest && sec > 2 && (flags & MATCH_FLAG_WHITE_REST))
	comp_window.document.getElementById("comp2rest").innerHTML =
	"<span style='font-size:"+fsize+"px;color:red;'>"+
	min+":"+(Math.floor(sec/10))+(sec%10)+"</span>";
    else
	comp_window.document.getElementById("comp2rest").innerHTML = "";
}

function set_hantei_winner(cmd) {
    if (cmd == CLEAR_SELECTION)
        big_displayed = false;

    if (cmd == HANSOKUMAKE_BLUE)
        whitepts[I] = 1;
    else if (cmd == HANSOKUMAKE_WHITE)
        bluepts[I] = 1;

    check_ippon();
}

function set_label(args) {
    var w = args[0];

    if (w == 103) {
	hide_clock_if_osaekomi = args[1];
	no_frames = args[2];
	hide_zero_osaekomi_points = args[3];
	slave_mode = args[4];
	hide_scores_if_zero = args[5];
	return;
    }
    if (w < 0 || w > labels.length-1) return;

    labels[w].x = args[1];
    labels[w].y = args[2];
    labels[w].w = args[3];
    labels[w].h = args[4];
    labels[w].size = args[5];
    labels[w].xalign = args[6];
    labels[w].fg = get_color(args[7], args[8], args[9]);
    labels[w].bg = get_color(args[10], args[11], args[12]);
}

function set_number(w, num) {
    labels[w].text = String(num);
    expose_label(w);
}

function set_osaekomi_value(tsec, sec) {
    labels[o_tsec].text = tsec;
    labels[o_sec].text = sec;
    expose_label(o_tsec);
    expose_label(o_sec);
}

function set_osaekomi_winner(who) {
    if (!oRunning) {
        return approve_osaekomi_score(who & CMASK);
    }

    if (who & GIVE_POINTS)
        return false;

    osaekomi_winner = who;

    if (who == WHITE)
        set_comment_text(_("Points going to blue"));
    else if (who == BLUE)
        set_comment_text(_("Points going to white"));
    else
        set_comment_text("");

    return true;
}

function set_points(blue, white)
{
    if (blue[0])
	set_number(bw, 1);
    else
	set_number(bw, 0);

    set_number(by, blue[1]);
    set_number(bk, blue[2]);
    set_number(bs, blue[3]);

    if (white[0])
	set_number(ww, 1);
    else
	set_number(ww, 0);

    set_number(wy, white[1]);
    set_number(wk, white[2]);
    set_number(ws, white[3]);

    if (blue[4]) {
	labels[comp1_leg_grab].size = 0.4;
	labels[comp1_leg_grab].fg = '#000';
    } else {
	labels[comp1_leg_grab].size = 0.2;
	labels[comp1_leg_grab].fg = '#ccc';
    }

    if (white[4]) {
	labels[comp2_leg_grab].size = 0.4;
	labels[comp2_leg_grab].fg = '#fff';
    } else {
	labels[comp2_leg_grab].size = 0.2;
	labels[comp2_leg_grab].fg = '#ccf';
    }

    expose_label(comp1_leg_grab);
    expose_label(comp2_leg_grab);
}

function set_score(score)
{
    labels[points].text = pts_to_str(score);
    expose_label(points);
}

function set_timer_osaekomi_color(osaekomi_state, pts) {
    var fg = "#fff", bg = "#000", color = "green";

    if (pts) {
        if (osaekomi_state == OSAEKOMI_DSP_BLUE ||
            osaekomi_state == OSAEKOMI_DSP_WHITE ||
            osaekomi_state == OSAEKOMI_DSP_UNKNOWN ||
	    osaekomi_state == OSAEKOMI_DSP_YES2) {
            var b = bk, w = wk;
            var sb, sw;

            switch (pts) {
            case 2: b = bk; w = wk; break;
            case 3: b = by; w = wy; break;
            case 4: b = bw; w = ww; break;
            }

            sb = labels[b].size;
            sw = labels[w].size;

            if (osaekomi_state == OSAEKOMI_DSP_BLUE ||
                osaekomi_state == OSAEKOMI_DSP_UNKNOWN)
                labels[b].size = 0.5;
            if (osaekomi_state == OSAEKOMI_DSP_WHITE ||
                osaekomi_state == OSAEKOMI_DSP_UNKNOWN)
                labels[w].size = 0.5;

            expose_label(b);
            expose_label(w);
            labels[b].size = sb;
            labels[w].size = sw;
        }
    } else {
        expose_label(bw);
        expose_label(by);
        expose_label(bk);
        expose_label(ww);
        expose_label(wy);
        expose_label(wk);
    }

    if (oRunning) {
        fg = "green";
        bg = bgcolor_points;
    } else {
	switch (osaekomi_state) {
	case OSAEKOMI_DSP_NO:
	case OSAEKOMI_DSP_YES2:
            fg = "#777";
            bg = bgcolor_points;
            break;
	case OSAEKOMI_DSP_YES:
            fg = "green";
            bg = bgcolor_points;
            break;
	case OSAEKOMI_DSP_BLUE:
            fg = "#000"
            bg = "#fff";
            break;
	case OSAEKOMI_DSP_WHITE:
            fg = "#fff";
            bg = bgcolor;
            break;
	case OSAEKOMI_DSP_UNKNOWN:
            fg = "#fff";
            bg = bgcolor_points;
            break;
	}
    }

    labels[points].fg = fg;
    labels[points].bg = bg;

    var pts1, pts2;
    pts1 = pts_to_white;
    pts2 = pts_to_blue;

    current_osaekomi_state = osaekomi_state;

    if (oRunning) {
        color = oclock_run;
        labels[pts1].fg = "#fff";
        labels[pts1].bg = bgcolor;
        labels[pts2].fg = "#000";
        labels[pts2].bg = "#fff";
    } else {
        color = oclock_stop;
        labels[pts1].fg = "#777";
        labels[pts1].bg = bgcolor_pts;
        labels[pts2].fg = "#777";
        labels[pts2].bg = bgcolor_pts;
	if (hide_clock_if_osaekomi) {
	    expose_label(t_min);
	    expose_label(colon);
	    expose_label(t_tsec);
	    expose_label(t_sec);
	}
    }

    labels[o_tsec].fg = color;
    labels[o_sec].fg = color;

    expose_label(points);
    expose_label(o_tsec);
    expose_label(o_sec);
    expose_label(pts_to_blue);
    expose_label(pts_to_white);
}

function set_timer_run_color(running, resttime) {
    var color;

    if (running) {
	if (resttime)
	    color = "#f00";
	else
	    color = clock_run;
    } else
	color = clock_stop;

    labels[t_min].fg = color;
    labels[colon].fg = color;
    labels[t_tsec].fg = color;
    labels[t_sec].fg = color;

    expose_label(t_min);
    expose_label(colon);
    expose_label(t_tsec);
    expose_label(t_sec);
}

function set_timer_value(min, tsec, sec) {
    if (min <= 20) {
        labels[t_min].text = min%10;
        labels[t_tsec].text = tsec;
        labels[t_sec].text = sec;
    } else {
        labels[t_min].text = '-';
        labels[t_tsec].text = '-';
        labels[t_sec].text = '-';
    }

    expose_label(t_min);
    expose_label(t_tsec);
    expose_label(t_sec);
}

function SHIDOMAX() {
    return (use_2017_rules ? 3 : 4);
}

var no_decorations = ",status=0,toolbar=0,menubar=0,location=0,addressbar=0,directories=0,scrollbars=no,resizable=no,titlebar=0";

function show_big() {
    var fsize = getpxver(0.1);
    if (show_window) return;

    show_window = window.open("", "_blank",
			      "height="+(canvas_height/4)+",width="+canvas_width+
			      no_decorations);
    $(show_window.document.body).html(
	"<html><title></title><body>"+
	    "<div style='text-align: center; height: "+(canvas_height/4)+";'>"+
	    "<span style='display: inline-block; vertical-align: middle; font-size:"+
	    fsize+"px'>"+big_text+
	    "</span></div></body></html>");
    show_window.onunload = function() {
	show_window = null;
	console.log("big closed");
    }
}

function show_message(cat_1, blue_1, white_1, cat_2, blue_2, white_2, flags, rnd) {
    var buf, name;
    var b_tmp = blue_1, w_tmp = white_1;
    var b_first, b_last, b_club, b_country;
    var w_first, w_last, w_club, w_country;

    b_first = b_last = b_club = b_country = "";
    w_first = w_last = w_club = w_country = "";
    saved_first1 = saved_first2 = saved_last1 = saved_last2 = saved_cat = "";
    saved_country1 = saved_country2 = "";

    var tmp = blue_1.split("\t");
    b_first = tmp[1]; b_last = tmp[0]; b_club = tmp[3]; b_country = tmp[2];
    tmp = white_1.split("\t");
    w_first = tmp[1]; w_last = tmp[0]; w_club = tmp[3]; w_country = tmp[2];

    saved_last1 = b_last;
    saved_last2 = w_last;
    saved_first1 = b_first;
    saved_first2 = w_first;
    saved_cat = cat_1;
    saved_country1 = b_country;
    saved_country2 = w_country;

    labels[cat1].text = cat_1;
    // Show flags. Country must be in IOC format.
    labels[flag_blue].text = b_country;
    labels[flag_white].text = w_country;
    labels[comp1_country].text = b_country;
    labels[comp2_country].text = w_country;

    labels[blue_club].text = b_club;
    labels[white_club].text = w_club;

    name = get_name_by_layout(b_first, b_last, b_club, b_country);
    labels[blue_name_1].text = name;
    name = get_name_by_layout(w_first, w_last, w_club, w_country);
    labels[white_name_1].text = name;

    labels[cat2].text = cat_2;

    tmp = blue_2.split("\t");
    b_first = tmp[1]; b_last = tmp[0]; b_club = tmp[3]; b_country = tmp[2];
    tmp = white_2.split("\t");
    w_first = tmp[1]; w_last = tmp[0]; w_club = tmp[3]; w_country = tmp[2];

    name = get_name_by_layout(b_first, b_last, b_club, b_country);
    labels[blue_name_2].text = name;

    name = get_name_by_layout(w_first, w_last, w_club, w_country);
    labels[white_name_2].text = name;

    if (flags & MATCH_FLAG_JUDOGI1_NOK)
        labels[comment].text =
        white_first ?
        _("White has a judogi problem.") :
        _("Blue has a judogi problem.");
    else if (flags & MATCH_FLAG_JUDOGI2_NOK)
        labels[comment].text =
	white_first ?
	_("Blue has a judogi problem.") :
	_("White has a judogi problem.");
    else
        labels[comment].text = "";

    labels[roundnum].text = round_to_str(rnd);
    saved_round = rnd;

    expose_label(cat1);
    expose_label(roundnum);
    expose_label(blue_name_1);
    expose_label(white_name_1);
    expose_label(cat2);
    expose_label(blue_name_2);
    expose_label(white_name_2);
    expose_label(blue_club);
    expose_label(white_club);
    expose_label(flag_blue);
    expose_label(flag_white);
    expose_label(comment);
    expose_label(comp1_country);
    expose_label(comp2_country);
}

function timeout() {
    update_clock();

    if (big_dialog && Date.now()/1000 > big_end)
        delete_big(null);
}

function toggle() {
    if (running) {
        running = false;
        elap = Date.now() - startTime;
        if (total > 0 && elap >= total)
            elap = total;
        if (oRunning) {
            //oRunning = false;
            //oElap = 0;
            //give_osaekomi_score();
        } else {
            oElap = 0;
            score = 0;
        }
        update_display();
    } else {
        if (total > 9000000) // don't let clock run if dashes in display '-:--'
            return;

        running = true;
        startTime = Date.now() - elap;
        if (oRunning) {
            oStartTime = Date.now() - oElap;
            set_comment_text("");
        }
        update_display();
    }
}

function update_clock() {
    if (running) {
        elap = Date.now() - startTime;
        if (total > 0.0 && elap >= total) {
            //running = false;
            elap = total;
            if (!oRunning) {
                running = false;
                if (rest_time == false)
                        show_soremade = true;
                if (rest_time) {
                    elap = 0;
                    oRunning = false;
                    oElap = 0;
                    rest_time = false;
                }
            }
        }

        if (running && oRunning) {
            oElap = Date.now() - oStartTime;
            score = 0;
            if (oElap >= yuko && !use_2017_rules)
                score = 2;
            if (oElap >= wazaari) {
                score = 3;
		if (!use_2017_rules &&
		    ((osaekomi_winner == BLUE && bluepts[W]) ||
		     (osaekomi_winner == WHITE && whitepts[W]))) {
                    running = false;
                    oRunning = false;
                    give_osaekomi_score();
                    approve_osaekomi_score(0);
                }
            }
            if (oElap >= ippon) {
                score = 4;
                running = false;
                oRunning = false;
                var tmp = osaekomi_winner;
                give_osaekomi_score();
                if (tmp)
                    approve_osaekomi_score(0);
            }
        }
    }

    update_display();

    if (show_soremade) {
        beep("SOREMADE");
	show_soremade = false;
    }
}

var compwincnt = 0;

function update_display() {
    var t, min, sec, oSec;
    var orun, score1;

    // move last second to first second i.e. 0:00 = soremade
    if (elap == 0) t = Math.floor(total/1000);
    else if (total == 0) t = Math.floor(elap/1000);
    else {
        t = Math.floor((total - elap + 900)/1000);
        if (t == 0 && total > elap)
            t = 1;
    }

    min = Math.floor(t/60);
    sec =  t - min*60;
    oSec = Math.floor(oElap/1000);

    if (t != last_m_time) {
        last_m_time = t;
        set_timer_value(min, Math.floor(sec/10), sec%10);
    }

    if (oSec != last_m_otime) {
        last_m_otime = oSec;
        set_osaekomi_value(Math.floor(oSec/10), oSec%10);
    }

    if (running != last_m_run) {
        last_m_run = running;
        set_timer_run_color(last_m_run, rest_time);
    }

    if (oRunning) {
	if (score >= 2) {
            if (osaekomi_winner == BLUE) orun = OSAEKOMI_DSP_BLUE;
	    else if (osaekomi_winner == WHITE) orun = OSAEKOMI_DSP_WHITE;
	    else orun = OSAEKOMI_DSP_UNKNOWN;

            if (++flash > 5)
		orun = OSAEKOMI_DSP_YES2;
            if (flash > 10)
		flash = 0;
	} else {
	    orun = OSAEKOMI_DSP_YES;
	}

        score1 = score;
    } else if (stackdepth) {
        if (stackval[stackdepth-1].who == BLUE)
            orun = OSAEKOMI_DSP_BLUE;
        else if (stackval[stackdepth-1].who == WHITE)
            orun = OSAEKOMI_DSP_WHITE;
        else
            orun = OSAEKOMI_DSP_UNKNOWN;

        if (++flash > 5)
            orun = OSAEKOMI_DSP_YES2;
        if (flash > 10)
            flash = 0;

        score1 = stackval[stackdepth-1].points;
    } else {
        orun = OSAEKOMI_DSP_NO;
        score1 = 0;
    }

    if (orun != last_m_orun) {
        last_m_orun = orun;
        set_timer_osaekomi_color(last_m_orun, score1);
    }

    if (last_score != score1) {
        last_score = score1;
        set_score(last_score);
    }

    if (compwincnt > 10) {
	set_competitor_window_rest_time(min, sec, rest_time, rest_flags, jcontrol, jcontrol_flags);
	compwincnt = 0;
    }
    compwincnt++;
}

function voting_result(data) {
    blue_wins_voting = false;
    white_wins_voting = false;
    hansokumake_to_blue = false;
    hansokumake_to_white = false;
    result_hikiwake = false;

    switch (data) {
    case HANTEI_BLUE:
            labels[comment].text =_("White won the voting");
	blue_wins_voting = true;
        break;
    case HANTEI_WHITE:
        labels[comment].text =_("Blue won the voting");
	white_wins_voting = true;
	break;
    case HANSOKUMAKE_BLUE:
	labels[comment].text =_("Hansokumake to white");
	hansokumake_to_blue = true;
        break;
    case HANSOKUMAKE_WHITE:
        labels[comment].text =_("Hansokumake to blue");
	hansokumake_to_white = true;
        break;
    case HIKIWAKE:
        labels[comment].text =_("Hikiwake");
        result_hikiwake = true;
        break;
    case CLEAR_SELECTION:
        labels[comment].text ="";
        break;
    }
    expose_label(comment);

    set_hantei_winner(data);
}

function xlate(en) {
    var msg = [commver,MSG_LANG,0,myaddr,en,""];
    sendmsg(msg);
}

// Configure

function configure() {
    var i;
    setCookie("timertatami", tatami, 30);

    rules_stop_ippon_2 = $("#stopippon").prop('checked');
    setCookie("timerstopippon", rules_stop_ippon_2, 30);

    require_judogi_ok = $("#timerjudogi").prop('checked');
    setCookie("timerjudogi", require_judogi_ok, 30);

    setCookie("timersound", sound, 30);
}

function setCookie(cname, cvalue, exdays) {
    var d = new Date();
    d.setTime(d.getTime() + (exdays*24*60*60*1000));
    var expires = "expires="+ d.toUTCString();
    document.cookie = cname + "=" + cvalue + ";" + expires + ";path=/";
}

function getCookie(cname) {
    var name = cname + "=";
    var ca = document.cookie.split(';');
    for(var i = 0; i <ca.length; i++) {
        var c = ca[i];
        while (c.charAt(0)==' ') {
            c = c.substring(1);
        }
        if (c.indexOf(name) == 0) {
            return c.substring(name.length,c.length);
        }
    }
    return "";
}

function read_configuration() {
    tatami = getCookie("timertatami");
    if (!tatami || tatami < 1 || tatami > 10)
	tatami = 1;
    $("#sel"+tatami).prop('checked', true);

    sound = getCookie("timersound");
    $("#snd"+sound).prop('checked', true);

    if (getCookie("timerjudogi"))
	$("#judogicontrol").prop('checked', true);
    if (getCookie("timerstopippon"))
	$("#stopippon").prop('checked', true);
}

// Fullscreen

function request_full_screen(element) {
    // Supports most browsers and their versions.
    var requestMethod = element.requestFullScreen ||
	element.webkitRequestFullScreen ||
	element.mozRequestFullScreen ||
	element.msRequestFullScreen;

    if (requestMethod) { // Native full screen.
        requestMethod.call(element);
    } else if (typeof window.ActiveXObject !== "undefined") { // Older IE.
        var wscript = new ActiveXObject("WScript.Shell");
        if (wscript !== null) {
            wscript.SendKeys("{F11}");
        }
    }
}

$("#fullscreen").click(function() {
    var elem = document.getElementById("maintable");
    request_full_screen(elem);
});

$(window).bind("resize", function(){
    var w = $(window).width();
    var h = $(window).height();
    var canvas = document.getElementById("timercanvas");
    canvas_width  = window.innerWidth;
    canvas_height = window.innerHeight - menuspace;
    canvas.width  = canvas_width;
    canvas.height = canvas_height;
    canvas.x = 0;
    canvas.y = menuspace;
    expose();
});

read_configuration();
configure();

/* Action */

$(document).ready(function() {
    var canvas = document.getElementById("timercanvas");
    canvas_width  = window.innerWidth;
    canvas_height = window.innerHeight - menuspace;
    canvas.width  = canvas_width;
    canvas.height = canvas_height;
    canvas.x = 0;
    canvas.y = menuspace;

    $.get("timer-custom.txt", function(data) {
	$.each(data.split(/[\n\r]+/), function(index, line) {
	    var c = line.charCodeAt(0);
	    if (c >= 48 && c <= 57) {
		var args = line.split(/[ ]+/);
		set_label(args);
	    }
	});
	expose();
    });

    if (!("WebSocket" in window)) {
	message('<p>Oh no, you need a browser that supports WebSockets.'+
		'How about <a href="http://www.google.com/chrome">Google Chrome</a>?</p>');
    } else {
	//The user has WebSockets
	connect();
    }

    clocktmo = window.setInterval(timeout, 100);
});

$( "#menucontest" ).menu({
  position: { my: "left top", at: "right-5 top+5" }
});
$( "#menupreferences" ).menu({
  position: { my: "left top", at: "right-5 top+5" }
});
var menu1 = $("#menucontest").menu();
$(menu1).mouseleave(function () {
    menu1.menu('collapseAll');
});
var menu2 = $("#menupreferences").menu();
$(menu2).mouseleave(function () {
    menu2.menu('collapseAll');
});

$(document).keypress(function(e){
    //console.log("keypress which: "+e.which+" ctl="+e.ctrlKey);
});

$(document).keydown(function(e){
    //console.log("keypress keycode: "+e.keyCode+" ctl="+e.ctrlKey);
    clock_key(e.which || e.keyCode, (e.ctrlKey ? 4 : 0) + (e.shiftKey ? 1 : 0));
});

$(window).bind('mousewheel DOMMouseScroll', function(e){
    var up_down = e.originalEvent.wheelDelta > 0 || e.originalEvent.detail < 0;
    var offset_t = $(window).scrollTop();
    var offset_l = $(window).scrollLeft();

    var left = Math.round( (e.originalEvent.clientX - offset_l) );
    var top = Math.round( (e.originalEvent.clientY - offset_t) );

    var i;
    for (i = 0; i < labels.length; i++) {
	if (getpxhor(labels[i].x) < left &&
	    getpxhor(Number(labels[i].x) + Number(labels[i].w)) > left &&
	    getpxver(labels[i].y) < top &&
	    getpxver(Number(labels[i].y) + Number(labels[i].h)) > top) {
	    if (i == t_min || i == t_tsec || i == t_sec) {
		if (running) return;

		if (i == t_min) {
		    if (up_down && elap <= total - 60000) elap += 60000;
		    else if (!up_down && elap >= 60000) elap -= 60000;
		} else if (i == t_tsec) {
		    if (up_down && elap <= total - 10000) elap += 10000;
		    else if (!up_down && elap >= 10000) elap -= 10000;
		} else {
		    if (up_down) elap += 1000;
		    else elap -= 1000;
		}

		if (elap < 0) elap = 0;
		else if (elap > total) elap = total;

		update_display();
		return;
	    }

	    if (i == o_tsec || i == o_sec) {
		if (running) return;

		if (up_down) oElap -= 1000;
		else oElap += 1000;
		if (oElap < 0) oElap = 0;
		else if (oElap > ippon) oElap = ippon;
		oRunning = oElap > 0;
		return;
	    }

	    handle_click(i, up_down);
	}
    }
});

$("#timer").click(function (e){
    var offset_t = $(this).offset().top - $(window).scrollTop();
    var offset_l = $(this).offset().left - $(window).scrollLeft();

    var left = Math.round( (e.clientX - offset_l) );
    var top = Math.round( (e.clientY - offset_t) );
    var i;
    for (i = 0; i < labels.length; i++) {
	if (getpxhor(labels[i].x) < left &&
	    getpxhor(Number(labels[i].x) + Number(labels[i].w)) > left &&
	    getpxver(labels[i].y) < top &&
	    getpxver(Number(labels[i].y) + Number(labels[i].h)) > top) {
	    handle_click(i, e.shiftKey);
	    break;
	}
    }
});

$("#key0").click(function(){
    clock_key(GDK_0, 0);
})
$("#key1").click(function(){
    clock_key(GDK_1, 0);
})
$("#key2").click(function(){
    clock_key(GDK_2, 0);
})
$("#key3").click(function(){
    clock_key(GDK_3, 0);
})
$("#key4").click(function(){
    clock_key(GDK_4, 0);
})
$("#key5").click(function(){
    clock_key(GDK_5, 0);
})
$("#key9").click(function(){
    clock_key(GDK_9, 0);
})

$(".tsel").change(function() {
    for (var i = 1; i <= NUM_TATAMIS; i++) {
	var id = "sel"+i;
	if (id == this.id) {
	    $("#"+id).prop('checked', true);
	    tatami = i;
	} else $("#"+id).prop('checked', false);
    }
    configure();
});

$(".sndsel").change(function() {
    for (var i = 0; i <= 10; i++) {
	var id = "snd"+i;
	if (id == this.id) {
	    $("#"+id).prop('checked', true);
	    sound = i;
	} else $("#"+id).prop('checked', false);
    }
    configure();
});

$("#viewcomp").click(function(){
    display_comp_window();
});

$("#stopippon").change(configure);
$("#judogicontrol").change(configure);

$("#hikiwake").click(function() {
    voting_result(HIKIWAKE);
});

/*
$( "#stopippon" ).menu({
    select: function( event, ui ) { console.log("menuselect"); }
});
*/
read_configuration();
configure();
