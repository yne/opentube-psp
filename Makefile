all:
	make -C core
	make -C gui
	make -C kernel
	make -C mp4
	make -C AVC
	make -C AAC
	echo done.
	