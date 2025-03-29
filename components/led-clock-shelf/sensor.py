import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_NAME, CONF_ENTITY_CATEGORY, UNIT_LUX, ICON_BRIGHTNESS_5
from . import led_clock_shelf_ns, LEDClockShelf

DEPENDENCIES = ["led_clock_shelf"]

LEDClockLightSensor = led_clock_shelf_ns.class_(
    "LEDClockLightSensor", sensor.Sensor, cg.Component
)

CONFIG_SCHEMA = sensor.SENSOR_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(LEDClockLightSensor),
    cv.Optional(CONF_NAME, default="LED Clock Light Sensor"): cv.string,
    cv.Required("parent_id"): cv.use_id(LEDClockShelf),
    cv.Optional(CONF_ENTITY_CATEGORY, default="diagnostic"): cv.entity_category,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)

    led_clock = await cg.get_variable(config["parent_id"])
    cg.add(var.set_led_clock(led_clock))
    cg.add(led_clock.set_light_sensor(var))