TOSHOKAN_VERSION:=v0.05a
HAKASE_SOURCE:=shared.h hakase.cc
FRIEND_SOURCE:=shared.h friend.cc

$(shell docker run --rm -v $(CURDIR):$(CURDIR) livadk/toshokan_tools:$(TOSHOKAN_VERSION) rsync -av /usr/local/share/misc/toshokan/ $(CURDIR)/.misc/ > /dev/null 2>&1)
include .misc/build_rules.mk
