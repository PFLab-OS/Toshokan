TOSHOKAN_VERSION:=v0.05a
HAKASE_SOURCE:=shared.h hakase.cc
FRIEND_SOURCE:=shared.h friend.cc int.S

$(shell docker run --rm -v $(CURDIR):$(CURDIR) livadk/toshokan_build_hakase:$(TOSHOKAN_VERSION) cp -r /usr/local/share/misc/toshokan $(CURDIR)/.misc)
include .misc/build_rules.mk
