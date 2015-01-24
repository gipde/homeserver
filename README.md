homeserver
==========

Travis: https://travis-ci.org/gipde/homeserver

Über den AVR-Net IO soll alles gesteuert werden

* AVR-Net IO
	* Temperatursensoren DS18S20 für Heizung (1 Pin pro OW)
	* Durchfluss Sensor für Umwälzpumpe
	* Stromzähler via Optokoppler
	* Wasserzähler via Optokoppler
	* Schaltung für Umwälzpumpe
	* Home-RF Funkmodul
	* TCPIP Stack
	* Ethernet Treiber (5 Pins SPI bereits verdrahtet)
	* Scheduler
	* Matrix-Display (6 Pins)
	* Datenpersistierung auf SD-Karte
	* Webserver
	* Sensorüberwachung via Heartbeat
	* Shell Zugang
	* Zeitsynchronisation
	* Batterieüberwachung der externen Geräte

* Räume
	* Home-RF
	* PIR Sensoren
	* Lichtsensoren
	* Temperatoru-Sensoren DS18S20 für Raumtemperatur
	* Feuchte-Sensoren für Raumfeuchte -> sind eher Teuer SHT1x HS1101
	* Fensterkontakte
	* Batteriebetrieben
	* Zeitsynchronisation

* Zugang
	* RFID
	* Kamera
	* Garagentor detektor

* Aussen
	* Wasserstandsmeldung in der Zisterne
	* Temperatur
	* Feuhte
	* Luftdruck
	* Helligkeit
	* Wind
	* Batteriebetrieben
	
* Bienen-Stand
	* Wägemodul
	* GPRS Datenübertragung direkt ins Internet
	* TCPIP
	* Zeit Sync DCF77
	* Dyndns o.ä.
	* Kamera
	* Temperatur
	* Feuchte
	* Helligkeit
	* Optokoppler am Flugloch zur Überwachung des Flugbetriebes
	* Batteriebetrieben
	* Verschlüsselung
