# Keymap behaviors only run on the central half; split peripherals don't have the keymap code.
if((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  target_sources_ifdef(CONFIG_ZMK_BEHAVIOR_LAYER_TAP_REPEAT app PRIVATE src/behaviors/behavior_layer_tap_repeat.c)
  target_sources_ifdef(CONFIG_ZMK_BEHAVIOR_HOLD_TAP_FOLLOW app PRIVATE src/behaviors/behavior_hold_tap_follow.c)
endif()
