#include QMK_KEYBOARD_H

#include "features/custom_shift_keys.h"
#include "features/orbital_mouse.h"

// DEBUG
// #include "print.h"

enum custom_keycodes {
    MAC_VIM_SEARCH = SAFE_RANGE,
    MAC_QUOT_SURROUND,
    MAC_EMAIL,
    MAC_NAME,
    TAB_KEY,
    NUMPAD_MO,
    SPECIALS_MO
};

#define KC_UNDO LCMD(KC_Z)

const custom_shift_key_t custom_shift_keys[] = {
  {KC_QUOT, KC_COLN},
  {KC_COMM, KC_TILD},
};
uint8_t NUM_CUSTOM_SHIFT_KEYS = sizeof(custom_shift_keys) / sizeof(custom_shift_key_t);

#define DEFAULT_LAYER 0
#define CAPSLOCK_LAYER 1
#define TAB_LAYER 2
#define NUMPAD_LAYER 3
#define R_CORNER_LAYER 4
#define MACRO_LAYER 5
#define DOUBLE_LAYER 6
#define L_CORNER_LAYER 7

// Track which layer we're currently in
uint8_t current_layer = DEFAULT_LAYER;

layer_state_t layer_state_set_user(layer_state_t state) {
    if (layer_state_cmp(state, DEFAULT_LAYER)) {
        rgblight_sethsv_noeeprom(HSV_RED);
        current_layer = DEFAULT_LAYER;
    } else if (layer_state_cmp(state, MACRO_LAYER)) {
        rgblight_sethsv_noeeprom(HSV_GREEN); // Different color for Q-prefix layer
        current_layer = MACRO_LAYER;
    } else {
        rgblight_sethsv_noeeprom(HSV_BLUE);
        current_layer = get_highest_layer(state);
    }
    return state;
}

void keyboard_post_init_user(void) {
    // Customise these values to desired behaviour
//    debug_enable=true;
//    debug_keyboard=true;
//    debug_mouse=true;
}

// Function to send "q" + key character
// bool handle_q_prefix(uint16_t keycode, keyrecord_t* record) {
//     // Only process alphanumeric keys and only when in the MACRO_LAYER
//     if (current_layer == MACRO_LAYER && record->event.pressed) {
//         // Check if it's a letter key (A-Z)
//         if (keycode >= KC_A && keycode <= KC_Z) {
//             char letter = 'a' + (keycode - KC_A); // Convert to lowercase letter
//             SEND_STRING("q");
//             send_char(letter);
//             return false; // Key handled, don't process further
//         } else if (keycode == KC_ENT) {
//             SEND_STRING("q");
//             tap_code(KC_ENT);
//             return false; // Key handled, don't process further
//         }
//     }
//     return true; // Continue normal processing
// }

static bool numpad_held = false;
static bool specials_held = false;
static bool osm_interrupted = false;
static bool osm_active = false;

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    // Check if we're in MACRO_LAYER and need to handle a Q-prefix
    // if (current_layer == MACRO_LAYER && !handle_q_prefix(keycode, record)) {
    //     return false;
    // }
    if (record->event.pressed && osm_active && keycode != NUMPAD_MO) {
        osm_interrupted = true;
    }
    
    if (!process_custom_shift_keys(keycode, record)) { return false; }
    if (!process_orbital_mouse(keycode, record)) { return false; }

    switch (keycode) {
        case MAC_VIM_SEARCH:
            if (record->event.pressed) {
                SEND_STRING(":s/");
            }
            break;
        case MAC_EMAIL:
            if (record->event.pressed) {
                SEND_STRING("pjcfifa@gmail.com");
            }
            break;
        case MAC_NAME:
            if (record->event.pressed) {
                SEND_STRING("Pat Cunniff");
            }
            break;
        case MAC_QUOT_SURROUND:
            if (record->event.pressed) {
                SEND_STRING(":s/\\%V\\(.*\\)\\%V\\(.\\)/\"\\1\\2\"/");
                tap_code(KC_ENT);
            }
            break;
        case NUMPAD_MO:
            if (record->event.pressed) {
                osm_interrupted = false;
                osm_active = true;
                layer_on(NUMPAD_LAYER);
                numpad_held = true;
            } else {
                layer_off(NUMPAD_LAYER);
                osm_active = false;
                numpad_held = false;
                if (!osm_interrupted) {
                    set_oneshot_mods(MOD_BIT(KC_LSFT));
                }
            }
            break;
        case SPECIALS_MO:
            if (record->event.pressed) {
                layer_on(CAPSLOCK_LAYER);
                specials_held = true;
            } else {
                layer_off(CAPSLOCK_LAYER);
                specials_held = false;
            }
            break;
    }

    // Handle DOUBLE_LAYER based on combo
    if (numpad_held && specials_held) {
        layer_on(DOUBLE_LAYER);
    } else {
        layer_off(DOUBLE_LAYER);
    }

    return true;
}

void housekeeping_task_user(void) {
    orbital_mouse_task();
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // ALPHA
    [DEFAULT_LAYER] = LAYOUT_planck_2x2u(
// TAB              , Q      , W      , E      , R        , T   , Y     , U   , I      , O      , P              , DEL               ,
LT(TAB_LAYER,KC_TAB), KC_Q   , KC_W   , KC_E   , KC_R     , KC_T, KC_Y  , KC_U, KC_I   , KC_O   , KC_P           , KC_BSPC           ,
SPECIALS_MO         , KC_A   , KC_S   , KC_D   , KC_F     , KC_G, KC_H  , KC_J, KC_K   , KC_L   , KC_QUOT        , KC_ENT            ,
KC_LSFT             , KC_Z   , KC_X   , KC_C   , KC_V     , KC_B, KC_N  , KC_M, KC_COMM, KC_DOT , KC_SLSH        , KC_RSFT           ,
MO(L_CORNER_LAYER)  , KC_LCTL, KC_HYPR, KC_LGUI, NUMPAD_MO      , KC_SPC      , KC_ESC , KC_LALT, MO(MACRO_LAYER), MO(R_CORNER_LAYER)
      ),

    // CAPSLOCK
    [CAPSLOCK_LAYER] = LAYOUT_planck_2x2u(
// TAB , Q      , W      , E      , R      , T         , Y      , U      , I      , O      , P      , DEL         ,
KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO     , KC_GRV , KC_LCBR, KC_RCBR, KC_PLUS, KC_DQUO, KC_TRNS     ,
KC_TRNS, KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO     , KC_PIPE, KC_LPRN, KC_RPRN, KC_EQL , KC_COLN, LSFT(KC_ENT),
KC_TRNS, KC_NO  , KC_NO  , KC_NO  , KC_NO  , LCTL(KC_B), KC_NO  , KC_LBRC, KC_RBRC, KC_MINS, KC_BSLS, KC_TRNS     ,
KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS            , KC_TRNS         , KC_TRNS, KC_TRNS, KC_TRNS, QK_LLCK     
      ),

    // TAB
    [TAB_LAYER] = LAYOUT_planck_2x2u(
// TAB , Q      , W      , E      , R      , T    , Y      , U           , I            , O      , P      , DEL    ,
KC_NO  , KC_NO  , KC_NO  , KC_UP  , KC_NO  , KC_NO, KC_HOME, LCTL(KC_END), LCTL(KC_HOME), KC_END , KC_NO  , KC_TRNS,
KC_NO  , KC_NO  , KC_LEFT, KC_DOWN, KC_RGHT, KC_NO, KC_LEFT, KC_DOWN     , KC_UP        , KC_RGHT, KC_NO  , KC_TRNS,
KC_TRNS, KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO, KC_NO  , KC_NO       , KC_NO        , KC_NO  , KC_NO  , KC_TRNS,
KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS       , KC_TRNS              , KC_NO        , KC_TRNS, KC_TRNS, QK_LLCK
),

    // NUMPAD
    [NUMPAD_LAYER] = LAYOUT_planck_2x2u(
// TAB , Q      , W      , E      , R      , T    , Y      , U   , I   , O      , P      , DEL    ,
KC_NO  , KC_1   , KC_2   , KC_3   , KC_4   , KC_5 , KC_6   , KC_7, KC_8, KC_9   , KC_0   , KC_TRNS,
KC_TRNS, KC_NO  , KC_F4  , KC_F5  , KC_F6  , KC_NO, KC_UNDS, KC_4, KC_5, KC_6   , KC_SCLN, KC_TRNS,
KC_TRNS, KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO, KC_NO  , KC_1, KC_2, KC_3   , KC_BSLS, KC_TRNS,
KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS       , KC_TRNS      , KC_0, KC_TRNS, KC_TRNS, QK_LLCK
),

    // R_CORNER_LAYER
    [R_CORNER_LAYER] = LAYOUT_planck_2x2u(
// TAB , Q      , W      , E      , R      , T    , Y      , U    , I    , O      , P      , DEL    ,
KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO, KC_NO  , KC_NO, KC_NO, KC_MUTE, KC_VOLD, KC_VOLU,
KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO, KC_NO  , KC_NO, KC_NO, KC_MPRV, KC_MPLY, KC_MNXT,
KC_TRNS, KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO, KC_NO  , KC_NO, KC_NO, KC_NO  , KC_NO  , KC_NO  ,
KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS       , KC_TRNS       , KC_NO, KC_TRNS, KC_TRNS, KC_TRNS
),

    // MACRO
    [MACRO_LAYER] = LAYOUT_planck_2x2u(
// TAB, Q    , W                , E        , R             , T    , Y       , U    , I    , O      , P      , DEL    ,
KC_NO , KC_NO, MAC_QUOT_SURROUND, MAC_EMAIL, MAC_VIM_SEARCH, KC_NO, KC_NO   , KC_NO, KC_NO, KC_NO  , KC_UP  , KC_NO  , 
KC_NO , KC_NO, KC_NO            , KC_NO    , KC_NO         , KC_NO, KC_NO   , KC_NO, KC_NO, KC_LEFT, KC_DOWN, KC_RIGHT, 
KC_NO , KC_NO, KC_NO            , KC_NO    , KC_NO         , KC_NO, MAC_NAME, KC_NO, KC_NO, KC_NO,   KC_NO,   KC_NO,
KC_NO , KC_NO, KC_NO            , KC_NO    , KC_NO                , KC_NO          , KC_NO, KC_TRNS, KC_TRNS, QK_LLCK
),

    // DOUBLE
    [DOUBLE_LAYER] = LAYOUT_planck_2x2u(
// TAB , Q      , W      , E      , R      , T    , Y     , U    , I      , O      , P      , DEL    ,
KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO, KC_NO , KC_NO, KC_NO  , KC_NO  , KC_NO  , KC_NO  ,
KC_TRNS, KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO, KC_NO , KC_LT, KC_GT  , KC_NO  , KC_NO  , KC_NO  ,
KC_TRNS, KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO, KC_NO , KC_NO, KC_NO  , KC_NO  , KC_NO  , KC_TRNS,
KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS       , KC_SPC       , KC_TRNS, KC_TRNS, KC_TRNS, QK_LLCK
),

    // L_CORNER
    [L_CORNER_LAYER] = LAYOUT_planck_2x2u(
// TAB , Q      , W      , E      , R      , T    , Y     , U    , I      , O      , P      , DEL    ,
QK_BOOT, KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO, KC_NO , KC_NO, KC_NO  , KC_NO  , KC_NO  , KC_NO  ,
KC_TRNS, KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO, KC_NO , KC_NO, KC_NO  , KC_NO  , KC_NO  , KC_NO  ,
KC_TRNS, KC_NO  , KC_NO  , KC_NO  , KC_NO  , KC_NO, KC_NO , KC_NO, KC_NO  , KC_NO  , KC_NO  , KC_TRNS,
KC_TRNS, OM_BTN2, OM_BTN1, KC_UNDO, KC_TRNS       , KC_SPC       , KC_TRNS, KC_TRNS, KC_TRNS, QK_LLCK
        ),
};
