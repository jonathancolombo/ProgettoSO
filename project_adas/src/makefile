deleteAllBin:
	rm hmi || frontwindshieldcamera || forwardingfacingradar || surroundviewcamera
	|| surround-view-camera || park-assist || parkassist || forwarding-facing-camera
	|| front-windshield-camera || steerbywire || steer-by-wire || brakebywire || brake-by-wire
hmiMake:
	rm hmi
	gcc hmi.c -o hmi
	clear
	echo "hmi è stato creato correttamente"
frontwindshieldscameraMake: 
	rm frontwindshieldcamera || front-windshield-camera
	gcc frontwindshieldcamera.c -o front-windshield-camera
	clear
	@echo "frontwindshieldcamera è stato creato correttamente."

forwardingfacingradarMake:
	rm forwardingfacingradar || forwarding-facing-camera
	gcc forwardingfacingradar.c -o forwarding-facing-camera
	clear
	@echo "forwardingfacingradar è stato creato correttamente."
surroundviewcameraMake:
	rm surround-view-camera || surroundviewcamera
	gcc surroundviewcamera.c -o surroundviewcamera
	clear
	@echo "svc è stato creato correttamente."
parkassistMake: 
	rm park-assist || paskassist
	gcc parkassist.c -o parkassist
	clear
	@echo "pa è stato creato correttamente."

allMake:
	gcc frontwindshieldcamera.c -o frontwindshieldcamera
	gcc forwardingfacingradar.c -o forwardingfacingradar
	gcc surroundviewcamera.c -o surroundviewcamera
	gcc parkassist.c -o parkassist
	gcc steerbywire.c -o steerbywire
	gcc brakebywire.c -o brakebywire
	gcc throttlebycontrol.c -o throttlebycontrol
	gcc ecu.c -o ecu
	gcc hmi.c -o hmi
	@echo "Tutti i file sono stati creati correttamente."