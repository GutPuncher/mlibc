
#include <stdint.h>
#include <stdlib.h>
#include <bits/ensure.h>
#include <mlibc/elf/startup.h>

// defined by the POSIX library
void __mlibc_initLocale();
// defined by the POSIX library
void __mlibc_initStdio();

extern "C" uintptr_t *__dlapi_entrystack();

extern char **environ;
static mlibc::exec_stack_data __mlibc_stack_data;

struct LibraryGuard {
	LibraryGuard();
};

static LibraryGuard guard;

LibraryGuard::LibraryGuard() {
	__mlibc_initLocale();
	__mlibc_initStdio();

	// Parse the exec() stack.
	mlibc::parse_exec_stack(__dlapi_entrystack(), &__mlibc_stack_data);

	// Initialize environ.
	// TODO: Copy the arguments instead of pointing to them?
	auto ev = __mlibc_stack_data.envp;
	while(*ev) {
		auto fail = putenv(*ev);
		__ensure(!fail);
		ev++;
	}
}

extern "C" void __mlibc_entry(int (*main_fn)(int argc, char *argv[], char *env[])) {
	auto result = main_fn(__mlibc_stack_data.argc, __mlibc_stack_data.argv, environ);
	exit(result);
}
