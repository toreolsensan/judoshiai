"use strict";

var webSocket;
var scaleSocket;
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

var NUM_LOOKUP = 8;

var commver = 3;
var apptype = APPLICATION_TYPE_WEIGHT;
var myaddr = 100;
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

/* Initialize */

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
$("#helpon").show();
$("#helpoff").hide();
$("#helptxt0").hide();
$("#helptxt2").hide();

//update_display();
$('#id').focus();

/* Functions */

function connect() {
    try {
	var url = window.location.href;
	var aa = url.lastIndexOf(':');
	var host = url.slice(7, aa);

	$("#dispweight").css("background-color", "red");

	webSocket = new ReconnectingWebSocket('ws://' + host + ':2315/');

        webSocket.onopen = function() {
	    sendmsg([commver,7,0,myaddr,apptype,0]);
	    $("#dispweight").css("background-color", "white");
	    get_translations();
        }

        webSocket.onmessage = function(msg){
	    parse_msg(msg.data);
        }

        webSocket.onclose = function(){
	    $("#dispweight").css("background-color", "red");
        }
    } catch(exception) {
   	message('<p>Error: '+exception);
    }
}

function scale_connect() {
    try {
	var url = "ws://" + $("#ipaddr").val() + ":" + $("#ipport").val() + "/";

	$("#serial").css("background-color", "red");
	$("#scale").hide();

	scaleSocket = new ReconnectingWebSocket(url);

        scaleSocket.onopen = function() {
	    $("#serial").css("background-color", "white");
	    $("#downloadhelp").hide();
	    setCookie("ipaddr", $("#ipaddr").val(), 30);
	    setCookie("ipport", $("#ipport").val(), 30);
	    scale_params();
	    $("#scale").show();
        }

        scaleSocket.onmessage = function(msg){
	    parse_msg(msg.data);
        }

        scaleSocket.onclose = function(){
	    $("#serial").css("background-color", "red");
	    $("#scale").hide();
        }
    } catch(exception) {
   	message('<p>Error: '+exception);
    }
}

function message(m) {
    //$("#message").html(m);
}

function sendmsg(msg) {
    var data = JSON.stringify({"msg":msg});
    webSocket.send(data);
}

function sendscalemsg(msg) {
    var data = JSON.stringify({"msg":msg});
    scaleSocket.send(data);
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
    console.log(JSON.stringify(message, null));
    if (msg[1] == MSG_EDIT_COMPETITOR) {
	if (msg[5] != judoka_ix)
	    return;

	if (msg[4] == EDIT_OP_GET) {
	    console.log('edit');
	    saved_msg = msg;
	    $("#name1").text(msg[6] + ' ' + msg[7] + "  ");
	    $("#name2").text(msg[16] + '/'+msg[9] +
			     ' (' + msg[23] + '): ' + msg[10]);
	    $("#weight").val((msg[12]/1000.0).toFixed(1));
            $('#weight').focus();
	    judoka_ix = msg[5];
	    if (msg[15] & JUDOGI_OK) {
		$('#control').prop('selectedIndex', 1);
	    } else if (msg[15] & JUDOGI_NOK) {
		$('#control').prop('selectedIndex', 2);
	    } else {
		$('#control').prop('selectedIndex', 0);
	    }

	    if (disp_mode == 1) {
		update_display();
	    }
	} else if (msg[4] == EDIT_OP_CONFIRM) {
	    $("#confname").text(msg[7] + ' ' + msg[6]);
	    $("#confclub").text(msg[9] + '/'+msg[16]);
	    $("#confweight").text((msg[12]/1000.0).toFixed(1));
	    $("#confcat").text(msg[24]);
	    if (msg[24] != msg[10]) {
		$("#confcat").css("color", "red");
	    } else {
		$("#confcat").css("color", "green");
	    }
	}
    } else if (msg[1] == MSG_LOOKUP_COMP) {
	handle_lookup(msg);
    } else if (msg[1] == MSG_SCALE) {
	$("#scale").text("" + Math.floor(msg[4]/1000) + "." + Math.floor((msg[4]%1000)/10));
    } else if (msg[1] == MSG_LANG) {
	$(".lang").each(function() {
	    if ($(this).text() == msg[4])
		$(this).html(msg[5]);
	});

	//$(".lang:contains("+msg[4]+")").html(msg[5]);
    }
}

function get_comp_by_id(ix) {
    console.log('get_comp_by_id '+ix);
    var msg = editcomp;
    judoka_ix = ix;
    $("#confname").text('');
    $("#confclub").text('');
    $("#confweight").text('');
    $("#confcat").text('');

    msg[4] = EDIT_OP_GET_BY_ID;
    msg[17] = String(ix);
    sendmsg(msg);
}

function weight_grams(s)
{
    if (!s) return 0;
    return s*1000;
}

function show_comp_data(msg) {
    $("#n_last").val(msg[6]);
    $("#n_first").val(msg[7]);
    $("#n_yob").val(msg[8]);
    $('#n_belt').prop('selectedIndex', msg[11]);
    $("#n_club").val(msg[9]);
    $("#n_country").val(msg[16]);
    $("#n_regcat").val(msg[10]);
    $("#n_realcat").text(msg[14]);
    $("#n_weight").val(msg[12]/1000);
    $('#n_seeding').prop('selectedIndex', msg[18]);
    $("#n_clubseeding").val(msg[19]);
    $("#n_id").val(msg[17]);
    $("#n_coachid").val(msg[22]);

    if (msg[15] & GENDER_MALE) {
	$('#n_gender').prop('selectedIndex', 1);
    } else if (msg[15] & GENDER_FEMALE) {
	$('#n_gender').prop('selectedIndex', 2);
    } else {
	$('#n_gender').prop('selectedIndex', 0);
    }

    if (msg[15] & JUDOGI_OK) {
	$('#n_control').prop('selectedIndex', 1);
    } else if (msg[15] & JUDOGI_NOK) {
	$('#n_control').prop('selectedIndex', 2);
    } else {
	$('#n_control').prop('selectedIndex', 0);
    }

    $("#n_hansokumake").prop('checked', msg[15]&HANSOKUMAKE);
    $("#n_comment").val(msg[21]);
}

function clear_comp_data(msg) {
    $("#n_last").val('');
    $("#n_first").val('');
    $("#n_yob").val('');
    $('#n_belt').prop('selectedIndex', 0);
    $("#n_club").val('');
    $("#n_country").val('');
    $("#n_regcat").val('');
    $("#n_realcat").text('');
    $("#n_weight").val(0);
    $('#n_seeding').prop('selectedIndex', 0);
    $("#n_clubseeding").val('');
    $("#n_id").val('');
    $("#n_coachid").val('');
    $('#n_gender').prop('selectedIndex', 0);
    $('#n_control').prop('selectedIndex', 0);
    $("#n_hansokumake").prop('checked', false);
    $("#n_comment").val('');
}

function send_weight(who) {
    saved_msg[1] = MSG_EDIT_COMPETITOR;
    saved_msg[3] = myaddr;
    saved_msg[4] = EDIT_OP_SET_WEIGHT;
    saved_msg[12] = weight_grams($('#weight').val());

    saved_msg[15] &= ~(JUDOGI_OK | JUDOGI_NOK);
    switch ($('#control').prop('selectedIndex')) {
    case 1: saved_msg[15] |= JUDOGI_OK; break;
    case 2: saved_msg[15] |= JUDOGI_NOK; break;
    }

    console.log(JSON.stringify({ 'msg': saved_msg }, null));

    sendmsg(saved_msg);
    $('#weight').val('');
    $('#id').focus();
    if ($("#autoprint").prop('checked'))
	print_svg();
}

function handle_lookup(msg) {
    var i;
    var rows = "";
    console.log("handle lookup");
    for (i = 0; i < NUM_LOOKUP; i++) {
	if (msg[2*i+5] == 0) {
	    break;
	}
	rows += "<tr><td id='lookup_"+i+"' onclick='get_comp_by_id("+msg[2*i+5]+")' >" +
	    "<a href='#'>" + msg[2*i+6] + "</a>" +
	    "</td></tr>";
    }
    $("#lookup").html(rows);
}

function print_elem(elem) {
    popup($(elem).html());
}

function popup(data) {
    var mywindow = window.open('', 'my div', 'height=400,width=600');

    mywindow.document.write('<html><head><title>my div</title>');
    mywindow.document.write('</head><body >');
    mywindow.document.write(data);
    mywindow.document.write('</body></html>');

    mywindow.print();
    mywindow.close();

    return true;
}

function print_svg() {
    var svg = svgtemplate;
    svg = svg.replace(/%c/g, "");
    svg = svg.replace(/-first/g, saved_msg[7]);
    svg = svg.replace(/-last/g, saved_msg[6]);
    svg = svg.replace(/-club/g, saved_msg[9]);
    svg = svg.replace(/-country/g, saved_msg[16]);
    svg = svg.replace(/-weight/g, saved_msg[12]/1000);
    svg = svg.replace(/-s/g, " ");
    svg = svg.replace(/%d/g, new Date);
    var mywindow = window.open('', 'my div', 'height=400,width=600');
    mywindow.document.write('<html><head><title>my div</title>');
    mywindow.document.write('</head><body >');
    mywindow.document.write(svg);
    mywindow.document.write('</body></html>');

    mywindow.print();
    mywindow.close();

    return true;
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

/* Action */

$(document).ready(function() {
    read_configuration();

    if (!("WebSocket" in window)) {
	message('<p>Oh no, you need a browser that supports WebSockets.'+
		'How about <a href="http://www.google.com/chrome">Google Chrome</a>?</p>');
    } else {
	//The user has WebSockets
	connect();
	scale_connect();
    }

    $("#n_weight").val(0);

    $.get("weight-label.svg", function(data) {
	svgtemplate = data;
    });

    $(function () { $('#tabs').tabs({
	activate: function (event, ui) {
            disp_mode = $("#tabs").tabs("option", "active");
	},
	active: 0
    })});
});

// header buttons

$("#setweight").click(function() {
    disp_mode = disp_mode_weight;
    update_display();
});
$("#editcomp").click(function() {
    disp_mode = disp_mode_edit;
    saved_msg = editcomp;
    update_display();
});

// weight

$('#id').change(function() {
    get_comp_by_id(this.value);
    this.value = '';
});

$('#weight').keypress(function(event) {
    var key = event.keyCode;
    if (key != '13') return;
    send_weight();
});

$('#submit').click(send_weight);

$("#print").click(function() {
    print_svg();
});

// new competitor

$("#n_submit").click(function() {
    var msg = editcomp;

    msg[4] = EDIT_OP_SET;
    if (saved_msg.length > 5) msg[5] = saved_msg[5];
    else msg[5] = 0;
    msg[13] = 1;

    msg[6] = $("#n_last").val();
    msg[7] = $("#n_first").val();
    msg[8] = Number($("#n_yob").val());
    msg[11] = Number($('#n_belt').prop('selectedIndex'));
    msg[9] = $("#n_club").val();
    msg[16] = $("#n_country").val();
    msg[10] = $("#n_regcat").val();
    msg[14] = $("#n_realcat").text();
    msg[12] = Number(weight_grams($("#n_weight").val()));
    msg[18] = Number($('#n_seeding').prop('selectedIndex'));
    msg[19] = $("#n_clubseeding").val();
    msg[17] = String($("#n_id").val());
    msg[22] = $("#n_coachid").val();
    msg[15] = 0;

    switch ($('#n_gender').prop('selectedIndex')) {
    case 1: msg[15] |= GENDER_MALE; break;
    case 2: msg[15] |= GENDER_FEMALE; break;
    }

    switch ($('#n_control').prop('selectedIndex')) {
    case 1: msg[15] |= JUDOGI_OK; break;
    case 2: msg[15] |= JUDOGI_NOK; break;
    }

    if ($("#n_hansokumake").prop('checked'))
	msg[15] |= HANSOKUMAKE;

    msg[21] = $("#n_comment").val();

    console.log(JSON.stringify({ 'msg': msg }, null));

    sendmsg(msg);
    clear_comp_data();
});

// edit competitor

$('#e_id').change(function() {
    get_comp_by_id(this.value);
    this.value = '';
});

$('#e_name').keypress(function(event) {
    var key = event.keyCode;
    if (key != '13') return;

    var msg = [commver,MSG_LOOKUP_COMP,0,myaddr,
	       this.value];
    var i;
    for (i = 0; i < NUM_LOOKUP; i++) {
	msg.push(0);
	msg.push("");
    }
    sendmsg(msg);
});

$("#helpon").click(function() {
    $("#helpon").hide();
    $("#helpoff").show();
    if (disp_mode == 0)
	$("#helptxt0").show("fade", 1000);
    else if (disp_mode == 2)
	$("#helptxt2").show("fade", 1000);
});
$("#helpoff").click(function() {
    $("#helpoff").hide();
    $("#helptxt0").hide("fade", 1000);
    $("#helptxt2").hide("fade", 1000);
    $("#helpon").show();
});

function update_display() {
    console.log("activate "+disp_mode);
    try {
	$('#tabs').tabs('option', 'active', disp_mode);
    } catch(e) {
    }

    switch (disp_mode) {
    case 0:
	break;
    case 1:
	show_comp_data(saved_msg);
	break;
    case 2:
	break;
    }
}

function scale_params() {
    var config = "type="+$("#stype").val()+
	";baud="+$("#sbaud").val()+
	";dev="+$("#sdevice").val();
    var msg = [commver,MSG_SCALE,0,myaddr,0,config];
    console.log("MSG="+msg);
    sendscalemsg(msg);

    setCookie("stype", $("#stype").prop('selectedIndex'), 30);
    setCookie("sbaud", $("#sbaud").prop('selectedIndex'), 30);
    setCookie("sdevice", $("#sdevice").val(), 30);
}

$("#newcomp").click(function() {
    saved_msg = [];
    clear_comp_data();
    update_display();
});

$("#scale").click(function() {
    $("#weight").val($("#scale").text());
});

$("#ipaddr").change(scale_connect);
$("#ipport").change(scale_connect);

$("#stype").change(scale_params);
$("#sbaud").change(scale_params);
$("#sdevice").change(scale_params);

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
    var ipaddr = getCookie("ipaddr");
    var ipport = getCookie("ipport");

    if (ipaddr != "") $("#ipaddr").val(ipaddr);
    if (ipport != "") $("#ipport").val(ipport);

    $("#stype").prop('selectedIndex', getCookie("stype"));
    $("#sbaud").prop('selectedIndex', getCookie("sbaud"));
    $("#sdevice").val(getCookie("sdevice"));
}

/*
$("#sbaud").val(getCookie("sbaud")).selectmenu('refresh');

$("#stype").selectmenu({
    width: "100%"
});
$("#sbaud").selectmenu({
    width: "100%"
});
*/
