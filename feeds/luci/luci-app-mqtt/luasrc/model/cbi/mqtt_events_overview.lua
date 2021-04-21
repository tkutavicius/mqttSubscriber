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
topicName = s:option(DummyValue, "topicName", translate("Topic name"), translate("Name of MQTT Topic"))
aT = s:option(DummyValue, "attributeType", translate("Attribute Type"), translate("Message attribute type"))
a = s:option(DummyValue, "attribute", translate("Attribute"), translate("Message attribute"))
dC = s:option(DummyValue, "decimalComparator", translate("Decimal Comparator"), translate("If message attribute type is decimal, this shows comparator for decimal attribute")) 
function dC.cfgvalue(self, section)
	local v = Value.cfgvalue(self, section)
	if v == "0" then
        return ">"
    elseif v == "1" then
        return "<"
    elseif v == "2" then
        return "<="
    elseif v == "3" then
        return "<="
    elseif v == "4" then
        return "=="
    elseif v == "5" then
        return "!="
    else
        return "-"
    end
end
sC = s:option(DummyValue, "stringComparator", translate("String Comparator"), translate("If message attribute type is string, this shows comparator for string attribute"))
aV = s:option(DummyValue, "attributeValue", translate("Attribute Value"), translate("Message attribute value to compare"))
rE = s:option(DummyValue, "recipientEmail", translate("Email address to be informed"))
return m;