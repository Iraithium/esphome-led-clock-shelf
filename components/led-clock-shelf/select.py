import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core
from esphome.components import select
from esphome.const import CONF_ID, CONF_NAME, CONF_OPTIONS, CONF_TYPE
from . import led_clock_shelf_ns, LEDClockShelf

DEPENDENCIES = ["led_clock_shelf"]

LEDClockColonStyleSelect = led_clock_shelf_ns.class_(
    "LEDClockColonStyleSelect", select.Select, cg.Component
)
LEDClockHourColorSelect = led_clock_shelf_ns.class_(
    "LEDClockHourColorSelect", select.Select, cg.Component
)
LEDClockMinuteColorSelect = led_clock_shelf_ns.class_(
    "LEDClockMinuteColorSelect", select.Select, cg.Component
)
LEDClockColonColorSelect = led_clock_shelf_ns.class_(
    "LEDClockColonColorSelect", select.Select, cg.Component
)
LEDClockDownlightColorSelect = led_clock_shelf_ns.class_(
    "LEDClockDownlightColorSelect", select.Select, cg.Component
)

BASE_SCHEMA = select.SELECT_SCHEMA.extend({
    cv.Required("parent_id"): cv.use_id(LEDClockShelf),
}).extend(cv.COMPONENT_SCHEMA)

COLON_STYLE_SCHEMA = BASE_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(LEDClockColonStyleSelect),
    cv.Optional(CONF_NAME, default="LED Clock Colon Style"): cv.string,
    cv.Optional(CONF_OPTIONS, default=["Dual", "Single", "None"]): cv.ensure_list(cv.string),
})

HOUR_COLOR_SCHEMA = BASE_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(LEDClockHourColorSelect),
    cv.Optional(CONF_NAME, default="LED Clock Hour Color"): cv.string,
})

MINUTE_COLOR_SCHEMA = BASE_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(LEDClockMinuteColorSelect),
    cv.Optional(CONF_NAME, default="LED Clock Minute Color"): cv.string,
})

COLON_COLOR_SCHEMA = BASE_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(LEDClockColonColorSelect),
    cv.Optional(CONF_NAME, default="LED Clock Colon Color"): cv.string,
})

DOWNLIGHT_COLOR_SCHEMA = BASE_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(LEDClockDownlightColorSelect),
    cv.Optional(CONF_NAME, default="LED Clock Downlight Color"): cv.string,
})

PLATFORM_SCHEMA = cv.typed_schema({
    "colon_style": COLON_STYLE_SCHEMA,
    "hour_color": HOUR_COLOR_SCHEMA,
    "minute_color": MINUTE_COLOR_SCHEMA,
    "colon_color": COLON_COLOR_SCHEMA,
    "downlight_color": DOWNLIGHT_COLOR_SCHEMA,
}, lower=True)

CONFIG_SCHEMA = cv.Any(
    PLATFORM_SCHEMA,
    cv.ensure_list(PLATFORM_SCHEMA),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    led_clock = await cg.get_variable(config["parent_id"])
    
    if config[CONF_TYPE] in ["hour_color", "minute_color", "colon_color", "downlight_color"]:
        parent_id = str(config["parent_id"])
        color_options = core.CORE.data.get("led_clock_shelf", {}).get(parent_id, ["Red", "Green", "Blue", "Yellow"])
        options = color_options
    else:
        options = config[CONF_OPTIONS]
    
    await select.register_select(var, config, options=options)
    
    cg.add(var.set_led_clock(led_clock))
    if config[CONF_TYPE] == "colon_style":
        cg.add(led_clock.set_colon_style_select(var))
    elif config[CONF_TYPE] == "hour_color":
        cg.add(led_clock.set_hour_color_select(var))
    elif config[CONF_TYPE] == "minute_color":
        cg.add(led_clock.set_minute_color_select(var))
    elif config[CONF_TYPE] == "colon_color":
        cg.add(led_clock.set_colon_color_select(var))
    elif config[CONF_TYPE] == "downlight_color":
        cg.add(led_clock.set_downlight_color_select(var))