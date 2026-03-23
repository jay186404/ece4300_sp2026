# 2026-03-23T03:33:03.646953700
import vitis

client = vitis.create_client()
client.set_workspace(path="Vitis")

platform = client.get_component(name="platform")
status = platform.build()

comp = client.get_component(name="app_component")
comp.build()

vitis.dispose()

