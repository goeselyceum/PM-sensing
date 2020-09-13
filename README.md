# PM-sensing
Hier vind je de meest recente bestanden voor de fijnstof sensor van Het Goese Lyceum.

Gebruikte hardware:
- Arduino Uno rev3
- SD datalogger shield met ingebouwde DS1307 RTC (real-time clock)
- BME280 sensor voor temperatuur, luchtdruk en luchtvochtigheid (werkt met sensoren van Adafruit, Sparkfun of GBMEP (laatse via Ali-express, Banggood e.d.)
- Sensirion SPS30 sensor voor fijnstof

De .dxf bestanden zijn voor het lasersnijden van het chassis voor de arduino en sensoren. Deze vind je in de map chassis.

PMlog bevat een voorbeeld van meetgevens. LET OP het is een tekstbestand. Het decimaalteken is een punt(.) Eerst vervangen door een komma voordat je importeert in Excel. Na het ophalen van de externe gegevens alsnog de betreffende kolommen het kenmerk getal meegegeven.
Het veldscheidingsteken is een puntkomma(;).

Functionaliteit:
  - meet fijnstof in PM2.5 en PM10
  - meet temperatuur en luchtvochtigheid
  - slaat iedere minuut deze gegevens op in een "," gescheiden .txt bestand
  - toont debug info via de seriele monitor
  - auto-cleaning van de SPS30 ventilator instalebaar via software
