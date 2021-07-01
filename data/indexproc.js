// Die Javascript-Datei für die index.html

var HeuteTag, HeuteMonat, HeuteJahr;
var JetztMinute, JetztStunde;

setInterval(function() {
  // Call a function repetatively with 1 Second interval
  getJSON();
}, 1000); //1000ms update rate

function getJSON() {
  var xhttp = new XMLHttpRequest();
  var vorname, nachname, foermlich, geschlecht;
  xhttp.onreadystatechange = function() {
	if (this.readyState == 4 && this.status == 200) {
		var ESPObj = JSON.parse(this.responseText);
		document.getElementById("wochentag").innerHTML = ESPObj.wochentag;
		document.getElementById("tag").innerHTML = ESPObj.tag;
		document.getElementById("uhrzeit").innerHTML = ESPObj.uhrzeit;
		document.getElementById("compile").innerHTML = ESPObj.compile;
		document.getElementById("temperatur").innerHTML = ESPObj.temperatur;
		document.getElementById("vcc").innerHTML = ESPObj.vcc;
		// Füllen der Zeilen der Pflanzen (0..5)
		for (i=0; i<6; i++) {
			document.getElementById("pflanzenName["+i+"]").innerHTML = ESPObj.pflanzenNamen[i];
			if (ESPObj.pumpenConfig[i] == true)
				document.getElementById("pumpenConfig["+i+"]").innerHTML = "<img src=\"/Pumpe0-30.png\">";
			else
				document.getElementById("pumpenConfig["+i+"]").innerHTML = "<img src=\"/Pumpe0Keine-30.png\">";
				
			if (ESPObj.pumpeStatus[i] == true)
				document.getElementById("runningPump["+i+"]").innerHTML = "<img src=\"/LED-G.png\">";
			else
				document.getElementById("runningPump["+i+"]").innerHTML = "<img src=\"/LED-R.png\">";
				
			document.getElementById("pumpeZeit["+i+"]").innerHTML = ESPObj.pumpeZeit[i];
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

function runPumpe(Pumpe) {
	var xhttp = new XMLHttpRequest();
	if (this.readyState == 4 && this.status == 200) {
		console.log(this.responseText);
	}
	console.log(Pumpe);
	xhttp.open("GET", "runPumpe?pumpe=" + Pumpe, true);
	xhttp.send();
}
