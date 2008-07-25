rem  Qemu Tor VM start script
rem  Modify parameters for Qemu as desired.
rem  Kernel parameters accepted via -append include:
rem    TZ=<timezome>
rem    PRIVIP=<tap32 IP address>
rem    IP=<VM IP address>
rem    MASK=<VM netmask>
rem    GW=<default gateway>
rem    MAC=<VM ethernet MAC address>
rem
SET MAC=00:11:22:33:44:55
SET DEVICE="Local Area Connection"
SET RAMSZ=32

qemu.exe -name "Tor VM" -L . -kernel vmlinuz -append "quiet loglevel=1" -hda hdd.img -m %RAMSZ% -std-vga -net nic,vlan=0,model=pcnet,macaddr=%MAC% -net pcap,vlan=0,devicename="%DEVICE%"
