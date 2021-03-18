local m, s, s1, s2, s3
local certs = require "luci.model.certificate"
local fs = require "nixio.fs"
require("luci.config")

m = Map("mqtt_sub")
s = m:section(NamedSection, "mqtt_sub", "mqtt_sub", translate("MQTT Broker Configuration"), translate("The Broker will ...listen... for connections on the specified"))

enabled_sub = s:option(Flag, "enabled", translate("Enable"), translate("Select to enable MQTT subscriber"))

remote_addr = s:option(Value, "remote_addr", translate("Hostname"), translate("Specify address of the broker"))
remote_addr:depends("enabled", "1")
remote_addr.placeholder  = "www.example.com"
remote_addr.default = "localhost"
remote_addr.datatype = "host"
remote_addr.parse = function(self, section, novld, ...)
	local enabled = luci.http.formvalue("cbid.mqtt_sub.mqtt_sub.enabled")
	local value = self:formvalue(section)
	if enabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: hostname is empty")
	end
	Value.parse(self, section, novld, ...)
end

remote_port = s:option(Value, "remote_port", translate("Port"), translate("Specify port of the broker"))
remote_port:depends("enabled", "1")
remote_port.default = "1883"
remote_port.placeholder = "1883"
remote_port.datatype = "port"
remote_port.parse = function(self, section, novld, ...)
	local enabled = luci.http.formvalue("cbid.mqtt_sub.mqtt_sub.enabled")
	local value = self:formvalue(section)
	if enabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: port is empty")
	end
	Value.parse(self, section, novld, ...)
end

topic = s:option(Value, "topic", translate("Message topic"), translate("Specify topic of the message"))
topic:depends("enabled", "1")
topic.default = "TeltonikaRUTX10"
topic.placeholder = "mytopic"
topic.datatype = "string"
topic.parse = function(self, section, novld, ...)
	local enabled = luci.http.formvalue("cbid.mqtt_sub.mqtt_sub.enabled")
	local value = self:formvalue(section)
	if enabled and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: topic is empty")
	end
	Value.parse(self, section, novld, ...)
end

remote_username = s:option(Value, "username", translate("Username"), translate("Specify username of remote host"))
remote_username.datatype = "credentials_validate"
remote_username.placeholder = translate("Username")
remote_username:depends("enabled", "1")
remote_username.parse = function(self, section, novld, ...)
	local enabled = luci.http.formvalue("cbid.mqtt_sub.mqtt_sub.enabled")
	local pass = luci.http.formvalue("cbid.mqtt_sub.mqtt_sub.password")
	local value = self:formvalue(section)
	if enabled and pass ~= nil and pass ~= "" and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: username is empty but password is not")
	end
	Value.parse(self, section, novld, ...)
end

remote_password = s:option(Value, "password", translate("Password"), translate("Specify password of remote host. Allowed characters (a-zA-Z0-9!@#$%&*+-/=?^_`{|}~. )"))
remote_password:depends("enabled", "1")
remote_password.password = true
remote_password.datatype = "credentials_validate"
remote_password.placeholder = translate("Password")
remote_password.parse = function(self, section, novld, ...)
	local enabled = luci.http.formvalue("cbid.mqtt_sub.mqtt_sub.enabled")
	local user = luci.http.formvalue("cbid.mqtt_sub.mqtt_sub.username")
	local value = self:formvalue(section)
	if enabled and user ~= nil and user ~= "" and (value == nil or value == "") then
		self:add_error(section, "invalid", "Error: password is empty but username is not")
	end
	Value.parse(self, section, novld, ...)
end

FileUpload.size = "262144"
FileUpload.sizetext = translate("Selected file is too large, max 256 KiB")
FileUpload.sizetextempty = translate("Selected file is empty")
FileUpload.unsafeupload = true

tls_enabled = s:option(Flag, "tls", translate("TLS"), translate("Select to enable TLS encryption"))
tls_enabled:depends("enabled", "1")

tls_type = s:option(ListValue, "tls_type", translate("TLS Type"), translate("Select the type of TLS encryption"))
tls_type:depends({enabled = "1", tls = "1"})
tls_type:value("cert", translate("Certificate based"))
tls_type:value("psk", translate("Pre-Shared-Key based"))

tls_insecure = s:option(Flag, "tls_insecure", translate("Allow insecure connection"), translate("Allow not verifying server authenticity"))
tls_insecure:depends({enabled = "1", tls = "1", tls_type = "cert"})

local certificates_link = luci.dispatcher.build_url("admin", "system", "admin", "certificates")
o = s:option(Flag, "_device_files", translate("Certificate files from device"), translatef("Choose this option if you want to select certificate files from device.\
																					Certificate files can be generated <a class=link href=%s>%s</a>", certificates_link, translate("here")))
o:depends({tls = "1", tls_type = "cert"})
local cas = certs.get_ca_files().certs
local certificates = certs.get_certificates()
local keys = certs.get_keys()

tls_cafile = s:option(FileUpload, "cafile", translate("CA file"), "")
tls_cafile:depends({tls = "1", _device_files="", tls_type = "cert"})

tls_certfile = s:option(FileUpload, "certfile", translate("Certificate file"), "")
tls_certfile:depends({tls = "1", _device_files="", tls_type = "cert"})

tls_keyfile = s:option(FileUpload, "keyfile", translate("Key file"), "")
tls_keyfile:depends({tls = "1", _device_files="", tls_type = "cert"})

tls_cafile = s:option(ListValue, "_device_cafile", translate("CA file"), "")
tls_cafile:depends({tls = "1", _device_files="1", tls_type = "cert"})

if #cas > 0 then
	for _,ca in pairs(cas) do
		tls_cafile:value("/etc/certificates/" .. ca.name, ca.name)
	end
else 
	tls_cafile:value("", translate("-- No files available --"))
end

function tls_cafile.write(self, section, value)
	m.uci:set(self.config, section, "cafile", value)
end

tls_cafile.cfgvalue = function(self, section)
	return m.uci:get(m.config, section, "cafile") or ""
end

tls_certfile = s:option(ListValue, "_device_certfile", translate("Certificate file"), "")
tls_certfile:depends({tls = "1", _device_files="1", tls_type = "cert"})

if #certificates > 0 then
	for _,certificate in pairs(certificates) do
		tls_certfile:value("/etc/certificates/" .. certificate.name, certificate.name)
	end
else 
	tls_certfile:value("", translate("-- No files available --"))
end

function tls_cafile.write(self, section, value)
	m.uci:set(self.config, section, "certfile", value)
end

tls_cafile.cfgvalue = function(self, section)
	return m.uci:get(m.config, section, "certfile") or ""
end

tls_keyfile = s:option(ListValue, "_device_keyfile", translate("Key file"), "")
tls_keyfile:depends({tls = "1", _device_files="1", tls_type = "cert"})

if #keys > 0 then
	for _,key in pairs(keys) do
		tls_keyfile:value("/etc/certificates/" .. key.name, key.name)
	end
else 
	tls_keyfile:value("", translate("-- No files available --"))
end

function tls_keyfile.write(self, section, value)
	m.uci:set(self.config, section, "keyfile", value)
end

tls_keyfile.cfgvalue = function(self, section)
	return m.uci:get(m.config, section, "keyfile") or ""
end

o = s:option(Value, "psk", translate("Pre-Shared-Key"), translate("The pre-shared-key in hex format with no leading “0x”"))
o.datatype = "lengthvalidation(0, 128)"
o.placeholder = "Key"
o:depends({tls = "1", tls_type = "psk"})

o = s:option(Value, "identity", translate("Identity"), translate("Specify the Identity"))
o.datatype = "uciname"
o.placeholder = "Identity"
o:depends({tls = "1", tls_type = "psk"})

s1 = m:section(TypedSection, "mqtt_topic", translate("MQTT Topics"))                                        
s1.template  = "cbi/tblsection"                                                                           
s1.addremove = true                                                                                       
s1.anonymous = true   
s1.template_addremove = "cbi/general_addremove"
s1.novaluetext = translate("There are no MQTT topics created yet") 
s1.delete_alert = true                                                                                 
s1.alert_message = translate("Are you sure you want to delete this topic?")                                     
s1.add_title = translate("Add new topic")                                                                
s1.value_title = translate("Name")
s1.value_description = translate("Name of new topic") 
s1.list_title = translate("QoS")    
s1.list_description = translate("QoS of new topic")  
s1.addlist = {                                                                                                                                 
	0,
	1,
	2                                                                                                         
}                                                                   
s1.create = function (self, section)
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
s1:option(DummyValue, "topicName", translate("Topic name"), translate("Name of MQTT Topic"))
s1:option(DummyValue, "qos", translate("QoS"), translate("QoS of MQTT Topic"))            

s2 = m:section(NamedSection, "mqtt_sub", "mqtt_sub",  translate("MQTT Messages"), translate(""))
o = s2:option(TextValue, "script")
o.readonly = true

function o.cfgvalue()
	return fs.readfile("/tmp/mqttMessages")
end

s3 = m:section(SimpleSection, "", translate(""))                                                                  
button = s3:option(DummyValue, "refresh")                     
button.template = "/mqtt/refreshbutton" 

return m
