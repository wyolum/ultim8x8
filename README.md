# ultim8x8

ultim8x8 is an APA102 RGB LED based panel consisting of 8x8 matrix. Panels can be chained to produce larger matrices. Besides +5V and GND, each board requires a CLocK and DATa signal. Each board has CLocK and DATa IN and OUT terminals. When cascading boards, the OUT from one board goes to the IN of the next board.

Each LED can require up to 60mA current at 5V for White light at full brightness. This means driving ALL the 64 LEDs at full brightness White color will require 3.8A Amps. You can get away with lower Amps if you drive LEDs at lower brightness or don't light them all up at the same time.

Boards can be cascaded using several methods. Simplest, which requires no soldering, is to screw in the supplied metal posts to the terminals marked +5V, GND, DI__A, CI__A, DO__H and CO__H. The Boards can then be interconnected using the ultim_BUS busbar, or copper tape stuck over a piece of acrylic.

When running at low Amps or driving a single board, the 4 pin 0.1" pitch header can be used.

## kicad

* [ultim8x8_v0](/kicad/kicad_8x8_v0) - first version, Main hardware design files. Schematic, Layout etc. (deprecated)
* [ultim8x8_v1](/kicad/kicad_8x8_v1) - second version, Main hardware design files. Schematic, Layout etc. (deprecated)
* [ultim8x8_v2](/kicad/kicad_8x8_v2) - third version, Main hardware design files. Schematic, Layout etc. (current)

* [ultim_bus_v1](/kicad/ultim_bus_v1) - first version, Hardware design files for a bus bar to inter-connect ultim8x8's (deprecated)
* [ultim_bus_v2](/kicad/ultim_bus_v2) - second version, Hardware design files for a bus bar to inter-connect ultim8x8's (deprecated)
* [ultim_bus_v3](/kicad/ultim_bus_v3) - third version, Hardware design files for a bus bar to inter-connect ultim8x8's (deprecated)
* [ultim_bus_v4](/kicad/ultim_bus_v4) - fourth version, Hardware design files for a bus bar to inter-connect ultim8x8's (deprecated)
* [ultim_bus_v5](/kicad/ultim_bus_v5) - fifth version, Hardware design files for a bus bar to inter-connect ultim8x8's (current)

* [ultim_bus_conn](/kicad/ultim_bus_conn) - connector strips to join multiple ultim__bus_bars

* [featherboard](/kicad/featherboard) - first version, Interface board to connect Adafruit Feather Huzzah board to ultim8x8
* [feather_v2](/kicad/feather_v2) - second version, Interface board to connect Adafruit Feather Huzzah board to ultim8x8
* [feather_v3](/kicad/feather_v3) - third version, Interface board to connect Adafruit Feather Huzzah board to ultim8x8



* [ultim8x8_data](/kicad/ultim8x8_data) - Data sheets, logos etc

## fabricate

* [buckle](/fabricate/buckle) - 3D CAD files for a Belt Buckle.
* [featherboard_case](/fabricate/featherboard_case) - 3D CAD files for a "wireless" ultim8x8 brick


![ultim8 top](/kicad/kicad_8x8_v2/ultim8x8_images/ultim8x8_01.png)

![ultim8 bottom](/kicad/kicad_8x8_v2/ultim8x8_images/ultim8x8_02.png)

License
-------
[CERN Open Hardware Licence v1.2 ]

[CERN Open Hardware Licence v1.2 ]:http://www.ohwr.org/attachments/2388/cern_ohl_v_1_2.txt
