#include <iostream>
#include <alsa/asoundlib.h>

int main() {
    int err;
    void **hints;
    char *name;

    // Get a list of available device names
    if ((err = snd_device_name_hint(-1, "pcm", &hints)) < 0) {
        std::cerr << "Cannot get device name hints: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Iterate over the device names
    for (void **hint = hints; *hint; hint++) {
        name = snd_device_name_get_hint(*hint, "NAME");
        if (name) {
            std::cout << "Device Name: " << name << std::endl;
            free(name);
        }
    }

    // Free the hints list
    snd_device_name_free_hint(hints);

    return 0;
}