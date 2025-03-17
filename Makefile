SHELL=cmd
ARDUINO=arduino-cli


sketch:
	cd Arduino && $(ARDUINO) compile --fqbn arduino:avr:uno SafeChicks

upload: sketch
	cd Arduino && $(ARDUINO) upload -p COM4 --fqbn arduino:avr:uno SafeChicks

list:
	$(ARDUINO) board list