m = SimpleForm("messages")
m.submit = false
m.reset = false

local s = m:section(Table, messages, translate("MQTT Messages"), translate("Table contains a chronological list of incoming MQTT messages."))
s.anonymous = true
s.template = "mqtt/tblsection_messages"
s.addremove = false
s.refresh = true
s.table_config = {
    truncatePager = true,
    labels = {
        placeholder = "Search...",
        perPage = "Messages per page {select}",
        noRows = "No messages found",
        info = ""
    },
    layout = {
        top = "<table><tr style='padding: 0 !important; border:none !important'><td style='display: flex !important; flex-direction: row'>{select}<span style='margin-left: auto; width:150px'>{search}</span></td></tr></table>",
        bottom = "{info}{pager}"
    }
}

o = s:option(DummyValue, "date", translate("Date"))
o = s:option(DummyValue, "topic", translate("Topic"))
o = s:option(DummyValue, "payload", translate("Payload"))
s:option(DummyValue, "", translate(""))

return m