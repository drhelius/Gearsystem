include ../desktop-shared/Makefile.sources

SOURCES_CXX += $(DESKTOP_SRC_DIR)/nfd/nfd_portal.cpp
CPPFLAGS += `pkg-config --cflags dbus-1`
LDFLAGS += `pkg-config --libs dbus-1`

include ../desktop-shared/Makefile.common
include ../desktop-shared/Makefile.install
