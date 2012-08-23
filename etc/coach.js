var competitors = new Array();
var coachid = 0;
var idFound = 0;
var judokas = new Array();
var numJudokas = 0;
var positions;
var matches = new Array();
var current_sheet = "";
var current_competitor = "";

function getPositions()
{
    var oRequest = new XMLHttpRequest();
    oRequest.open("GET", "c-winners.txt?" + new Date().getTime(), false);
    oRequest.send(null)

    if (oRequest.status == 200) {
	//console.log("pos="+oRequest.responseText.substr(0, 50));
	positions = oRequest.responseText;
    }
}

function getIds()
{
    var oRequest = new XMLHttpRequest();
    oRequest.open("GET", "c-ids.txt?" + new Date().getTime(), false);
    oRequest.send(null)
    if (oRequest.status == 200) {
	var r = oRequest.responseText;
	var lines = new Array();
	lines = r.split("\n");

	for (line in lines) {
	    //console.log("getIds i="+line+" line="+lines[line]);
	    var competitor = new Array();
	    competitor = lines[line].split("\t");
	    competitors[line] = competitor;
	}
    }
}

function getMatches()
{
    var oRequest = new XMLHttpRequest();
    oRequest.open("GET", "c-matches.txt?" + new Date().getTime(), false);
    oRequest.send(null)

    if (oRequest.status == 200) {
	var r = oRequest.responseText;
	var lines = new Array();
	lines = r.split("\n");

	for (line in lines) {
	    //console.log("getIds i="+line+" line="+lines[line]);
	    var info = new Array();
	    info = lines[line].split("\t");
	    matches[line] = info;
	}
    }
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
    var div = document.getElementById ("sheet1");
    
    var offset = {x : 0, y : 0};
    GetOffset (div, offset);
    
    var scrolled = {x : 0, y : 0};
    GetScrolled (div.parentNode, scrolled);
    
    var posX = offset.x - scrolled.x;
    var posY = offset.y - scrolled.y;
    
    if (posY > 40) {
	window.scrollBy(0, 40);
	setTimeout("scrolldown()", 50);
    }
}

function getSheet(name)
{
    current_sheet = name;

    if (name == "empty") {
	for (var i = 1; i <= 5; i++) {
	    document.getElementById("sheet" + i).innerHTML = "";
	}
	return;
    }

    var file = escape_utf8(name);

    for (var i = 1; i <= 5; i++) {
	var f;
	if (i == 1) {
	    f = file + ".png?" + new Date().getTime();
	} else {
	    f = file + "-" + (i-1) + ".png?" + new Date().getTime();
	}
	oRequest = new XMLHttpRequest();
	oRequest.open('HEAD', f, false);
	oRequest.send(null)
	if (oRequest.status == 200 || oRequest.status == 304) {
	    var src = "<img src=\""+f+"\" class=\"catimg\"";
	    if (i == 1) {
		src += " onload=\"scrolldown()\"";
	    }
	    src += " onclick=\"window.scrollTo(0,0)\"/>";
	    document.getElementById("sheet"+i).innerHTML = src;
	}
    }
	
    document.getElementById("competitor").focus();
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
    return (grams/1000).toString();
}

function getJudokaLine(judoka, sheet)
{
    var r = "";
    var c = getCompetitor(judoka);
    if (typeof(c) != "undefined") {
	r += "<tr onMouseOver=\"this.bgColor='silver';\" onMouseOut=\"this.bgColor='#FFFFFF';\"";
	if (sheet == false) {
	    r += " onclick=\"getSheet('"+c[7]+"')\"";
	}
	var kg = gramsToKg(c[6]);
	if (kg == 0) {
	    r += "><td>"+c[1]+"<td><b>"+c[0]+"</b><td style=\"background-color:orange\">"+kg+"<td>"+c[7]+"<td>";
	} else {
	    r += "><td>"+c[1]+"<td><b>"+c[0]+"</b><td>"+kg+"<td>"+c[7]+"<td>";
	}

	if (typeof(matches) != "undefined") {
	    if (matches[judoka][0] != "0") {
		r += matches[judoka][0];
	    }
	}

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
		r += "Finished, place " + (p & 0x0f).toString();
	    } else if (p == 0) {
		r += "Not drawn"
	    } else {
		if (typeof(matches) != "undefined") {
		    //r += "[m="+matches[judoka][0]+":"+matches[judoka][1]+"]";
		    if (matches[judoka][0] > 0 && matches[judoka][1] == "0") {
			r += "Match ongoing";
		    } else if (matches[judoka][0] > 0 && matches[judoka][1] != "-1") {
			r += "Match after "+matches[judoka][1]+" matches";
		    } else if ((matchStat & 16) && ((matchStat & 4) == 0)) {
			r += "Finished";
		    } else if (matchStat & 2) {
			r += "Started";
		    } else if (matchStat & 16) {
			r += "Drawing ready";
		    }
		}
	    }
	    //r += "[s=0x"+matchStat.toString(16)+"]";
	}
	r += "</tr>\n";

	if (sheet) {
	    getSheet(c[7]);
	}
    }
    return r;
}

function checkCompetitor()
{

    //document.getElementById("result").innerHTML=document.getElementById("competitor").value;
    var v = document.getElementById("competitor").value;
    if (v.length < 1) { 
	v = current_competitor;
	getSheet(current_sheet);
    } else {
	getSheet("empty");
    }
    if (v.length < 1) { return; }

    getFiles();
    current_competitor = v;
    checkCompetitor1(v);

    document.getElementById("competitor").value = "";
    document.getElementById("competitor").focus();
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
	    r += "<b>Coach:</b> "+c[1]+" "+c[0]+" ("+c[10]+")<br>";
	}
    }

    r += "<table id=\"judokas\" class=\"tablesorter\" >\n";
    r += "<thead><tr bgcolor='#c8d7e6'><th>Name<th>Surname<th>Weight<th>Category<th>Tatami<th>Status</tr></thead><tbody>";    

    if (numJudokas > 0) { // coach id
	for (var i = 0; i < numJudokas; i++) {
	    r += getJudokaLine(judokas[i], false);
	}

	r += "</tbody></table>\n";
	document.getElementById("result").innerHTML = r;
    } else if (idFound > 0) { // competitor
	r += getJudokaLine(idFound, true);
	r += "</tbody></table>\n";
	document.getElementById("result").innerHTML = r;
    } else { // no hit
	r += getJudokaLine(v, true);
	r += "</tbody></table>\n";
	document.getElementById("result").innerHTML = r;
    }

    $("#judokas").tablesorter({ 
        sortList: [[1,0]] 
    });
}

function doFocus()
{
    document.getElementById('competitor').focus();
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
    document.getElementById("competitor").value = "";
    document.getElementById("result").innerHTML = "";
    for (var i = 1; i <= 5; i++) {
	document.getElementById("sheet"+i).innerHTML = "";
    }
}

function initialize()
{
    getFiles();
    document.getElementById("competitor").value = "";
    document.getElementById("competitor").focus();
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