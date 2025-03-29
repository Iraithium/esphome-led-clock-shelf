import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID, CONF_NAME
from . import led_clock_shelf_ns, LEDClockShelf

DEPENDENCIES = ["led_clock_shelf"]

LEDClockDisplaySwitch = led_clock_shelf_ns.class_(
    "LEDClockDisplaySwitch", switch.Switch, cg.Component
)
LEDClockDownlightsSwitch = led_clock_shelf_ns.class_(
    "LEDClockDownlightsSwitch", switch.Switch, cg.Component
)

BASE_SCHEMA = switch.SWITCH_SCHEMA.extend({
    cv.Required("parent_id"): cv.use_id(LEDClockShelf),
}).extend(cv.COMPONENT_SCHEMA)

DISPLAY_SCHEMA = BASE_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(LEDClockDisplaySwitch),
    cv.Optional(CONF_NAME, default="LED Clock Display Switch"): cv.string,
})

DOWNLIGHTS_SCHEMA = BASE_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(LEDClockDownlightsSwitch),
    cv.Optional(CONF_NAME, default="LED Clock Downlights Switch"): cv.string,
})

PLATFORM_SCHEMA = cv.typed_schema({
    "display": DISPLAY_SCHEMA,
    "downlights": DOWNLIGHTS_SCHEMA,
}, lower=True)

CONFIG_SCHEMA = cv.Any(
    PLATFORM_SCHEMA,
    cv.ensure_list(PLATFORM_SCHEMA),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await switch.register_switch(var, config)

    led_clock = await cg.get_variable(config["parent_id"])
    cg.add(var.set_led_clock(led_clock))
    cg.add(var.set_name(config[CONF_NAME]))