#pragma once

#include <signal.h>
#include <stdbool.h>

int _kernel_sigemptyset(sigset_t *set);
int _kernel_sigaddset(sigset_t *set, const int sgl);

bool _kernel_sigismember(sigset_t *set, const int sgl);
