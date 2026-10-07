# zmk-config

ZMK firmware for a Corne 3x5 (Typeractive kit): nice!nano v2 controllers, no displays.
The keymap is in `config/corne.keymap`, and the build targets are listed in `build.yaml`.

The keymap was inspired by [Peter J's keymap](https://keymapdb.com/keymaps/peterxjang/).

The older Dactyl Cygnus 4x6 shield (`boards/shields/cygnus`, `config/cygnus.keymap`) is still here but isn't built.
Much of that code comes from <https://github.com/juhakaup/zmk-cygnus-4x6>.

## Keymap

Hold Enter for Num, hold Backspace for Nav, and press Enter + Backspace together and hold for System.

Two quick chords always give the shifted key, however fast and whichever key is released first: Z then `/` types `?`, and `'` then A types `A`.

```
BASE   (top line = tap, bottom line = hold)

┌───────┬───────┬───────┬───────┬───────┐      ┌───────┬───────┬───────┬───────┬───────┐
│   Q   │   W   │   E   │   R   │   T   │      │   Y   │   U   │   I   │   O   │   P   │
│  Ctrl │       │       │       │       │      │       │       │       │       │       │
├───────┼───────┼───────┼───────┼───────┤      ├───────┼───────┼───────┼───────┼───────┤
│   A   │   S   │   D   │   F   │   G   │      │   H   │   J   │   K   │   L   │   '   │
│ Shift │       │       │       │       │      │       │       │       │       │ Shift │
├───────┼───────┼───────┼───────┼───────┤      ├───────┼───────┼───────┼───────┼───────┤
│   Z   │   X   │   C   │   V   │   B   │      │   N   │   M   │   ,   │   .   │   /   │
│ Shift │       │       │       │       │      │       │       │       │       │ Shift │
└───────┴───────┼───────┼───────┼───────┤      ├───────┼───────┼───────┼───────┴───────┘
                │  Esc  │ Space │  Tab  │      │ PrtSc │ Enter │  Bksp │
                │  Alt  │  Gui  │  Ctrl │      │       │  Num  │  Nav  │
                └───────┴───────┴───────┘      └───────┴───────┴───────┘


NUM   (hold Enter)   top = key · middle = with Shift · bottom = hold

┌───────┬───────┬───────┬───────┬───────┐      ┌───────┬───────┬───────┬───────┬───────┐
│   1   │   2   │   3   │   4   │   5   │      │   6   │   7   │   8   │   9   │   0   │
│   !   │   @   │   #   │   $   │   %   │      │   ^   │   &   │   *   │   (   │   )   │
│       │       │       │       │       │      │       │       │       │       │       │
├───────┼───────┼───────┼───────┼───────┤      ├───────┼───────┼───────┼───────┼───────┤
│   F1  │   F2  │   F3  │   F4  │   F5  │      │   F6  │   -   │   =   │   `   │   ;   │
│       │       │       │       │       │      │       │   _   │   +   │   ~   │   :   │
│ Shift │       │       │       │       │      │       │       │       │       │ Shift │
├───────┼───────┼───────┼───────┼───────┤      ├───────┼───────┼───────┼───────┼───────┤
│   F7  │   F8  │   F9  │  F10  │  F11  │      │  F12  │       │   [   │   ]   │   \   │
│       │       │       │       │       │      │       │       │   {   │   }   │   |   │
│ Shift │       │       │       │       │      │       │       │       │       │ Shift │
└───────┴───────┼───────┼───────┼───────┤      ├───────┼───────┼───────┼───────┴───────┘
                │       │       │       │      │       │       │  Del  │
                │       │       │       │      │       │       │       │
                │       │       │       │      │       │  held │       │
                └───────┴───────┴───────┘      └───────┴───────┴───────┘


NAV   (hold Backspace)

┌───────┬───────┬───────┬───────┬───────┐      ┌───────┬───────┬───────┬───────┬───────┐
│       │       │  Ms ↑ │ M-Clk │       │      │  Home │  End  │  PgUp │  PgDn │       │
│       │       │       │       │       │      │       │       │       │       │       │
├───────┼───────┼───────┼───────┼───────┤      ├───────┼───────┼───────┼───────┼───────┤
│ L-Clk │  Ms ← │  Ms ↓ │  Ms → │ R-Clk │      │   ←   │   ↓   │   ↑   │   →   │       │
│       │       │       │       │       │      │       │       │       │       │       │
├───────┼───────┼───────┼───────┼───────┤      ├───────┼───────┼───────┼───────┼───────┤
│       │  Prev │  Stop │  Play │  Next │      │ Bri − │ Bri + │ Vol − │ Vol + │       │
│       │       │       │       │       │      │       │       │       │       │       │
└───────┴───────┼───────┼───────┼───────┤      ├───────┼───────┼───────┼───────┴───────┘
                │       │       │       │      │       │       │       │
                │       │       │       │      │       │       │  held │
                └───────┴───────┴───────┘      └───────┴───────┴───────┘


SYSTEM   (hold Enter + Backspace together)

┌───────┬───────┬───────┬───────┬───────┐      ┌───────┬───────┬───────┬───────┬───────┐
│  BT 0 │  BT 1 │  BT 2 │  BT 3 │  BT 4 │      │       │       │       │       │       │
│       │       │       │       │       │      │       │       │       │       │       │
├───────┼───────┼───────┼───────┼───────┤      ├───────┼───────┼───────┼───────┼───────┤
│ BT CLR│       │       │       │       │      │       │       │       │       │       │
│       │       │       │       │       │      │       │       │       │       │       │
├───────┼───────┼───────┼───────┼───────┤      ├───────┼───────┼───────┼───────┼───────┤
│  BOOT │       │       │       │ USB/BT│      │       │       │       │       │  BOOT │
│       │       │       │       │       │      │       │       │       │       │       │
└───────┴───────┼───────┼───────┼───────┤      ├───────┼───────┼───────┼───────┴───────┘
                │       │       │       │      │       │       │       │
                │       │       │       │      │       │  held │  held │
                └───────┴───────┴───────┘      └───────┴───────┴───────┘
```

Blank keys do nothing, or pass through to the Base layer.

## Building

The build runs locally in the official ZMK container, using podman:

```sh
./build.sh              # build everything in build.yaml -> firmware/*.uf2
./build.sh corne_left   # build one target
./build.sh --update     # pull newer ZMK/Zephyr first
./build.sh --pristine   # clean rebuild
```

The first run downloads ZMK, Zephyr and modules into `.zmk/`, which is gitignored. Later runs reuse them.

## Custom behaviors and tests

- `src/behaviors/behavior_layer_tap_repeat.c`: the Backspace/Nav key (`&ltr`). Tap for Backspace, hold for Nav, and hold alone to repeat Backspace.
- `src/behaviors/behavior_hold_tap_follow.c`: a copy of ZMK's hold-tap with one extra setting, `hold-follow-key-positions`, which makes the Z and `'` chords above work. Hold-taps share global state, so every hold-tap in the keymap uses this copy (the stock `&mt` is disabled), and it needs syncing by hand if ZMK's hold-tap changes.

`./test.sh` runs the simulator tests in `tests/` against both. Each test replays timed key presses and compares the keycodes sent with a snapshot (`./test.sh --accept` rewrites the snapshots, so check the diff). Run `./build.sh` once first.

## Flashing

Double-tap reset on a nice!nano to mount it as a USB drive, then copy the matching `.uf2` onto it.
You can also hold Enter + Backspace (System layer) and press `BOOT`, the bottom outer key on either side.

If the halves won't pair, flash `settings_reset.uf2` to both halves, then flash the normal firmware again.

The left half is the central and has ZMK Studio enabled over USB, at <https://zmk.studio>.

## Battery

Both halves report their battery level over Bluetooth. `config/corne.conf` enables `CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING` and `..._PROXY`, so the left half (central) also passes on the right half's level as a second Battery Service.

On Linux (BlueZ), `org.bluez.Battery1.Percentage` (what upower and most desktops show) only reports the left half; bluetoothd uses the first Battery Service and ignores the rest.
To read both, read the Battery Level characteristics (`0x2A19`) over GATT. The right half's has a User Description (`0x2901`) of `Peripheral 0`, and the left half's has none.
A status bar such as waybar can be configured to read these and show both percentages.

After changing these settings, remove and re-pair the keyboard so the host picks up the new service.
Press `BT CLR` on the System layer to clear the keyboard's side too. ZMK asks the host to confirm pairing, so `bluetoothctl pair` needs an agent (`agent NoInputNoOutput`, then `default-agent`, in an interactive `bluetoothctl`), or it fails with `AuthenticationFailed`.
