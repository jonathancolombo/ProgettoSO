rmAllLog:
	rm ECU.log
	rm brake.log
	rm cameras.log
	rm radar.log
	rm steer.log
	rm throttle.log
	rm utility.data
	rm assist.log
	echo "Tutti i file di log generati sono stati cancellati."
rmAllMake: 
	rm frontwindshieldcamera
	rm forwardfacingradar
	rm surroundviewcamera
	rm parkassist
	rm steerbywire
	rm brakebywire
	rm throttlebycontrol
	rm ecu
	rm hmi
	echo "Tutti gli eseguibili sono stati rimossi."
allMake:	
	gcc frontwindshieldcamera.c -o frontwindshieldcamera
	gcc forwardfacingradar.c -o forwardfacingradar
	gcc surroundviewcamera.c -o surroundviewcamera
	gcc parkassist.c -o parkassist
	gcc steerbywire.c -o steerbywire
	gcc brakebywire.c -o brakebywire
	gcc throttlebycontrol.c -o throttlebycontrol
	gcc ecu.c -o ecu
	gcc hmi.c -o hmi
	gcc hmiInput.c -o hmiInput
	@echo "Tutti i file sono stati creati correttamente."