include ../desktop-shared/Makefile.sources

SOURCES_CXX += $(DESKTOP_SRC_DIR)/nfd/nfd_portal.cpp
CPPFLAGS += `pkg-config --cflags dbus-1`
CPPFLAGS += -DPREVENT_ROM_FOLDER_USAGE
LDFLAGS += `pkg-config --libs dbus-1`

include ../desktop-shared/Makefile.common
include ../desktop-shared/Makefile.install
