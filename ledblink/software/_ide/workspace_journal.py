# 2026-03-21T10:02:30.252651700
import vitis

client = vitis.create_client()
client.set_workspace(path="software")

advanced_options = client.create_advanced_options_dict(dt_overlay="0")

platform = client.create_platform_component(name = "platform",hw_design = "$COMPONENT_LOCATION/../led.xsa",os = "linux",cpu = "psv_cortexa72",domain_name = "linux_psv_cortexa72",advanced_options = advanced_options)

platform = client.get_component(name="platform")
status = platform.build()

vitis.dispose()

