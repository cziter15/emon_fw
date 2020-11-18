Inspired by other project, that counts rotations of energy meter's ring, I've decided to create my own esp-based board doing the same.

First board revision based on digital readings, this technique however, introduced many problems, because it was very light-sensitive. Bright sunlight sometimes affected measurements.

Second revision uses ADC instead, so I can detect rotations more precisely. From EMI perspective this board is designed better. I've had few troubles soldering this, because it failed to start for the first time - due to wrong AMS1117 version ordered. However I've managed to replace this IC with correct one.

Along with hardware-part, I'm working on software - framework for IoT devices like this one, so end user application might be developed fairly quick!