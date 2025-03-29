import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_ID, CONF_NAME, CONF_MIN_VALUE, CONF_MAX_VALUE, CONF_STEP
from . import led_clock_shelf_ns, LEDClockShelf

DEPENDENCIES = ["led_clock_shelf"]

LEDClockBrightnessNumber = led_clock_shelf_ns.class_(
    "LEDClockBrightnessNumber", number.Number, cg.Component
)

CONFIG_SCHEMA = number.NUMBER_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(LEDClockBrightnessNumber),
    cv.Optional(CONF_NAME, default="LED Clock Brightness"): cv.string,
    cv.Required("parent_id"): cv.use_id(LEDClockShelf),
    cv.Optional(CONF_MIN_VALUE, default=0): cv.int_range(min=0, max=255),
    cv.Optional(CONF_MAX_VALUE, default=255): cv.int_range(min=0, max=255),
    cv.Optional(CONF_STEP, default=1): cv.int_range(min=1, max=255),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await number.register_number(
        var,
        config,
        min_value=config[CONF_MIN_VALUE],
        max_value=config[CONF_MAX_VALUE],
        step=config[CONF_STEP]
    )

    led_clock = await cg.get_variable(config["parent_id"])
    cg.add(var.set_led_clock(led_clock))
    cg.add(var.set_name(config[CONF_NAME]))