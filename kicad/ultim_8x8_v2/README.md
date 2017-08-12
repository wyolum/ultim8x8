# ultim_8x8

ultim8x8 is an APA102 RGB LED based panel consisting of 8x8 matrix. Panels can be chained to produce larger matrices. Besides +5V and GND, each board requires a CLocK and DATa signal. Each board has CLocK and DATa IN and OUT terminals. When cascading boards, the OUT from one board goes to the IN of the next board.

Each LED can require up to 60mA current at 5V for White light at full brightness. This means driving ALL the 64 LEDs at full brightness White color will require 3.8A Amps. You can get away with lower Amps if you drive LEDs at lower brightness or don't light them all up at the same time.

Boards can be cascaded using several methods. Simplest, which requires no soldering, is to screw in the supplied metal posts to the terminals marked +5V, GND, D_IN, C_IN, D_OUT and C_OUT. The Boards can then be interconnected using the ultim_BUS busbar, or copper tape stuck over a piece of acrylic.

When running at low Amps or driving a single board, the 4 pin 0.1" pitch headers can be used. There are two pairs of headers marked In1/Out1 and In2/Out2.


![ultim_8x8 top](ultim8x8_images/ultim8x8_01.png)

![ultim_8x8 top](ultim8x8_images/ultim8x8_02.png)

License
-------
[CERN Open Hardware Licence v1.2 ]

[CERN Open Hardware Licence v1.2 ]:http://www.ohwr.org/attachments/2388/cern_ohl_v_1_2.txt
