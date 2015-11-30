#!/usr/bin/make -f


all:
	$(MAKE) -j4 -C cpp/build/linux

clean:
	$(MAKE) -C cpp/build/linux clean

install:
	install -d $(DESTDIR)/etc/openzwave
	install -d $(DESTDIR)/usr/include/openzwave
	install -d $(DESTDIR)/usr/include/openzwave/command_classes
	install -d $(DESTDIR)/usr/include/openzwave/value_classes
	install -d $(DESTDIR)/usr/include/openzwave/platform
	install -d $(DESTDIR)/usr/include/openzwave/platform/unix
	install -d $(DESTDIR)/usr/share/openzwave
	install -d $(DESTDIR)/usr/lib
	cp -r config $(DESTDIR)/usr/share/openzwave
	install cpp/lib/linux/libopenzwave.so.1.0 $(DESTDIR)/usr/lib
	install cpp/lib/linux/libopenzwave.a $(DESTDIR)/usr/lib
	cp -r cpp/src/*.h $(DESTDIR)/usr/include/openzwave
	cp -r cpp/src/value_classes/*.h $(DESTDIR)/usr/include/openzwave/value_classes
	cp -r cpp/src/command_classes/*.h $(DESTDIR)/usr/include/openzwave/command_classes
	cp -r cpp/src/platform/*.h $(DESTDIR)/usr/include/openzwave/platform
	cp -r cpp/src/platform/unix/*.h $(DESTDIR)/usr/include/openzwave/platform/unix
