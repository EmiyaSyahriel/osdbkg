////////////////////////
/// ENTRY.CPP - Helper functions
/// Created at 14/05/2022 11:23 AM by EmiyaSyahriel
////////////////////////

#include "OSDBKG.HPP"
#include <cstring>


void osd_helper_init() {
	memcpy(states.wait_buffer, strings.dummy_text, strlen(strings.dummy_text));
}

// Doing some memcpy to spend some time
void osd_dummy_things() {
	memcpy(states.wait_buffer + 24, states.wait_buffer + 0, 8);
	memcpy(states.wait_buffer + 0, states.wait_buffer + 8, 8);
	memcpy(states.wait_buffer + 8, states.wait_buffer + 16, 8);
	memcpy(states.wait_buffer + 16, states.wait_buffer + 24, 8);
}
