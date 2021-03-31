module("luci.controller.mosquitto", package.seeall)

function index()
  entry( { "admin", "services", "mqtt"}, firstchild(), _("MQTT"), 150)
  entry( { "admin", "services", "mqtt", "broker" }, cbi("mqtt_broker"), _("Broker"), 1).leaf = true
  entry( { "admin", "services", "mqtt", "publisher" }, cbi("mqtt_pub"), _("Publisher"), 2).leaf = true
  entry( { "admin", "services", "mqtt", "subscriber" }, alias("admin","services","mqtt","subscriber","settings"), _("Subscriber"), 3)
  entry( { "admin", "services", "mqtt", "subscriber", "settings" }, cbi("mqtt_sub"), _("Broker Settings"), 4).dependent=false
  entry( { "admin", "services", "mqtt", "subscriber", "topics" }, cbi("mqtt_topics"), _("Subscription Topics"), 5).dependent=false
  entry( { "admin", "services", "mqtt", "subscriber", "messages" }, form("mqtt_messages"), _("Messages"), 6).dependent=false
end
