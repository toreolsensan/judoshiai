"use strict";

var webSocket;
var svgtemplate;

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

var NUM_TATAMIS = 10;
var NUM_POS     = 11;
var NUM_LOOKUP  = 8;

function ROUND_IS_FRENCH(_n) {
    return ((_n & ROUND_TYPE_MASK) == 0 ||
	    (_n & ROUND_TYPE_MASK) == ROUND_REPECHAGE);
}
function ROUND_TYPE(_n) { return (_n & ROUND_TYPE_MASK); }
function ROUND_NUMBER(_n) { return (_n & ROUND_MASK); }
function ROUND_TYPE_NUMBER(_n) { return (ROUND_TYPE(_n) | ROUND_NUMBER(_n)); }

var commver = 3;
var apptype = APPLICATION_TYPE_INFO;
var myaddr = 101;
var judoka_ix = 0;

var editcomp = [commver,MSG_EDIT_COMPETITOR,0,myaddr,
		0,0,"","",0,"","",0,0,1,"",0,"","",0,0,0,"","","",""];
var saved_msg;
var control = 0;
var disp_mode = 0;
var disp_mode_weight = 0;
var disp_mode_new = 1;
var disp_mode_edit = 2;

var screenwidth = window.innerWidth
    || document.documentElement.clientWidth
    || document.body.clientWidth;

var screenheight = window.innerHeight
    || document.documentElement.clientHeight
    || document.body.clientHeight;

var fontsize = 16;//screenheight/32;

var show_menu = true;
var tatami_order= [];

/* Initialize */

var match_list = [];
for (var i = 0; i < NUM_TATAMIS; i++) {
    var t = [];
    for (var j = 0; j < NUM_POS; j++) {
	t[j] = {
	    category:0,
	    number:0,
	    blue:0,
	    white:0,
	    flags:0,
	    round:0,
	    rest_end:0
	};
    }
    match_list[i] = t;
    tatami_order.push(i);
}

/*
  name_data[index] = { last, first, club }
*/
var name_data = [];

name_data[0] = {
    last:  "",
    first: "",
    club:  ""
};

var comp_queue = [];
var comptmo;

var show_tatami = [];
for (var i = 0; i < NUM_TATAMIS; i++)
    show_tatami[i] = false;
show_tatami[0] = true;
show_tatami[1] = true;
show_tatami[2] = true;

for (var i = 0; i < NUM_TATAMIS; i++)
    $("#sel"+(i+1)).prop('checked', show_tatami[i]);

var show_bracket = false;

/*
$("td").css("font-family", "Arial");
$("td").css("font-size", fontsize);
$(".fs2").css("font-size", 2*fontsize);
$(".fs4").css("font-size", 4*fontsize);
$("input").css("font-size", fontsize);
$("select").css("font-size", fontsize);
$("button").css("font-size", fontsize);
$(".center").css({"margin-left":"auto", "margin-right":"auto"});
$(".grey").css("background-color", "lightgrey");
$(".green").css("background-color", "lightgreen");
$(".red").css("backgroundColor", "red");
*/

/* Functions */

function num_tatamis() {
    var n = 0;
    for (var i = 0; i < NUM_TATAMIS; i++)
	if (show_tatami[i]) n++;
    return n;
}

function connect(){
    try {
	var url = window.location.href;
	var aa = url.lastIndexOf(':');
	var host = url.slice(7, aa);

	$(".hdr").css("background-color", "red");

	webSocket = new ReconnectingWebSocket('ws://' + host + ':2315/');

        webSocket.onopen = function() {
	    sendmsg([commver,MSG_DUMMY,0,myaddr,apptype,0]);
	    sendmsg([commver,MSG_ALL_REQ,0,myaddr]);
	    $(".hdr").css("background-color", "white");
	    get_translations();
	    get_comp_data();
        }

        webSocket.onmessage = function(msg){
	    parse_msg(msg.data);
        }

        webSocket.onclose = function(){
	    $(".hdr").css("background-color", "red");
        }
    } catch(exception) {
   	message('<p>Error: '+exception);
    }
}

function message(m) {
    console.log("Message:" + m);
}

function sendmsg(msg) {
    var data = JSON.stringify({"msg":msg});
    webSocket.send(data);
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
    if (msg[1] == MSG_NAME_INFO) {
	name_data[msg[4]] = {
	    last:  msg[5],
	    first: msg[6],
	    club:  msg[7]
	};

	for (var t = 0; t < NUM_TATAMIS; t++) {
	    if (!show_tatami[t])
		continue;
	    for (var p = 0; p < NUM_POS; p++) {
		var m = match_list[t][p];
		if (m.blue == msg[4] ||
		    m.white == msg[4] ||
		    m.cat == msg[4]) {
		    draw_match(t+1, p);
		}
	    }
	}
    } else if (msg[1] == MSG_MATCH_INFO) {
	handle_info(msg.slice(4, 13));
    } else if (msg[1] == MSG_11_MATCH_INFO) {
	for (var i = 0; i < 11; i++) {
	    handle_info(msg.slice(4+i*9, 13+i*9));
	}
    } else if (msg[1] == MSG_LANG) {
	$(".lang").each(function() {
	    if ($(this).text() == msg[4])
		$(this).html(msg[5]);
	});

	//$(".lang:contains("+msg[4]+")").html(msg[5]);
    }
}

function draw_match(tatami, pos) {
    var m = match_list[tatami-1][pos];
    var ndata;

    if (m.category) {
	ndata = name_data[m.category];
	if (ndata) {
	    $("#"+mk_id("cat", tatami, pos)).text(ndata.last +" #"+ m.number);
	} else {
	    $("#"+mk_id("cat", tatami, pos)).text("?");
	}
    }

    if (m.blue) {
	ndata = name_data[m.blue];
	if (ndata) {
	    $("#"+mk_id("bfirst", tatami, pos)).text(ndata.first);
	    $("#"+mk_id("blast", tatami, pos)).text(ndata.last);
	    $("#"+mk_id("bclub", tatami, pos)).text(ndata.club);
	} else {
	    $("#"+mk_id("blast", tatami, pos)).text("?");
	}
    }

    if (pos == 0)
	return;

    if (m.white) {
	ndata = name_data[m.white];
	if (ndata) {
	    $("#"+mk_id("wfirst", tatami, pos)).text(ndata.first);
	    $("#"+mk_id("wlast", tatami, pos)).text(ndata .last);
	    $("#"+mk_id("wclub", tatami, pos)).text(ndata.club);
	} else {
	    $("#"+mk_id("wlast", tatami, pos)).text("?");
	}
    }

    $("#"+mk_id("rnd", tatami, pos)).text(round_to_str(m.round));
}

var last_cat = 0;
var last_num = 0;

function get_bracket() {
    for (var i = 0; i < NUM_TATAMIS; i++) {
	if (show_tatami[i]) {
	    $("#bracketimage").html("<img src='web?op=5&t="+(i+1)+"&s=1'/>");
	    break;
	}
    }
}

function handle_info(msg) {
    match_list[msg[0]-1][msg[1]] = {
	    category: msg[2],
	    number:   msg[3],
	    blue:     msg[4],
	    white:    msg[5],
	    flags:    msg[6],
	    round:    msg[8],
	    rest_end: 0
    };

    if (!name_data[msg[2]]) comp_queue.push(msg[2]);
    if (!name_data[msg[4]]) comp_queue.push(msg[4]);
    if (!name_data[msg[5]]) comp_queue.push(msg[5]);

    if (is_active(msg[0]))
	draw_match(msg[0], msg[1]);

    if (show_bracket &&
	msg[1] == 1 &&
	(msg[2] != last_cat ||
	 msg[3] != last_num)) {
	last_cat = msg[2];
	last_num = msg[3];
	get_bracket();
    }
}

function get_comp_by_id(ix) {
    var msg = editcomp;
    judoka_ix = ix;
    msg[4] = EDIT_OP_GET_BY_ID;
    msg[17] = String(ix);
    sendmsg(msg);
}

function xlate(en) {
    var msg = [commver,MSG_LANG,0,myaddr,en,""];
    sendmsg(msg);
}

// translations

var words = [];
var wordix = 0;
var tmo;

function askone() {
    if (wordix >= words.length) {
	window.clearTimeout(tmo);
	return;
    }
    xlate(words[wordix]);
    wordix++;
}

function get_translations() {
    $(".lang").each(function() {
	words.push($(this).text());
	//xlate($(this).text());
    });
    tmo = window.setInterval(askone, 100);
}

// competitor req queue

function get_one_comp() {
    var c = comp_queue[0];
    if (c) {
	comp_queue.shift();
	sendmsg([commver,MSG_NAME_REQ,0,myaddr,c]);
    }
}

function get_comp_data() {
    comptmo = window.setInterval(get_one_comp, 100);
}

/* Action */

$(document).ready(function() {
    make_table();

    if (!("WebSocket" in window)) {
	message('<p>Oh no, you need a browser that supports WebSockets.'+
		'How about <a href="http://www.google.com/chrome">Google Chrome</a>?</p>');
    } else {
	//The user has WebSockets
	connect();
    }

});

var tablestr;

function is_active(t) {
    return show_tatami[t-1];
}

function mk_id(name, tatami, num) {
    return 'm' + name + "_t" + tatami + "_n" + num;
}

/* table looks */

function make_table() {
    tablestr = "";

    tablestr += "<table id='maintable'><tr><td id='brtd1'>";

    tablestr += "<table id='infotable'>";

    tablestr += "<tr>";
    for (var i = 0; i < NUM_TATAMIS; i++) {
	t = tatami_order[i];
	if (!is_active(t+1)) continue;
	tablestr += "<th class='hdr' colspan='2'>Tatami "+(t+1)+"</th>";
    }
    tablestr += "</tr><tr>";
    for (var i = 0; i < NUM_TATAMIS; i++) {
	t = tatami_order[i];
	if (!is_active(t+1)) continue;
	tablestr += "<td class='winner'><div class='lang'>Prev winner:</div></td>";
	tablestr += "<td class='winner'>&nbsp;</td>";
    }
    tablestr += "</tr><tr>";
    for (var i = 0; i < NUM_TATAMIS; i++) {
	t = tatami_order[i];
	if (!is_active(t+1)) continue;
	tablestr += "<td class='winner' id='"+mk_id("cat", t+1, 0)+"'>&nbsp;</td>";
	tablestr += "<td class='winner' id='"+mk_id("bfirst", t+1, 0)+"'>&nbsp;</td>";
    }
    tablestr += "</tr><tr>";
    for (var i = 0; i < NUM_TATAMIS; i++) {
	t = tatami_order[i];
	if (!is_active(t+1)) continue;
	tablestr += "<td class='winner'>&nbsp;</td>";
	tablestr += "<td class='winner' id='"+mk_id("blast", t+1, 0)+"'>&nbsp;</td>";
    }
    tablestr += "</tr><tr>";
    for (var i = 0; i < NUM_TATAMIS; i++) {
	t = tatami_order[i];
	if (!is_active(t+1)) continue;
	tablestr += "<td class='winner1'>&nbsp;</td>";
	tablestr += "<td class='winner1' id='"+mk_id("bclub", t+1, 0)+"'>&nbsp;</td>";
    }
    tablestr += "</tr>";

    for (var i = 1; i < NUM_POS; i++)
	make_match_rows(i);

    tablestr += "</table>";
    tablestr += "</td><td id='brtd2'><div id='bracketimage'></div></td></tr></table>";

    $("#info").html(tablestr);
    init_colors();

    if (show_bracket)
	get_bracket();
}

function make_match_rows(num) {
    tablestr += "<tr>";
    for (var i = 0; i < NUM_TATAMIS; i++) {
	t = tatami_order[i];
	if (!is_active(t+1)) continue;
	tablestr += "<td class='cat' id='"+mk_id("cat", t+1, num)+"'>&nbsp;</td>";
	tablestr += "<td class='rnd' id='"+mk_id("rnd", t+1, num)+"'>&nbsp;</td>";
    }
    tablestr += "</tr><tr>";
    for (var i = 0; i < NUM_TATAMIS; i++) {
	t = tatami_order[i];
	if (!is_active(t+1)) continue;
	tablestr += "<td class='bfirst' id='"+mk_id("bfirst", t+1, num)+"'>&nbsp;</td>";
	tablestr += "<td class='wfirst' id='"+mk_id("wfirst", t+1, num)+"'>&nbsp;</td>";
    }
    tablestr += "</tr><tr>";
    for (var i = 0; i < NUM_TATAMIS; i++) {
	t = tatami_order[i];
	if (!is_active(t+1)) continue;
	tablestr += "<td class='blast' id='"+mk_id("blast", t+1, num)+"'>&nbsp;</td>";
	tablestr += "<td class='wlast' id='"+mk_id("wlast", t+1, num)+"'>&nbsp;</td>";
    }
    tablestr += "</tr><tr>";
    for (var i = 0; i < NUM_TATAMIS; i++) {
	t = tatami_order[i];
	if (!is_active(t+1)) continue;
	tablestr += "<td class='bclub' id='"+mk_id("bclub", t+1, num)+"'>&nbsp;</td>";
	tablestr += "<td class='wclub' id='"+mk_id("wclub", t+1, num)+"'>&nbsp;</td>";
    }
    tablestr += "</tr>";
}

function init_colors() {
    $("#infotable").css("width", "100%");
    //$(".infotable").css("width", "100%");

    $("#infotable").css("border", "0");
    $("#infotable").css("border-collapse", "collapse");
    $(".winner").css("background-color", "yellow");
    $(".winner1").css("background-color", "yellow");
    $(".cat").css("background-color", "white");
    $(".rnd").css("background-color", "white");
    $(".bfirst").css("background-color", "white");
    $(".blast").css("background-color", "white");
    $(".bclub").css("background-color", "white");
    if ($("#redbg").prop('checked')) {
	$(".wfirst").css("background-color", "red");
	$(".wlast").css("background-color", "red");
	$(".wclub").css("background-color", "red");
    } else {
	$(".wfirst").css("background-color", "blue");
	$(".wlast").css("background-color", "blue");
	$(".wclub").css("background-color", "blue");
    }
    $(".wfirst").css("color", "white");
    $(".wlast").css("color", "white");
    $(".wclub").css("color", "white");

    $("#infotable td:nth-child(2n+1)").css("border-left-style", "solid");
    $("#infotable td:nth-child(2n+1)").css("border-left-width", "1px");
    $("#infotable tr:nth-child(4n+1)").css("border-bottom-style", "solid");
    $("#infotable tr:nth-child(4n+1)").css("border-bottom-width", "1px");

    if (show_bracket) {
	$("#brtd1").css("width", "25%");
	$("#brtd2").css("width", "75%");
    } else {
	$("#brtd1").css("width", "100%");
	$("#brtd2").css("width", "0%");
    }

    $("#infotable td").css("width", ""+(50/num_tatamis())+"%");

    $(".rb").css("border-right-style", "solid");
    $(".rb").css("border-right-width", "1px");
    $(".lb").css("border-left-style", "solid");
    $(".lb").css("border-left-width", "1px");
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

// Configure

function configure() {
    show_bracket = $("#bracket").prop('checked');
    var found = false;
    var tatamis = 0;

    setCookie("infobracket", show_bracket ? 1 : 0, 30);

    for (var i = 0; i < NUM_TATAMIS; i++) {
	var sel = $("#sel"+(i+1)).prop('checked');
	if (sel && !found) {
	    show_tatami[i] = true;
	    tatamis += 1 << i;
	    if (show_bracket) found = true;
	} else {
	    show_tatami[i] = false;
	    $("#sel"+(i+1)).prop('checked', false);
	}
    }
    setCookie("infotatamis", tatamis, 30);

    tatami_order = [];
    if ($("#mirror").prop('checked')) {
	for (var i = NUM_TATAMIS-1; i >= 0; i--)
	    tatami_order.push(i);
	setCookie("infomirror", 1, 30);
    } else {
	for (var i = 0; i < NUM_TATAMIS; i++)
	    tatami_order.push(i);
	setCookie("infomirror", 0, 30);
    }

    make_table();

    for (var t = 0; t < NUM_TATAMIS; t++) {
	if (!show_tatami[t])
	    continue;
	for (var p = 0; p < NUM_POS; p++)
		draw_match(t+1, p);
    }

    if ($("#redbg").prop('checked'))
	setCookie("inforedbg", 1, 30);
    else
	setCookie("inforedbg", 0, 30);
}

$("#bracket").change(configure);
$("#redbg").change(configure);
$("#mirror").change(configure);
$(".tsel").change(function() {
    if (show_bracket &&
	$("#" + this.id).prop('checked')) {
	for (var i = 0; i < NUM_TATAMIS; i++) {
	    var id = "sel" + (i+1);
	    if (id != this.id) $("#" + id).prop('checked', false);
	}
    }
    configure();
});

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
    var tatamis = getCookie("infotatamis");
    console.log("tatamis="+tatamis);
    if (tatamis != "") {
	for (var i = 0; i < NUM_TATAMIS; i++) {
	    $("#sel"+(i+1)).prop('checked', tatamis & (1 << i));
	}
    }

    if (getCookie("inforedbg") == "1")
	$("#redbg").prop('checked', true);

    if (getCookie("infomirror") == "1")
	$("#mirror").prop('checked', true);

    if (getCookie("infobracket") == "1")
	$("#bracket").prop('checked', true);
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
    $("#maintable").css("width", w + "px");
    $("#maintable").css("height", h + "px");
});

$( "#menu" ).menu({
  position: { my: "left top", at: "right-5 top+5" }
});
var menu1 = $("#menu").menu();
$(menu1).mouseleave(function () {
    menu1.menu('collapseAll');
});

read_configuration();
configure();
