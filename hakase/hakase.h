#pragma once

int setup();
int boot(int max);

void offloader_tryreceive();

// @return: whether all friend core finished its execution (returned from
// friend_main()).
bool is_friend_stopped();
