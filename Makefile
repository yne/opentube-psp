all:
	make -C core
#	make -C gui-graphic
	make -C gui-text
	make -C kernel
	make -C mp4
	make -C AVC
	make -C AAC
	make -C modplay
	echo done.
	