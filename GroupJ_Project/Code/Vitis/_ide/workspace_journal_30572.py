# 2026-03-22T23:37:51.740649400
import vitis

client = vitis.create_client()
client.set_workspace(path="GroupJ_Project")

platform = client.get_component(name="platform")
status = platform.build()

comp = client.get_component(name="app_component")
comp.build()

comp = client.create_app_component(name="app_component1",platform = "$COMPONENT_LOCATION/../platform/export/platform/platform.xpfm",domain = "standalone_microblaze_0")

client.delete_component(name="app_component1")

client.delete_component(name="componentName")

status = comp.clean()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

vitis.dispose()

