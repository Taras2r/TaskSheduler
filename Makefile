# -*- Makefile -*-
#https://www.youtube.com/watch?v=GExnnTaBELk&ab_channel=BarryBrown

#target: dependecies
#	action

#target:
#	action

#-g --gen-debug          generate debugging information
#-Wall             		 --all-warnings
#-mmcu=MCU               Select the target MCU.
# Optimization level, can be [0, 1, 2, 3, s].
#     0 = turn off optimization. s = optimize for size.
#     (Note: 3 is not always the best optimization level. See avr-libc FAQ.)


FILE = TaskSheduler
MCU = atmega8
OPT = s
F_CPU = 16000000
PROGRAMMER = avrispmkII

EEPROM_READ_FILE = "f:/CVSSandbox/AVR/Projects/TaskSheduler/eeprom"
FLASH_READ_PATH = "f:/CVSSandbox/AVR/Projects/TaskSheduler/flash"

$(FILE).hex: $(FILE).map
	@printf '\n-----avr-libc specific commands-----\n'
	avr-objcopy -j .text -j .data -O ihex $(FILE).elf $(FILE).hex
	avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex $(FILE).elf $(FILE)_eeprom.hex
	@printf '\n-----Memory info command-----\n'
	avr-size -C --mcu=atmega8 -t $(FILE).elf
	@echo Next commands require programmer and target connection:
	@printf '\t ---------Reading Commands--------- \n'
	@printf '\t info       - fuses, lock and clibration bits overview \n'
	@printf '\t rfuse      - fuse reading \n'
	@printf '\t rcalibrbit - calibration bits readding \n'
	@printf '\t rlockbits  - lock bits reading \n'
	@printf '\t rflashf    - flash memmory reading \n'
	@printf '\t reepromf   - eeprom memmory reading \n\n'
	@printf '\t NOTE: Fuse Calculator can be found by link: \n'
	@printf '\t https://www.engbedded.com/fusecalc/  \n\n'
	
	@printf '\t ---------Writing Commands--------- \n'
	@printf '\t wfuse [L_F H_F E_F] - fuse writing [values to be written]\n'
	@printf '\t wcalibrbit - calibration bits writing \n'
	@printf '\t wlockbits  - lock bits writing \n'
	@printf '\t wflashf    - flash memmory writing \n'
	@printf '\t weepromf   - eeprom memmory writing \n\n'

$(FILE).map: $(FILE).lst
	@printf '\n-----Linker Commands-----\n'
	avr-gcc -g -mmcu=$(MCU) -Wl,-Map,$(FILE).map -o $(FILE).elf $(FILE).o

$(FILE).lst: $(FILE).elf
	avr-objdump -h -S $(FILE).elf > $(FILE).lst

$(FILE).elf: $(FILE).o
	avr-gcc -g -O$(OPT) -mmcu=$(MCU) -o $(FILE).elf $(FILE).o 

$(FILE).o: $(FILE).c
	@printf '\n-----Commpiler Commands-----\n'
	avr-gcc -g -O$(OPT) -mmcu=$(MCU) -c $(FILE).c

clean:
	rm -f *.o *.out *.elf *.lst *.map

#######################AVRDUDE READ#######################
info:
	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) -U lfuse:r:-:h -U hfuse:r:-:h \
	-U efuse:r:-:h -U lock:r:-:h -U calibration:r:-:h -v

rfuse:
	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) -U lfuse:r:-:h -U hfuse:r:-:h \
	-U efuse:r:-:h

#readff:
#	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) \
#	-U lfuse:r:"f:/CVSSandbox/AVR/Projects/MyMake/fusel":h \
#	-U hfuse:r:"f:/CVSSandbox/AVR/Projects/MyMake/fuseh":h \
#	-U efuse:r:"f:/CVSSandbox/AVR/Projects/MyMake/fusee":h

rcalibrbits:
	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) -U calibration:r:-:h

rlockbits:
	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) -U lock:r:-:h
	 
reeprom:
	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) -U eeprom:r:-:h
	
reepromf:
	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) -U \
	eeprom:r:$(EEPROM_READ_FILE):h
	
rflashf:
	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) -U \
	flash:r:$(FLASH_READ_FILE):h

#######################AVRDUDE WRITE#######################
L_F = 0
H_F = 0
E_F = 

wfuse:
ifeq (,$(filter 0,$(L_F) $(H_F)))
	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) -U lfuse:w:$(L_F):m \
	-U hfuse:w:$(H_F):m -U efuse:w:$(E_F):m
else
	@echo Enter L_F=0xvalue and H_F=0xvalue E_F=0xvalue just after wfuse command
endif


		
wcalibrbits:
	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) -U calibration:r:-:h

wlockbits:
	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) -U lock:r:-:h
	 
weeprom:
	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) -U eeprom:r:-:h
	
weepromf:
	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) -U \
	eeprom:w:$(FILE)_eeprom.hex
	
wflashf:
	avrdude -p $(MCU) -P usb -c $(PROGRAMMER) -U \
	flash:w:$(FILE).hex
	
#@echo Fuse Calculator https://www.engbedded.com/fusecalc/