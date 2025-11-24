## flipperhack

A roguelike game for the Flipper Zero.


### Build

```bash
make clean
make build
make package
make install
```

### Installation & Running

```bash
python ./install.py # install to flipper
python ./install.py --launch # install to flipper and launch
```

You should now be able to run flipperhack from the app menu.

### Manual Installation

- Install flipperhack.fap to /ext/apps/Games/
- Install everything in ./data to /ext/apps_data/flipperhack/

### Controls

- Arrows: move
- Bump: attack
- Center button (short): wait
- Center button (long): menu
- Back button: menu