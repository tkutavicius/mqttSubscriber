local m, s
local certs = require "luci.model.certificate"
local fs = require "nixio.fs"
require("luci.config")

m = Map("mqtt_sub")
s = m:section(NamedSection, "mqtt_sub", "mqtt_sub", translate("MQTT Broker Configuration"), translate("The Broker will ...listen... for connections on the specified"))

enabled_sub = s:option(Flag, "enabled", translate("Enable"), translate("Select to enable MQTT subscriber"))

remote_addr = s:option(Value, "remote_addr", translate("Hostname"), translate("Specify address of the broker"))
remote_addr:depends("enabled", "1")
remote_addr.placeholder  = "localhost"
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

tls_cafile = s:option(FileUpload, "cafile", translate("CA file"), "")
tls_cafile:depends({tls = "1", tls_type = "cert"})

tls_certfile = s:option(FileUpload, "certfile", translate("Certificate file"), "")
tls_certfile:depends({tls = "1", tls_type = "cert"})

tls_keyfile = s:option(FileUpload, "keyfile", translate("Key file"), "")
tls_keyfile:depends({tls = "1", tls_type = "cert"})

o = s:option(Value, "psk", translate("Pre-Shared-Key"), translate("The pre-shared-key in hex format with no leading ???0x???"))
o.datatype = "lengthvalidation(0, 128)"
o.placeholder = "Key"
o:depends({tls = "1", tls_type = "psk"})

o = s:option(Value, "identity", translate("Identity"), translate("Specify the Identity"))
o.datatype = "uciname"
o.placeholder = "Identity"
o:depends({tls = "1", tls_type = "psk"})        

return m
