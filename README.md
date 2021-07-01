# Wasserwerk
Dieses Projekt dient der Bewässerung von max. sechs Pflanzen. In jeder Pflanze wird die Bodenfeuchte in Prozent gemessen. Der Messwert hat wenig mit den tatsächlichen Werten zu tun.
Fest stehen die Eckwerte 0% (staubtrocken) und 100% (blankes Wasser). Die Steuerung erfolgt über ein Webseite, die das Projekt im lokalen Netz anbietet. Diese Webseite sollte NUR im lokalen Netz angeboten werden.

Die Kommunikation zwischen Webclient und Server ist JSON-basiert. Es wird eine Update-Funktion angeboten, die allerdings NICHT Internet-fähig ist, da die Updates vollkommen ungeschützt
übertragen werden.

Alle I/Os gehen über I2C mit Ausnahme des Thermometers. Hier wird OneWire verwendet.
