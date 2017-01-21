var competitors = new Array();
var coachid = 0;
var idFound = 0;
var judokas = new Array();
var numJudokas = 0;
var positions;
var matches = new Array();
var current_sheet = "";
var current_competitor = "";
var map = new Array();

function getPositions()
{
    $.get("c-winners.txt?" + new Date().getTime(), function(data, status) {
	if (status == "success") {
	    positions = data;
	}
    });
}

function getIds()
{
    $.get("c-ids.txt?" + new Date().getTime(), function(data, status) {
	if (status == "success") {
	    var lines = new Array();
	    lines = data.split("\n");

	    for (line in lines) {
		var competitor = new Array();
		competitor = lines[line].split("\t");
		competitors[line] = competitor;
	    }
	}
    });
}

function getMatches()
{
    $.get("c-matches.txt?" + new Date().getTime(), function(data, status) {
	if (status == "success") {
	    var lines = new Array();
	    lines = data.split("\n");

	    for (line in lines) {
		var info = new Array();
		info = lines[line].split("\t");
		matches[line] = info;
	    }
	}
    });
}

function GetOffset (object, offset) {
    if (!object)
        return;
    offset.x += object.offsetLeft;
    offset.y += object.offsetTop;
    
    GetOffset (object.offsetParent, offset);
}

function GetScrolled (object, scrolled) {
    if (!object)
        return;
    scrolled.x += object.scrollLeft;
    scrolled.y += object.scrollTop;
    
    if (object.tagName.toLowerCase () != "html") {
        GetScrolled (object.parentNode, scrolled);
    }
}

function scrolldown() {
    var div = $("#sheets")[0];
    
    var offset = {x : 0, y : 0};
    GetOffset (div, offset);
    
    var scrolled = {x : 0, y : 0};
    GetScrolled (div.parentNode, scrolled);
    
    var posX = offset.x - scrolled.x;
    var posY = offset.y - scrolled.y;
    
    $("html, body").animate({ scrollTop: posY }, 1000);
}

function imgLoaded(name, page, judoka) {
    if ($("#box"+page)[0]) // handled already
	return;

    var offset = {x : 0, y : 0};
    var div = $("#img"+page)[0];
    if (!div) alert("no img"+page);
    GetOffset(div, offset);
    var file = escape_utf8(name);
    var f;
    //console.log("imgLoaded: cat="+name+" page="+page+" judoka="+judoka+"; offset x="+offset.x+" y="+offset.y);

    if (page == 0) {
        f = file + ".map?" + new Date().getTime();
	scrolldown();
    } else {
        f = file + "-" + page + ".map?" + new Date().getTime();
    }

    $("#compbox").append("<div id=\"box"+page+"\"></div>"); // page handled tag

    $.get(f, function(data, status) {
	if (status == "success") {
            var lines = new Array();
            lines = data.split("\n");

            for (line in lines) {
		var map = new Array();
		map = lines[line].split(",");
		if (map[4] != judoka) {
                    continue;
		}
		var boxes = "<div style=\"display: inline-block; position:absolute;";
		boxes += "left:"+(offset.x+parseInt(map[0]))+"px;top:"+(offset.y+parseInt(map[1]) - 2)+"px;";
		boxes += "width:"+(parseInt(map[2])-parseInt(map[0]))+"px; height:"+(parseInt(map[3])-parseInt(map[1]))+"px;";
		boxes += "border:2px solid #f00;\">&nbsp</div>";
		$("#compbox").append(boxes);
            }

	    // next sheet
	    f = file + "-" + (page+1) + ".png?" + new Date().getTime();
	    $.ajax({
		type: "HEAD",
		async: true,
		url: f,
		success: function(result, status, xhr) {
		    //console.log("SUCCESS2: xhr="+xhr + " status="+status + " result="+result);
		    var src = "<img src=\"" + f + "\" class=\"catimg\" id=\"img"+(page+1)+"\" "+
			"onload=\"imgLoaded('"+name+"',"+(page+1)+","+judoka+")\"><br>";
		    $("#sheets").append(src);
		}
	    });
	} // success
    });
}

function getSheet(name, judoka) {
    current_sheet = name;

    $("#sheets").html("");
    $("#compbox").html("");

    if (name == "empty")
        return;

    //console.log("getSheet called by " + arguments.callee.caller.toString());

    var file = escape_utf8(name);

    // find the first png file
    var f = file + ".png?" + new Date().getTime();

    $.ajax({
        type: "HEAD",
        async: true,
        url: f,
        success: function(result, status, xhr) {
	    //console.log("SUCCESS: xhr="+xhr + " status="+status + " result="+result);
            $("#sheets").html("<img src=\"" + f + "\" class=\"catimg\" id=\"img0\" onload=\"imgLoaded('"+name+"',0,"+judoka+")\"><br>");
	    $("#competitor").focus();
        },
	error: function(xhr, status, error) {
	    //console.log("ERROR: xhr="+xhr + " status="+status + " error="+error);
	    // svg files
	    var src = "";
	    for (var i = 1; i <= 10; i++) {
		if (i == 1) {
		    f = file + ".svg?" + new Date().getTime();
		} else {
		    f = file + "-" + (i - 1) + ".svg?" + new Date().getTime();
		}
		oRequest = new XMLHttpRequest();
		oRequest.open('GET', f, false);
		oRequest.send(null)
		if (oRequest.status == 200 || oRequest.status == 304) {
		    src += oRequest.responseText.replace("cmpx", "cmp" + judoka) + "<br>";
		} else {
		    break;
		}
	    }

	    $("#sheets").html(src);
	    $("#competitor").focus();
	}
    });
}

function getCompetitor(ix)
{
    var oRequest = new XMLHttpRequest();
    //oRequest.timeout = 2000;
    oRequest.open("GET", "c-" + ix + ".txt?" + new Date().getTime(), false);
    oRequest.send(null);

    if (oRequest.status == 200) {
	return oRequest.responseText.split("\n");
    }
}

function getCategory(name)
{
    var file = "c-";

    file += escape_utf8(name);
    file += ".txt?" + new Date().getTime();

    var oRequest = new XMLHttpRequest();
    //oRequest.timeout = 2000;
    oRequest.open("GET", file, false);
    oRequest.send(null);

    if (oRequest.status == 200) {
	return oRequest.responseText.split("\n");
    }
}

function gramsToKg(grams)
{
    var s = (grams/1000).toString();
    var k = s.length;
    var i = s.indexOf(".");

    if (grams == 0) { return "0"; }
    if (i == -1) { s += ".00"; }
    else if (k == (i + 2)) { s += "0"; }

    return s;
}

function getJudokaLine(judoka, sheet)
{
    var r = "";
    var c = getCompetitor(judoka);
    if (typeof(c) != "undefined") {
	r += "<tr onMouseOver=\"this.bgColor='silver';\" onMouseOut=\"this.bgColor='#FFFFFF';\"";
	if (sheet == false) {
	    r += " onclick=\"getSheet('"+c[7]+"','"+judoka+"')\"";
	}
	// weight
	var kg = gramsToKg(c[6]);
	if (kg == 0) {
	    r += "><td>"+c[1]+"<td><b>"+c[0]+"</b><td style=\"background-color:orange\">"+kg+"<td>"+c[7]+"<td>";
	} else {
	    r += "><td>"+c[1]+"<td><b>"+c[0]+"</b><td>"+kg+"<td>"+c[7]+"<td>";
	    //           first         last           weight     category
	}

	// tatami
	if (typeof(matches) != "undefined") {
	    if (matches[judoka][0] != "0") {
		r += matches[judoka][0];
	    }
	}

	// status
	r += "<td>";

	var matchStat = 0;
	var cat = getCategory(c[7]);
	if (typeof(cat) != "undefined") {
	    matchStat = cat[7];
	}

	if (typeof(positions) != "undefined") {
	    var p = positions.charCodeAt(judoka);
	    p -= 0x20;
	    //r += "[p=0x"+p.toString(16)+"]";
	    if ((p & 0x0f) > 0) {
		r += txt_finished;
	    } else if (p == 0) {
		r += txt_not_drawn;
	    } else {
		if (typeof(matches) != "undefined") {
		    //r += "[m="+matches[judoka][0]+":"+matches[judoka][1]+"]";
		    if (matches[judoka][0] > 0 && matches[judoka][1] == "0") {
			r += txt_match_ongoing;
		    } else if (matches[judoka][0] > 0 && matches[judoka][1] != "-1") {
			r += txt_match_after_1 + " " + matches[judoka][1] + " " + txt_match_after_2;
		    } else if ((matchStat & 16) && ((matchStat & 4) == 0)) {
			r += txt_finished;
		    } else if (matchStat & 2) {
			r += txt_started;
		    } else if (matchStat & 16) {
			r += txt_drawing_ready;
		    }
		}
	    }
	    //r += "[s=0x"+matchStat.toString(16)+"]";
	    // position
	    r += "<td>";
	    if ((p & 0x0f) > 0) {
		r += (p & 0x0f).toString();
	    }
	}
	r += "</tr>\n";

	if (sheet) {
	    getSheet(c[7], judoka);
	}
    }
    return r;
}

function checkCompetitor()
{
    var v = $("#competitor").val();
    console.log("competitor="+v+" len="+v.length+" current="+current_competitor);
    if (v.length < 1) { 
	v = current_competitor;
	getSheet(current_sheet, v);
    } else {
	getSheet("empty", 0);
    }
    if (v.length < 1) { return; }

    getFiles();
    current_competitor = v;
    checkCompetitor1(v);

    $("#competitor").val("");
    $("#competitor").focus();
}

function checkCompetitor1(v)
{
    //console.log("ID='"+v+"'");

    idFound = 0;
    numJudokas = 0;

    for (i in competitors) {
	var comp = competitors[i];

	if (v == comp[0]) {
	    idFound = i;
	} 

	if (v == comp[1]) {
	    judokas[numJudokas] = i;
	    numJudokas++;
	}
    }

    var r = "";

    if (idFound > 0 && numJudokas > 0) {
	var c = getCompetitor(idFound);
	if (typeof(c) != "undefined") {
	    r += "<b>"+txt_coach+":</b> "+c[1]+" "+c[0]+" ("+c[10]+")<br>";
	}
    }

    r += "<table id=\"judokas\" class=\"tablesorter\" >\n";
    r += "<thead><tr bgcolor='#c8d7e6'><th>"+txt_firstname+"<th>"+txt_lastname+
	"<th>"+txt_weight+"<th>"+txt_category+"<th>Tatami<th>"+txt_status+"<th>"+txt_place+
	"</tr></thead><tbody>";    

    if (numJudokas > 0) { // coach id
	for (var i = 0; i < numJudokas; i++) {
	    r += getJudokaLine(judokas[i], false);
	}

	r += "</tbody></table>\n";
	$("#result").html(r);
    } else if (idFound > 0) { // competitor
	r += getJudokaLine(idFound, true);
	r += "</tbody></table>\n";
	$("#result").html(r);
    } else { // no hit
	r += getJudokaLine(v, true);
	r += "</tbody></table>\n";
	$("#result").html(r);
    }

    $("#judokas").tablesorter({ 
        sortList: [[1,0]] 
    });
}

function doFocus()
{
    $('#competitor').focus();
}

function getFiles()
{
    getIds();
    getPositions();
    getMatches();
}

function update()
{
    if (current_competitor.length > 0) {
	checkCompetitor1(current_competitor);
    }
/*
    if (current_sheet.length > 0) {
	getSheet(current_sheet);
    }
*/
}

function refresh() {
    checkCompetitor();
}

function clearx() {
    current_competitor = "";
    current_sheet = "empty";
    $("#competitor").val("");
    $("#result").html("");
    $("#sheets").html("");
    $("#compbox").html("");
}

function initialize()
{
    $("#coachhdr").html(txt_coach + txt_display);
    getFiles();
    $("#competitor").val("");
    $("#competitor").focus();
    setInterval(function () { doFocus() }, 1000);
    //setInterval(function () { getFiles() }, 5000);
    //setInterval(function () { update() }, 5000);
}

function log(msg) {
    setTimeout(function() {
        throw new Error(msg);
    }, 0);
}

function escape_utf8(data) 
{
    if (data == '' || data == null){
               return '';
    }
    data = data.toString();
    var buffer = '';
    
    for (var i = 0; i < data.length; i++) {
        var c = data.charCodeAt(i);
        var bs = new Array();
        if (c > 0x10000) {
            // 4 bytes
            bs[0] = 0xF0 | ((c & 0x1C0000) >>> 18);
            bs[1] = 0x80 | ((c & 0x3F000) >>> 12);
            bs[2] = 0x80 | ((c & 0xFC0) >>> 6);
            bs[3] = 0x80 | (c & 0x3F);
        } else if (c > 0x800) {
            // 3 bytes
            bs[0] = 0xE0 | ((c & 0xF000) >>> 12);
            bs[1] = 0x80 | ((c & 0xFC0) >>> 6);
            bs[2] = 0x80 | (c & 0x3F);
        } else if (c > 0x80) {
            // 2 bytes
            bs[0] = 0xC0 | ((c & 0x7C0) >>> 6);
            bs[1] = 0x80 | (c & 0x3F);
        } else {
            // 1 byte
            bs[0] = c;
        }

        for (var j = 0; j < bs.length; j++){
            var b = bs[j];
            var hex = nibble_to_hex((b & 0xF0) >>> 4) 
                + nibble_to_hex(b & 0x0F);
	    buffer += hex;
        }
    }

    return buffer;
}

function nibble_to_hex(nibble) 
{
        var chars = '0123456789abcdef';
        return chars.charAt(nibble);
}

function openCatWindow(cat, judoka) {
    top.catWinRef=window.open('','categorywindow',
			      +',menubar=0'
			      +',toolbar=0'
			      +',status=0'
			      +',scrollbars=1'
			      +',resizable=1');

    top.catWinRef.document.writeln(
	'<html><head><title>'+cat+'</title>'
	    + '<script type="text/javascript" src="jquery-1.10.2.min.js" charset="utf-8"></script>'
	    + '<script type="text/javascript" src="coach.js" charset="utf-8"></script></head>'
	    + '<body bgcolor=white onLoad="getSheet(\''+cat+'\','+judoka+')">\r\n'
	    + '<div id="sheets"></div>'
	    + '<div id="compbox"></div>'
	    + '<div id="competitor"></div>'
	    + '</body></html>'
    );
    top.catWinRef.document.close()
}

var tooltip=function(){
	var id = 'tt';
	var top = 3;
	var left = 3;
	var maxw = 300;
	var speed = 10;
	var timer = 20;
	var endalpha = 95;
	var alpha = 0;
	var tt,t,c,b,h;
	var ie = document.all ? true : false;
	return{
		show:function(v,w){
			if(tt == null){
				tt = document.createElement('div');
				tt.setAttribute('id',id);
				t = document.createElement('div');
				t.setAttribute('id',id + 'top');
				c = document.createElement('div');
				c.setAttribute('id',id + 'cont');
				b = document.createElement('div');
				b.setAttribute('id',id + 'bot');
				tt.appendChild(t);
				tt.appendChild(c);
				tt.appendChild(b);
				document.body.appendChild(tt);
				tt.style.opacity = 0;
				tt.style.filter = 'alpha(opacity=0)';
				document.onmousemove = this.pos;
			}
			tt.style.display = 'block';
			c.innerHTML = v;
			tt.style.width = w ? w + 'px' : 'auto';
			if(!w && ie){
				t.style.display = 'none';
				b.style.display = 'none';
				tt.style.width = tt.offsetWidth;
				t.style.display = 'block';
				b.style.display = 'block';
			}
			if(tt.offsetWidth > maxw){tt.style.width = maxw + 'px'}
			h = parseInt(tt.offsetHeight) + top;
			clearInterval(tt.timer);
			tt.timer = setInterval(function(){tooltip.fade(1)},timer);
		},
		pos:function(e){
			var u = ie ? event.clientY + document.documentElement.scrollTop : e.pageY;
			var l = ie ? event.clientX + document.documentElement.scrollLeft : e.pageX;
			tt.style.top = (u - h) + 'px';
			tt.style.left = (l + left) + 'px';
		},
		fade:function(d){
			var a = alpha;
			if((a != endalpha && d == 1) || (a != 0 && d == -1)){
				var i = speed;
				if(endalpha - a < speed && d == 1){
					i = endalpha - a;
				}else if(alpha < speed && d == -1){
					i = a;
				}
				alpha = a + (i * d);
				tt.style.opacity = alpha * .01;
				tt.style.filter = 'alpha(opacity=' + alpha + ')';
			}else{
				clearInterval(tt.timer);
				if(d == -1){tt.style.display = 'none'}
			}
		},
		hide:function(){
			clearInterval(tt.timer);
			tt.timer = setInterval(function(){tooltip.fade(-1)},timer);
		}
	};
}();
