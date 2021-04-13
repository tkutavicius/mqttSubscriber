local dsp = require "luci.dispatcher"
m = Map("mqtt_events")
s = m:section(TypedSection, "mqtt_event", translate("MQTT Events"))                                        
s.template  = "cbi/tblsection"                                                                           
s.addremove = true                                                                                       
s.anonymous = true   
s.template_addremove = "cbi/add_rule"
s.extedit = dsp.build_url("admin", "services", "mqtt", "subscriber", "events", "%s")
s.novaluetext = translate("There are no MQTT events created yet") 
s.delete_alert = true                                                                                 
s.alert_message = translate("Are you sure you want to delete this event?")
s:option(DummyValue, "topicName", translate("Topic name"), translate("Name of MQTT Topic"))
s:option(DummyValue, "attributeType", translate("Attribute Type"), translate("QoS of MQTT Topic"))
s:option(DummyValue, "attribute", translate("Attribute"), translate("QoS of MQTT Topic"))
s:option(DummyValue, "decimalComparator", translate("Decimal Comparator"), translate("QoS of MQTT Topic"))
s:option(DummyValue, "stringComparator", translate("String Comparator"), translate("QoS of MQTT Topic"))
s:option(DummyValue, "attributeValue", translate("Attribute Value"), translate("QoS of MQTT Topic"))
return m;