#pragma once

void _kernel_exec_f(
		int (*exec_func)(int argc, const char **argv),
		int argc, const char **argv);