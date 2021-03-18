module("luci.controller.mosquitto", package.seeall)

function index()
  entry( { "admin", "services", "mqtt"}, firstchild(), _("MQTT"), 150)
  entry( { "admin", "services", "mqtt", "broker" }, cbi("mqtt_broker"), _("Broker"), 1).leaf = true
  entry( { "admin", "services", "mqtt", "publisher" }, cbi("mqtt_pub"), _("Publisher"), 2).leaf = true
  entry( { "admin", "services", "mqtt", "subscriber" }, cbi("mqtt_sub"), _("Subscriber"), 2).leaf = true
end
