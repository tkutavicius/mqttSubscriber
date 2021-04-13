local dsp = require "luci.dispatcher"
local uci = require "luci.model.uci".cursor()

local m, s, o
local sid = arg[1]

m = Map("mqtt_events")
m.redirect = dsp.build_url("admin", "services", "mqtt", "subscriber", "events")

if not sid or not m.uci:get("mqtt_events", arg[1]) then
        luci.http.redirect(m.redirect)
end

s = m:section(NamedSection, sid, "mqtt_events", translate("MQTT Event Settings"))
s.addremove = false

lv = s:option(ListValue, "topicName", translate("Topic"), translate("MQTT topic name"));
m.uci:foreach("mqtt_topics","mqtt_topic",
    function(i)
        lv:value(i.topicName, i.topicName)
    end)

o = s:option(ListValue, "attributeType", translate("Attribute Type"), translate("Type of the attribute"))
o:value("Decimal", "Decimal")
o:value("String", "String")

o1 = s:option(Value, "attribute", translate("Attribute"))
o1.rmempty = false

o2 = s:option(ListValue, "decimalComparator", translate("Comparator"))
o2:depends("attributeType","Decimal")
o2:value(">", ">")
o2:value("<", "<")
o2:value(">=", ">=")
o2:value("<=", "<=")
o2:value("==", "==")
o2:value("!=", "!=")

o3 = s:option(ListValue, "stringComparator", translate("Comparator"))
o3:depends("attributeType","String")
o3:value("Equal To", "Equal To")
o3:value("Not Equal To", "Not Equal To")

o4 = s:option(Value, "attributeValue", translate("Attribute Value"))
o4.rmempty = false

return m

