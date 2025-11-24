ASSETS := title gameover

ASSET_DIR := images
DATA_DIR  := data
BUILD_DIR := tools

CONVERT := convert
CONVERT_ARGS := -resize 128x64\! -threshold 50% -depth 1

CONVERT_TO_BIN := $(BUILD_DIR)/convert_to_bin.py

# uFBT output location
FAP_OUTPUT := $(HOME)/.ufbt/build/flipperhack.fap

# your chosen distribution folder
DIST_DIR := dist
DIST_DATA := $(DIST_DIR)/data
MANIFEST_OUTPUT := application.fam
BUILD_FAP := $(HOME)/.ufbt/build/$(APPID).fap
DIST_FAP := $(DIST)/$(APPID).fap

# ---------------------------
# 1. PNG -> raw bin
# ---------------------------
$(ASSET_DIR)/raw_%.bin: $(ASSET_DIR)/%.png
	$(CONVERT) $< $(CONVERT_ARGS) gray:$@

# raw -> final game bin
$(DATA_DIR)/%.bin: $(ASSET_DIR)/raw_%.bin
	python3 $(CONVERT_TO_BIN) $< $@

# Gather assets
BIN_ASSETS := $(ASSETS:%=$(DATA_DIR)/%.bin)
DAT_ASSETS := $(wildcard $(DATA_DIR)/*.dat)
OVL_ASSETS := $(wildcard $(DATA_DIR)/ovl/*.ovl)
ALL_ASSETS := $(BIN_ASSETS) $(DAT_ASSETS) $(OVL_ASSETS)

assets: $(BIN_ASSETS)
	@echo "Assets converted."

# ---------------------------
# Build via ufbt
# ---------------------------
build:
	ufbt build

# ---------------------------
# Package into ./dist exactly
# ---------------------------
package: build assets
	mkdir -p $(DIST_DIR)
	mkdir -p $(DIST_DATA)

	# copy FAP
	cp "$(FAP_OUTPUT)" "$(DIST_DIR)/flipperhack.fap"

	# copy manifest
	cp "$(MANIFEST_OUTPUT)" "$(DIST_DIR)/application.fam"

	# copy asset bins and dats
	cp $(ALL_ASSETS) $(DIST_DATA)/

	@echo
	@echo "Distribution prepared in ./dist/"
	@echo "Installer script will upload:"
	@echo "  dist/flipperhack.fap"
	@echo "  dist/data/*.bin"
	@echo "  dist/data/*.dat"
	@echo "  dist/data/ovl/*.ovl"

clean:
	ufbt -c

clean-raw:
	rm -f $(ASSETS:%=$(ASSET_DIR)/raw_%.bin)

clean-bin:
	rm -f $(DATA_DIR)/*.bin

install:
	python ./install.py

all: clean build assets package install

.PHONY: all package build assets clean clean-raw clean-bin install
