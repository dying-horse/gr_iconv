Allgemeine Hinweise:

* Nutzer, die dieses lua-Modul nur installieren wollen, gehen dabei am
  besten so wie [hier](https://github.com/dying-horse/gr_rocks/#readme)
  beschrieben vor.
* Mehr Dokumentation gibt es, wenn man im Basis-Verzeichnis
  `luadoc .` aufruft.  Hierzu benötigt man das
  [luadoc](http://keplerproject.github.com/luadoc/)-System, das man sich
  mit Hilfe von [luarocks](http://luarocks.org/) beschaffen kann:
  `luarocks install luadoc`.


Dieser Modul muß zunächst auf folgende Weise aktiviert werden:

	require "gr_iconv"


Der Modul `gr_iconv` stellt i.w. die Klassen `iconv` und `iconv.source`
bereit.

Die Klasse `iconv` enthält den Zustand während der Zeichensatzumwandlung.
Der Programmierer benötigt diese Klasse, wenn er für bei der
Zeichenumwandlung auftretende Ausnahmen Fehlerbehandlungen realisieren möchte.

`iconv.source` ist eine Unterklasse der Klasse `source`, wie sie in einem
anderen, im
[Modul `gr_source`](https://github.com/dying-horse/gr_source#readme)
definiert wurde.  Objekte der Klasse `iconv.source` nehmen die
eigentliche Zeichenumwandlung vor.  Bei der Erzeugung eines Objektes
dieser Klasse durch `iconv.source:new()` muß ein
[`source`-Objekt](https://github.com/dying-horse/gr_source#readme)
angegeben werden, das die umzuwandelnden Zeichen produziert.  Wenigstens
einmal muß die Methode `iconv.source.setencoding` aufgerufen worden
sein, bevor die Zeichensatzumwandlung beginnen kann.  Ein Objekt der
Klasse `iconv` kann angegeben werden, wenn man Ausnahmen behandeln möchte.

Ein Beispiel:

	require "gr_iconv"
	require "gr_source"
	
	-- erzeugt Unterklasse `my_iconv` von `iconv`.
	my_iconv = {}
	setmetatable(my_iconv, { __index = iconv })
	
	-- Diese Unterklasse überlädt Fehlerbehandlungsroutinen
	-- `iconv.except_...`
	function my_iconv:except_invalid_mb()
	 print("ungültige Bytefolge")
	end
	
	function my_iconv:except_not_supported()
	 print("Zeichensatzumwandlung wird nicht unterstützt.")
	end
	
	function my_iconv:except_out_of_memory()
	 print("Der gesamte Speicher wurde verbraucht.")
	end
	
	function my_iconv:except_too_many_fd()
	 print("zuviele file descriptors.")
	end
	
	function my_iconv:except_too_many_fd_per_process()
	 print("zuviele file descriptors pro Prozeß.")
	end
	
	-- Ein Objekt der Klasse `iconv.source` wird mit iconv.source.new
	-- erzeugt.  Als Argument wird ein source-Objekt übergeben, das der
	-- Datei irgendeine.datei entspricht, die die umzuwandelnden
	-- Zeichen enthält.  Als optionaler Parameter mit dem Schlüssel
	-- iconvstate wird ein Objekt der oben definierten
	-- Klasse `my_iconv` angegeben, so daß die Ausnahmebehandlung
	-- wirksam wird.
	my_iconv_src = iconv.source:new(source.file:new("irgendeine.datei"),
	 { iconvstate = my_iconv:new() })
	
	-- Die Zeichensätze von und wohin `my_iconv_src` übersetzen
	-- soll, wird hier eingestellt:  Die Datei
	-- `irgendeine.datei` ist im ISO-8859-3-Zeichensatz verschlüsselt.
	my_iconv_src:setencoding("ISO-8859-3", "ISO-8859-1")
	
	-- `my_iconv_src` wird gebraucht.
	for c in my_iconv_src
	do   print(c)   -- druckt ISO-8859-1-Zeichen aus
	end


Low-level-Methoden der Klasse `iconv`
=====================================

Neben der Möglichkeit, Ausnahmebehandlungen zu definieren, enthält die
Klasse `iconv` einige Methoden, die unter normalen Umständen nicht direkt
benutzt werden.

`iconv.open(self, from, to)` erzeugt eine Struktur im `iconv`-Objekt
`self`, die bei der Umwandlung von Zeichen des in der Zeichenkette `from`
bestimmten Zeichensatzes in Zeichen des in der Zeichenkette `to`
bestimmten Zeichensatzes notwendig ist.  Kehrt die Methode
`iconv.open()` mit der Zeichenkette "pull" zurück, so muß danach
die Methode `iconv.pull()` und die Methode `iconv.go()` aufgerufen werden.
Ansonsten muß die Methode `iconv.push()` aufgerufen werden.

`iconv.push(self, chunk)` übergibt den Block `chunk` dem
Zeichenkettenumwandlungsmechanismus.  Nach Aufruf dieser Methode
muß die Methode `iconv.go()` aufgerufen werden, damit dieser Block
verarbeitet wird.

`iconv.pull(self)` liest die umgewandelten Daten.

`iconv.go()` wandelt mit `iconv.push()` bereitgestellte Zeichen um.
Die umgewandelten Zeichen können mit `iconv.pull()` gelesen werden.  Dabei
ist der Rückgabecode von `iconv.go()` zu berücksichtigen.  Diese
Rückgabecodes haben folgende Bedeutung:

* Zeichenkette "pull":  Die nächste Aktion muß `iconv.pull()` sein.
  Darauf muß wiederum die Methode `iconv.go()` aufgerufen werden.
* Zeichenkette "ok": Die Methode `iconv.pull()` kann genau einmal
  aufgerufen werden, muß aber nicht.  Stattdessen können mit der
  Methode `iconv.push()` neue Zeichen bereitgestellt werden.  Danach
  muß die Methode `iconv.go()` aufgerufen werden.
* Zeichenkette "push": Die Methode `iconv.push()` muß als
  nächstes aufgerufen werden, danach die Methode `iconv.go()`
* Zeichenkette "err": Der Fehlerfall.
