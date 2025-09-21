Tester komponentów elektrycznych - miernik rezystyrów, kondensatorów oparty na mikroprocesorze ATmega328P
Tester mierzy pojemność wykorzystując metodę ładowania kondensatora przez znany rezystor i pomiar czasu
osiągnięcia określonego napięcia.
-------------------------------------------------------------------------------------------------------
ZASADY DZIAŁANIA

-Metoda pomiarowa: Uc = VCC * (1 - e**(-t/T))
C = -t/(680 * ln(1 - (Uc / 5000))))
-Auto-range automatycznie wybiera potrzebny rezystor 470 kOhm (dla małych kondensatorów)
lub 680 Ohm (dla dużych kondesatorów)

-------------------------------------------------------------------------------------------------------

PROCES POMIARU POJEMNOŚCI
-Rozładowanie kondensatorów
-Ładowanie przez znany rezystor
-Pomiar czasu i napięcia (do 400mV)
-Obliczanie pojemności z równania ładowania
-Filtracja wyników
------------------------------------------------------------------------------------------------------
PROCES POMIARU REZYSTANCJI
- wybór znanego rezystora w zależności od napięcia na nieznanym rezystorze
- obliczanie nieznanej rezystancji z dzielnika napięciowego

PROCES WYZNACZENIA CHARAKTERYSTYKI TRANZYSTORÓW I DIOD

***********Bootloader programming*********************
avrdude -p m328p -c arduino -P /dev/ttyUSB0 -b 115200 -U lfuse:w:0xFF:m -U hfuse:w:0xDE:m -U efuse:w:0x05:m
*****************************************************
flash: main.hex
	avrdude -p m328p -c arduino -P $(PORT) -b 115200 -U flash:w:$<