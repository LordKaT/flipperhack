ASSETS := title gameover

ASSET_DIR := images
DATA_DIR  := data
BUILD_DIR := tools

CONVERT := convert
CONVERT_ARGS := -resize 128x64\! -threshold 50% -depth 1

CONVERT_TO_BIN := $(BUILD_DIR)/convert_to_bin.py

FAP_OUTPUT := $(HOME)/.ufbt/build/flipperhack.fap

DIST_DIR := dist
DIST_DATA := $(DIST_DIR)/data
DIST_GFX := $(DIST_DATA)/gfx

$(ASSET_DIR)/raw_%.bin: $(ASSET_DIR)/%.png
	$(CONVERT) $< $(CONVERT_ARGS) gray:$@

$(DIST_GFX):
	mkdir -p $(DIST_GFX)

$(DIST_GFX)/%.bin: $(ASSET_DIR)/raw_%.bin | $(DIST_GFX)
	python3 $(CONVERT_TO_BIN) $< $@

GFX_ASSETS := $(ASSETS:%=$(DIST_GFX)/%.bin)
ROM_ASSETS := $(wildcard $(DATA_DIR)/rom/*.rom)
ALL_ASSETS := $(GFX_ASSETS) $(ROM_ASSETS)

assets: $(GFX_ASSETS)
	@echo "Assets converted."

build:
	ufbt build

package: build assets
	mkdir -p $(DIST_DIR)
	mkdir -p $(DIST_DATA)
	cp "$(FAP_OUTPUT)" "$(DIST_DIR)/flipperhack.fap"
	#cp -r $(DATA_DIR)/rom $(DIST_DATA)/

clean:
	ufbt -c

clean-raw:
	rm -f $(ASSETS:%=$(ASSET_DIR)/raw_%.bin)

clean-bin:
	rm -f $(DIST_GFX)/*.bin

install:
	python ./install.py

all: clean build assets package install

.PHONY: all package build assets clean clean-raw clean-bin install
