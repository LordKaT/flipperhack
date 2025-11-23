## flipperhack

A roguelike game for the Flipper Zero.


### Build

```bash
ufbt build
```

Use ImageMagick to convert assets/title.png to scratch/raw_title.bin:

```bash
convert assets/title.png -resize 128x64\! -threshold 50% -depth 1 gray:scratch/raw_title.bin
```

Now use the build scripts to convert to proper bin, b64 encode, and push to flipper:

```bash
./build/convert_to_bin.py scratch/raw_title.bin data/title.bin
./build/push_to_flipper.py data/title.bin
```


### Installation

Not automated yet.

Install to /ext/apps/Games/flipperhack.fap

Data to /ext/apps_data/flipperhack/


### Controls

- Arrows: move
- Bump: attack
- Center button (short): wait
- Center button (long): menu
