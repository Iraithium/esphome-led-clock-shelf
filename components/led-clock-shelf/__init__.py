import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core
from esphome.const import CONF_ID, CONF_TIME_ID, CONF_PIN, CONF_BRIGHTNESS
import esphome.components.time

led_clock_shelf_ns = cg.esphome_ns.namespace("led_clock_shelf")
LEDClockShelf = led_clock_shelf_ns.class_("LEDClockShelf", cg.Component)

cg.add_library("FastLED", "3.5.0")

# Custom color validation
def validate_color_value(value):
    try:
        from esphome.config_validation import color
        if isinstance(value, str):
            return color(value)
    except ImportError:
        if isinstance(value, str):
            hex_str = value.lstrip('#')
            if len(hex_str) != 6:
                raise cv.Invalid("HEX color must be 6 digits (e.g., '#FF0000' or 'FF0000')")
            try:
                int(hex_str, 16)
                return hex_str
            except ValueError:
                raise cv.Invalid("Invalid HEX color format. Use '#RRGGBB' or 'RRGGBB'")
    
    if isinstance(value, list) and len(value) == 3:
        return [cv.uint8_t(v) for v in value]
    
    raise cv.Invalid("Color must be a HEX string (e.g., '#FF0000') or RGB list (e.g., [255, 0, 0])")

COLOR_SCHEMA = cv.Schema({
    cv.Required("name"): cv.string_strict,
    cv.Required("value"): validate_color_value,
})

CONFIG_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(LEDClockShelf),
    cv.Required(CONF_PIN): cv.int_,
    cv.Optional(CONF_BRIGHTNESS, default=255): cv.int_range(min=0, max=255),
    cv.Optional("colon_style", default=0): cv.int_range(min=0, max=2),
    cv.Optional(CONF_TIME_ID): cv.use_id(esphome.components.time.RealTimeClock),
    cv.Optional("downlights_enabled", default=False): cv.boolean,
    cv.Optional("light_sensor_pin", default=34): cv.int_,
    cv.Optional("brightness_reduction_dark", default=50): cv.int_range(min=0, max=100),
    cv.Optional("brightness_transition_duration", default=2000): cv.positive_int,
    cv.Optional("colors", default=[]): cv.ensure_list(COLOR_SCHEMA),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_pin(config[CONF_PIN]))
    cg.add(var.set_default_brightness(config[CONF_BRIGHTNESS]))
    cg.add(var.set_colon_style(config["colon_style"]))
    
    if CONF_TIME_ID in config:
        time_obj = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time(time_obj))

    cg.add(var.set_downlights_enabled(config["downlights_enabled"]))
    cg.add(var.set_light_sensor_pin(config["light_sensor_pin"]))
    cg.add(var.set_brightness_reduction_dark(config["brightness_reduction_dark"]))
    cg.add(var.set_brightness_transition_duration(config["brightness_transition_duration"]))

    # Pass custom colors to the component and store names in CORE.data
    if "colors" in config and config["colors"]:
        colors = []
        color_names = []
        for color_conf in config["colors"]:
            name = color_conf["name"]
            value = color_conf["value"]
            if isinstance(value, tuple):
                r, g, b, _ = value
            elif isinstance(value, str):
                r = int(value[0:2], 16)
                g = int(value[2:4], 16)
                b = int(value[4:6], 16)
            else:  # RGB list
                r, g, b = value
            colors.append((name, cg.global_ns.class_("CRGB")(r, g, b)))
            color_names.append(name)
        cg.add(var.set_custom_colors(colors))
        
        # Store color names in CORE.data under a unique key
        core.CORE.data.setdefault("led_clock_shelf", {})[str(config[CONF_ID])] = color_names
    else:
        # Default colors if none are provided
        default_colors = [
            ("Red", cg.global_ns.class_("CRGB")(255, 0, 0)),
            ("Green", cg.global_ns.class_("CRGB")(0, 255, 0)),
            ("Blue", cg.global_ns.class_("CRGB")(0, 0, 255)),
            ("Yellow", cg.global_ns.class_("CRGB")(255, 255, 0)),
            ("White", cg.global_ns.class_("CRGB")(255, 255, 255)),
            ("Orange", cg.global_ns.class_("CRGB")(255, 165, 0)),
            ("Purple", cg.global_ns.class_("CRGB")(128, 0, 128)),
            ("Cyan", cg.global_ns.class_("CRGB")(0, 255, 255)),
            ("Pink", cg.global_ns.class_("CRGB")(255, 105, 180)),
            ("Magenta", cg.global_ns.class_("CRGB")(255, 0, 255)),
        ]
        cg.add(var.set_custom_colors(default_colors))
        core.CORE.data.setdefault("led_clock_shelf", {})[str(config[CONF_ID])] = [name for name, _ in default_colors]