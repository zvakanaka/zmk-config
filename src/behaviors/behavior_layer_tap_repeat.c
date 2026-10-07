/*
 * Layer-tap that repeats its key when held alone.
 *
 *   &ltr LAYER KEYCODE
 *
 * Like QMK's LT() with its default settings, plus a repeat when held on its own:
 * - Released before tapping-term-ms: taps KEYCODE. Keys pressed meanwhile are held back and sent
 *   after it, so rolls stay in order.
 * - Still held at tapping-term-ms: LAYER turns on, and any keys held back are sent on LAYER.
 * - Once LAYER is on, pressing any other key keeps it on until release, however long you hold.
 * - Held alone until repeat-term-ms: LAYER turns off and KEYCODE is tapped every repeat-interval-ms
 *   until release. The firmware repeats it itself because the OS repeat delay would come on top.
 * - Pressed again within quick-tap-ms of a tap: KEYCODE is held down straight away (OS repeat).
 * - KEYCODE is never sent while another key is held, other than Ctrl or Alt (Ctrl/Alt+Backspace).
 *   Holding Shift, Gui or a letter means this key was meant as LAYER (e.g. Shift+arrows), so a
 *   quick release sends the held-back keys on LAYER instead, and holding it doesn't repeat.
 *
 * Stock hold-taps can't do this: "held past the term" can only mean the hold binding.
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_layer_tap_repeat

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <drivers/behavior.h>
#include <dt-bindings/zmk/modifiers.h>

#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/hid.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#define MAX_CAPTURED_EVENTS 16

struct ltr_config {
    int32_t tapping_term_ms;
    int32_t repeat_term_ms;
    int32_t repeat_interval_ms;
    int32_t quick_tap_ms;
};

enum ltr_status {
    LTR_IDLE,
    LTR_UNDECIDED,  // before tapping-term: other keys are held back
    LTR_LAYER,      // layer on, no other key pressed yet
    LTR_LAYER_USED, // layer on and used; stays on until release
    LTR_REPEATING,  // firmware repeat of the keycode
    LTR_HELD,       // keycode held down after a quick tap
};

// Only one &ltr key can be active at a time; a second one pressed meanwhile just taps its keycode.
static struct {
    enum ltr_status status;
    const struct ltr_config *config;
    uint32_t position;
    uint32_t layer;
    uint32_t keycode;
    int64_t timestamp;
} active;

static int64_t last_tap_time;
static uint32_t last_tap_position = UINT32_MAX;

static struct zmk_position_state_changed_event captured[MAX_CAPTURED_EVENTS];
static int captured_count;

static struct k_work_delayable ltr_timer;

static int ltr_position_listener(const zmk_event_t *eh);
ZMK_LISTENER(behavior_layer_tap_repeat, ltr_position_listener);

static void tap_keycode(uint32_t keycode, int64_t timestamp) {
    raise_zmk_keycode_state_changed_from_encoded(keycode, true, timestamp);
    raise_zmk_keycode_state_changed_from_encoded(keycode, false, timestamp);
}

// True if a key other than Ctrl or Alt is held down.
static bool other_key_held(void) {
    if (zmk_hid_get_explicit_mods() & ~(MOD_LCTL | MOD_RCTL | MOD_LALT | MOD_RALT)) {
        return true;
    }
    // Non-modifier keys: an NKRO bitmap or an HKRO list of usages, all zero when none are held.
    const struct zmk_hid_keyboard_report *report = zmk_hid_get_keyboard_report();
    for (size_t i = 0; i < sizeof(report->body.keys); i++) {
        if (report->body.keys[i]) {
            return true;
        }
    }
    return false;
}

static bool have_captured_press(uint32_t position) {
    for (int i = 0; i < captured_count; i++) {
        if (captured[i].data.position == position && captured[i].data.state) {
            return true;
        }
    }
    return false;
}

// Send held-back key events on, starting at this listener so they're seen with the new state.
static void release_captured_events(void) {
    int count = captured_count;
    captured_count = 0;
    for (int i = 0; i < count; i++) {
        LOG_DBG("%d ltr releasing %d %s", active.position, captured[i].data.position,
                captured[i].data.state ? "down" : "up");
        ZMK_EVENT_RAISE_AT(captured[i], behavior_layer_tap_repeat);
    }
}

static void start_layer(void) {
    LOG_DBG("%d ltr layer %d on", active.position, active.layer);
    active.status = LTR_LAYER;
    zmk_keymap_layer_activate(active.layer, false);
    int64_t repeat_at = active.timestamp + active.config->repeat_term_ms;
    k_work_reschedule(&ltr_timer, K_MSEC(MAX(repeat_at - k_uptime_get(), 0)));
    release_captured_events();
}

static void ltr_timer_handler(struct k_work *work) {
    switch (active.status) {
    case LTR_UNDECIDED:
        start_layer();
        break;
    case LTR_LAYER:
        if (other_key_held()) {
            LOG_DBG("%d ltr held with another key, staying on layer", active.position);
            active.status = LTR_LAYER_USED;
            break;
        }
        LOG_DBG("%d ltr held alone, repeating", active.position);
        active.status = LTR_REPEATING;
        zmk_keymap_layer_deactivate(active.layer, false);
        // fall through
    case LTR_REPEATING:
        tap_keycode(active.keycode, k_uptime_get());
        k_work_reschedule(&ltr_timer, K_MSEC(active.config->repeat_interval_ms));
        break;
    default:
        break;
    }
}

static int on_ltr_binding_pressed(struct zmk_behavior_binding *binding,
                                  struct zmk_behavior_binding_event event) {
    const struct ltr_config *config = zmk_behavior_get_binding(binding->behavior_dev)->config;

    if (active.status != LTR_IDLE) {
        tap_keycode(binding->param2, event.timestamp);
        return ZMK_BEHAVIOR_OPAQUE;
    }

    active.config = config;
    active.position = event.position;
    active.layer = binding->param1;
    active.keycode = binding->param2;
    active.timestamp = event.timestamp;

    if (config->quick_tap_ms > 0 && last_tap_position == event.position &&
        event.timestamp - last_tap_time < config->quick_tap_ms && !other_key_held()) {
        LOG_DBG("%d ltr quick tap, holding", event.position);
        active.status = LTR_HELD;
        return raise_zmk_keycode_state_changed_from_encoded(active.keycode, true, event.timestamp);
    }

    active.status = LTR_UNDECIDED;
    int64_t decide_at = event.timestamp + config->tapping_term_ms;
    k_work_reschedule(&ltr_timer, K_MSEC(MAX(decide_at - k_uptime_get(), 0)));
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_ltr_binding_released(struct zmk_behavior_binding *binding,
                                   struct zmk_behavior_binding_event event) {
    if (active.status == LTR_IDLE || active.position != event.position) {
        return ZMK_BEHAVIOR_OPAQUE;
    }

    k_work_cancel_delayable(&ltr_timer);
    enum ltr_status status = active.status;
    active.status = LTR_IDLE;

    switch (status) {
    case LTR_UNDECIDED:
        if (other_key_held()) {
            LOG_DBG("%d ltr released with another key held, layer only", active.position);
            zmk_keymap_layer_activate(active.layer, false);
            release_captured_events();
            zmk_keymap_layer_deactivate(active.layer, false);
            break;
        }
        tap_keycode(active.keycode, event.timestamp);
        last_tap_time = event.timestamp;
        last_tap_position = event.position;
        release_captured_events();
        break;
    case LTR_LAYER:
    case LTR_LAYER_USED:
        zmk_keymap_layer_deactivate(active.layer, false);
        break;
    case LTR_HELD:
        raise_zmk_keycode_state_changed_from_encoded(active.keycode, false, event.timestamp);
        break;
    default:
        break;
    }

    return ZMK_BEHAVIOR_OPAQUE;
}

static int ltr_position_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);

    if (ev == NULL || active.status == LTR_IDLE || ev->position == active.position) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    // Events can queue up behind a late timer; decide by timestamp so they land on the right side.
    if (active.status == LTR_UNDECIDED &&
        ev->timestamp >= active.timestamp + active.config->tapping_term_ms) {
        k_work_cancel_delayable(&ltr_timer);
        start_layer();
    }

    switch (active.status) {
    case LTR_UNDECIDED:
        // A release is only held back if its press was, so keys held from before pass through.
        if (!ev->state && !have_captured_press(ev->position)) {
            return ZMK_EV_EVENT_BUBBLE;
        }
        if (captured_count == MAX_CAPTURED_EVENTS) {
            k_work_cancel_delayable(&ltr_timer);
            start_layer();
            break;
        }
        LOG_DBG("%d ltr holding back %d %s", active.position, ev->position,
                ev->state ? "down" : "up");
        captured[captured_count++] = copy_raised_zmk_position_state_changed(ev);
        return ZMK_EV_EVENT_CAPTURED;
    case LTR_LAYER:
        if (ev->state) {
            LOG_DBG("%d ltr layer used by %d", active.position, ev->position);
            k_work_cancel_delayable(&ltr_timer);
            active.status = LTR_LAYER_USED;
        }
        break;
    default:
        break;
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_SUBSCRIPTION(behavior_layer_tap_repeat, zmk_position_state_changed);

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata layer_values[] = {
    {.display_name = "Layer", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_LAYER_ID},
};

static const struct behavior_parameter_value_metadata key_values[] = {
    {.display_name = "Key", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_HID_USAGE},
};

static const struct behavior_parameter_metadata_set metadata_sets[] = {{
    .param1_values = layer_values,
    .param1_values_len = ARRAY_SIZE(layer_values),
    .param2_values = key_values,
    .param2_values_len = ARRAY_SIZE(key_values),
}};

static const struct behavior_parameter_metadata metadata = {
    .sets_len = ARRAY_SIZE(metadata_sets),
    .sets = metadata_sets,
};

#endif

static const struct behavior_driver_api ltr_driver_api = {
    .binding_pressed = on_ltr_binding_pressed,
    .binding_released = on_ltr_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif
};

static int ltr_init(const struct device *dev) {
    static bool initialized;
    if (!initialized) {
        k_work_init_delayable(&ltr_timer, ltr_timer_handler);
        initialized = true;
    }
    return 0;
}

#define LTR_INST(n)                                                                                \
    static const struct ltr_config ltr_config_##n = {                                              \
        .tapping_term_ms = DT_INST_PROP(n, tapping_term_ms),                                       \
        .repeat_term_ms = DT_INST_PROP(n, repeat_term_ms),                                         \
        .repeat_interval_ms = DT_INST_PROP(n, repeat_interval_ms),                                 \
        .quick_tap_ms = DT_INST_PROP(n, quick_tap_ms),                                             \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, ltr_init, NULL, NULL, &ltr_config_##n, POST_KERNEL,                 \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &ltr_driver_api);

DT_INST_FOREACH_STATUS_OKAY(LTR_INST)

#endif
