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
s:option(DummyValue, "attributeType", translate("Attribute Type"), translate("Message attribute type"))
s:option(DummyValue, "attribute", translate("Attribute"), translate("Message attribute"))
s:option(DummyValue, "decimalComparator", translate("Decimal Comparator"), translate("If message attribute type is decimal, this shows comparator for decimal attribute"))
s:option(DummyValue, "stringComparator", translate("String Comparator"), translate("If message attribute type is string, this shows comparator for string attribute"))
s:option(DummyValue, "attributeValue", translate("Attribute Value"), translate("Message attribute value to compare"))
return m;