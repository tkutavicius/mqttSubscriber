m = Map("mqtt_topics")
s = m:section(TypedSection, "mqtt_topic", translate("MQTT Topics"))                                        
s.template  = "cbi/tblsection"                                                                           
s.addremove = true                                                                                       
s.anonymous = true   
s.template_addremove = "mqtt/addremove"
s.novaluetext = translate("There are no MQTT topics created yet") 
s.delete_alert = true                                                                                 
s.alert_message = translate("Are you sure you want to delete this topic?")                                     
s.add_title = translate("Add new topic")                                                                
s.value_title = translate("Name")
s.value_description = translate("Name of new topic") 
s.list_title = translate("QoS")    
s.list_description = translate("QoS of new topic")  
s.addlist = {                                                                                                                                 
	0,
	1,
	2                                                                                                         
}                                                                   
s.create = function (self, section)
	local stat, exists
	local value = self.map:formvalue("cbi.cts." .. self.config .. "." .. self.sectiontype .. ".name")
	local qos = self.map:formvalue("cbi." .. self.config .. "." .. self.sectiontype .. ".select")
	local add = self.map:formvalue("cbi.cts." .. self.config .. "." .. self.sectiontype .. ".add")   
                                                                                                         
if value and #value > 0 then                                                                 
	self.map.uci:foreach(self.config, self.sectiontype, function(s)                          
		if s.topicName and s.topicName == value then                                               
			exists = true                                                            
			return false                                                             
		end                                                                              
	end)                                                                                     																							 
	if exists then                                                                        
		self.map:error_msg(translatef("Topic '%s' already exists", value))
		return false                                                                   
	end                                                                                    
	stat = AbstractSection.create(self, section)                                           																					   
	if stat then                                                                           
		self.map:set(stat, "topicName", value)
		self.map:set(stat, "qos", qos-1)                                             
	end                                                                                    
	elseif add and (not value or #value == 0) then                                                 
		m:error_msg(translate("No topic name provided"))                                     
		return nil                                                                             
	end                                                                                            																							   
	return stat                                               
end 
s:option(DummyValue, "topicName", translate("Topic name"), translate("Name of MQTT Topic"))
s:option(DummyValue, "qos", translate("QoS"), translate("QoS of MQTT Topic"))
return m;