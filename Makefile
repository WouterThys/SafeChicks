
SHELL=cmd
COPY=scp

ARDUINO=arduino-cli


FILE=concat.txt


download:
	${COPY} pi@raspberrypi.local:~/source/SerialMonitor/*.txt .\SensorLogs\

show:
	python.exe .\annelies.py .\SensorLogs\${FILE}
