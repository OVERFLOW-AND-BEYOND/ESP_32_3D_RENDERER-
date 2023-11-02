Hi !

This is a very cute project that I decided to develop in collaboration with my art professor: an esp32 is used to create an orthogonal projection of a predefined object.
The bitluni's VGA library is used to make the rendered shapes appear on the screen (you will find all the licensing details inside the first lines of the program), while
the actual shapes are renderer using pure math, no external libraries are involved in the rendering pipeline. 

Even if not perfect, I decided to share this project anyways, since I think that this could be a great inspiration for all the curious people out there.

In order to make it work, you just need to download the arduino ide, install the required libraries (bitluni's library and also ESP32 specific ones) and then upload the code!
You can find the pin configuration for the VGA screen in bitluni's youtube channel, witch has a bunch of videos that explain the whole process in detail, In the original code,
some buttons are also used to interact with the device.

The device is currently capable to render an orthogonal / prospective projection of a cube / piramid , witch can be rotated on the 3d axis using the dedicate button.

I hope that this project will be a source of inspiration for all the creative people.

ENJOY !