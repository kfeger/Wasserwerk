// Die Javascript-Datei für die settings.html

var EpochRente;
presentHost();


document.addEventListener("DOMContentLoaded", function(event) { 
getJSON(); 
//sendJSON(); 
});

function presentHost() {
	var host = window.location.hostname;
	console.log(host);
}


function getJSON() {	// Variablen aus EEPROM laden
  var xhttp = new XMLHttpRequest();

  xhttp.onreadystatechange = function() {
	if (this.readyState == 4 && this.status == 200) {
		var ESPObj = JSON.parse(this.responseText);
		document.getElementById("compile").innerHTML = ESPObj.compile;
		// Füllen der Zeilen der Pflanzen (0..5)
		for (i=0; i<6; i++) {
			document.getElementById("pflanzenName["+i+"]").value = ESPObj.pflanzenNamen[i];
			if (ESPObj.pumpenConfig[i] == 1)
				document.getElementById("pumpenConfig["+i+"]").checked = true;
			else
				document.getElementById("pumpenConfig["+i+"]").checked = false;
			document.getElementById("pumpeZeit["+i+"]").value = ESPObj.pumpeZeit[i];
			document.getElementById("pflanzenFeuchte["+i+"]").innerHTML = ESPObj.pflanzenFeuchte[i];
		}
		// Spezialbehandlung für Überlauf und Wassereimer (6,7)
		document.getElementById("pflanzenFeuchte[6]").innerHTML = ESPObj.pflanzenFeuchte[6];
		if (ESPObj.pflanzenFeuchte[6] > 20)
			document.getElementById("pumpenConfig[6]").innerHTML = "<img src=\"/uebergelaufen30.png\">";
		else
			document.getElementById("pumpenConfig[6]").innerHTML = "<img src=\"/trocken30.png\">";

		document.getElementById("pflanzenFeuchte[7]").innerHTML = ESPObj.pflanzenFeuchte[7];
		if (ESPObj.pflanzenFeuchte[7] > 20)
			document.getElementById("pumpenConfig[7]").innerHTML = "<img src=\"/eimerVoll30.png\">";
		else
			document.getElementById("pumpenConfig[7]").innerHTML = "<img src=\"/eimerLeer30.png\">";

	}
  };
  xhttp.open("GET", "readJSON", true);
  xhttp.send();
}

function testPumpe(Pumpe) {
	var xhttp = new XMLHttpRequest();
	if (this.readyState == 4 && this.status == 200) {
		console.log(this.responseText);
	}
	console.log(Pumpe);
	xhttp.open("GET", "TestPumpe?pumpe=" + Pumpe, true);
	xhttp.send();

}


function sendJSON() {
	var url;
	var xhttp = new XMLHttpRequest();
	url = window.location.hostname;
	var i;
	var sendObj = {
		pflanzeName: new Array,
		pumpeZeit: new Array,
		pumpenConfig: new Array,
	};
	
	for (i= 0; i < 6; i++) {
		sendObj.pflanzeName[i] = document.getElementById("pflanzenName["+i+"]").value;
		sendObj.pumpeZeit[i] = document.getElementById("pumpeZeit["+i+"]").value;
		sendObj.pumpenConfig[i] = document.getElementById("pumpenConfig["+i+"]").checked
		/*if (document.getElementById("pumpenConfig["+i+"]").checked == true)
			sendObj.pumpenConfig[i] = true;
		else
			sendObj.pumpenConfig[i] = false;*/
	}

	var sendDataJSON = JSON.stringify(sendObj);
	console.log(sendDataJSON);
	xhttp.onreadystatechange = function() {
	if (this.readyState == 4 && this.status == 200) {
		console.log(this.responseText);
		}
	else {
		console.log("Respone Error: " + this.responseText);
		}
	};
	xhttp.open("POST", "/getJSON", true);
	xhttp.setRequestHeader("Content-type", "application/text");
	xhttp.send(sendDataJSON);
}

function sendReboot() {
	var xhttp = new XMLHttpRequest();
	if (this.readyState == 4 && this.status == 200) {
		console.log(this.responseText);
	}
	console.log("Reboot Host");
	xhttp.open("GET", "hostReboot?reboot=" + 1, true);
	xhttp.send();
	
}

