# 2026-03-22T18:45:37.363342500
import vitis

client = vitis.create_client()
client.set_workspace(path="GroupJ_Project")

platform = client.create_platform_component(name = "platform",hw_design = "$COMPONENT_LOCATION/../../../FPGA/Vivado/microblaze/microblaze.xsa",os = "standalone",cpu = "microblaze_0",domain_name = "standalone_microblaze_0",compiler = "gcc")

comp = client.create_app_component(name="app_component",platform = "$COMPONENT_LOCATION/../platform/export/platform/platform.xpfm",domain = "standalone_microblaze_0")

platform = client.get_component(name="platform")
status = platform.build()

comp = client.get_component(name="app_component")
comp.build()

client.delete_component(name="platform")

client.delete_component(name="platform")

client.delete_component(name="app_component")

client.delete_component(name="componentName")

platform = client.create_platform_component(name = "platform",hw_design = "$COMPONENT_LOCATION/../../../FPGA/Vivado/microblaze/microblaze.xsa",os = "standalone",cpu = "microblaze_0",domain_name = "standalone_microblaze_0",compiler = "gcc")

comp = client.create_app_component(name="app_component",platform = "$COMPONENT_LOCATION/../platform/export/platform/platform.xpfm",domain = "standalone_microblaze_0")

client.delete_component(name="app_component")

comp = client.create_app_component(name="app_component",platform = "$COMPONENT_LOCATION/../platform/export/platform/platform.xpfm",domain = "standalone_microblaze_0")

status = platform.build()

comp.build()

status = platform.build()

comp.build()

client.delete_component(name="app_component")

status = platform.build()

comp = client.create_app_component(name="app_component",platform = "$COMPONENT_LOCATION/../platform/export/platform/platform.xpfm",domain = "standalone_microblaze_0")

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

