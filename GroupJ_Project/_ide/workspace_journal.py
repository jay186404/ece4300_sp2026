# 2026-03-23T04:46:36.078707400
import vitis

client = vitis.create_client()
client.set_workspace(path="GroupJ_Project")

# 2026-03-23T04:46:36.078707400
import vitis

client = vitis.create_client()
client.set_workspace(path="GroupJ_Project")

client.delete_component(name="platform")

client.delete_component(name="app_component")

vitis.dispose()

