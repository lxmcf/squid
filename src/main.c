#include <libusb-1.0/libusb.h>

#include <string.h>

#include <unistd.h>
#include <stdio.h>

#define EXIT_SUCCESS 0
#define EXIT_NO_ROOT 1
#define EXIT_FAIL_USB 2
#define EXIT_FAIL_DEVICES 3

#define PASS(x) (x >= 0)

static libusb_context* context;
static libusb_device** devices;

int main (int argc, const char* argv[]) {
    if (geteuid () != 0) {
        fprintf (stderr, "Please run as root!\n");

        return EXIT_NO_ROOT;
    }

    if (libusb_init (&context)) {
        fprintf (stderr, "Failed to initialise libusb!\n");

        return EXIT_FAIL_USB;
    }

    ssize_t device_count = libusb_get_device_list (context, &devices);
    if (device_count < 0) {
        fprintf (stderr, "Failed to get USB devices\n");
        libusb_exit (context);

        return EXIT_FAIL_DEVICES;
    }

    libusb_device* device;
    unsigned char product[256];

    for (ssize_t i = 0; i < device_count; i++) {
        device = devices[i];
        struct libusb_device_descriptor descriptor;

        if (PASS (libusb_get_device_descriptor (device, &descriptor))) {
            libusb_device_handle* handle;

            if (PASS (libusb_open (device, &handle))) {
                libusb_get_string_descriptor_ascii (handle, descriptor.iProduct, (unsigned char*)product, sizeof (product));

                libusb_close (handle);

                // FIXME: Move to Vendor ID check
                if (strstr (product, "NZXT") != NULL) {
                    printf ("Device Name:\t%s\n", product);
                }
            }
        }
    }

    libusb_free_device_list (devices, 1);
    libusb_exit (context);

    return EXIT_SUCCESS;
}